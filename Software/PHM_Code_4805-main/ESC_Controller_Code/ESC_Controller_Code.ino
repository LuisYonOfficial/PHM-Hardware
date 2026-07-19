
#include <Servo.h>

Servo esc;

const int escPin = 3;
const int minPulseRate = 0;
const int maxPulseRate = 180;

const int throttleChangeDelay = 15;


int throttleTarget = 0;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.setTimeout(500);

    esc.attach(escPin, minPulseRate, maxPulseRate);

    esc.write(0);
}

void loop() {
    // put your main code here, to run repeatedly:
    int val = 0;

    if (Serial.available() > 0)
    {
        val = Serial.parseInt();

        if (val == 0)
        {
            goto end_loop;
        }

        val = convertThrottle(val);
        throttleTarget = val;
        //Serial.print("Val: ");
    //Serial.println(val);

    //esc.writeMicroseconds(1020);
    }



    changeThrottle(val);

end_loop:;


    int adc0 = analogRead(A0);
    double reading = adc0;
    //  if(adc0 > 515)
    //  {
    //    reading = c_map(adc0, 515, 1023, 0, 30);
    //  }
    //  else
    //  {
    //    reading = c_map(adc0, 512, 0, 0, 30);
    //  }
    reading = reading - (1023 / 2); //offset
    reading = abs(reading); //normalize current positive
    reading = c_map(reading, 0, 513, 0, 30);

    Serial.print("reading: ");
    Serial.println(reading);
}



int convertThrottle(int perc)
{
    return(map(perc, 0, 100, minPulseRate, maxPulseRate));
}

void changeThrottle(int val_us)
{
    //static int oldval_us = minPulseRate;

    //int curThrottle = esc.read();
    //int curThrottle = map(esc.read(), 0, 180, minPulseRate, maxPulseRate);

    //int toTarget = map(throttleTarget, minPulseRate

    //Serial.print("Cur throt:");
    //Serial.print(esc.read());
    //Serial.print(" targetT:");
    //Serial.println(throttleTarget);


    if (esc.read() < throttleTarget)
    {
        //Serial.println("here - 1");
        //curThrottle = curThrottle + 1;
        esc.write(esc.read() + 1);
        //esc.read();
        delay(throttleChangeDelay);
    }
    else if (esc.read() > throttleTarget)
    {
        //Serial.println("here - 2");
        //curThrottle = curThrottle + 1;
        esc.write(esc.read() - 1);
        //esc.read();
        delay(throttleChangeDelay);
    }
    else
    {
        //Serial.println("here - 3");
        esc.write(esc.read());
    }
}



double c_map(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}