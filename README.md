Thie file contains the PCB design repo for my VT Senior Design Project: "Prognostic Health Monitoring of Power Electronics". Basis of the project was to design a "Proof Of Concept" system that could be retrofitted to any Switching Power Converter (DC-AC, AC-DC, DC-DC0 that is onboard an overarching system. The board itself to be used as a lifespan monitor to alert technicians as to when maintenancing or replacing electronics could be done to prevent critical failures during operation. For my team's methodology we implemented a DAQ that is capable of collective live FET data for determination of remaining use cycles prior to thermal fatigue failure and bond breakdown. 

Additional documentation related to project and experimental data can be provided upon request!

Dev Code developed by Corwin Warner

<img width="1496" height="1438" alt="image" src="https://github.com/user-attachments/assets/5ede6941-0511-4455-8f50-de8ca9d6a03c" />

Some more insight into the Coffin-Manson implementation can be found here: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=9846435

Points Of Measurement for Coffin-Manson Degradation Estimation: 
- Individual FET Junction Temperatures (Internal Temp but we'll estimate with NTC Thermistory on top of the FET package)
- Junction Temp Swing (Max to Min Temp during a power cycle)
- Absolute Junction Temperature

<img width="370" height="200" alt="image" src="https://github.com/user-attachments/assets/2ce38586-c7f0-40c3-9641-f8223bb859bd" />

Additionall Modifiers for Predictive Fine-Tuning: 
- Humidity
- FET Current (Drain Current)
- Junction Voltage (Gate Voltage in our approach)
- Vibration (Through IMU measurement)

Schematic Layout: 
<img width="1531" height="1038" alt="image" src="https://github.com/user-attachments/assets/b39ca7cf-3154-44f8-8181-349d9905936b" />

System Logic Diagram: 
<img width="1147" height="549" alt="image" src="https://github.com/user-attachments/assets/ad53c0ef-2959-47b5-b17a-16ef99efc587" />

Initial breadboarding to validate code and sensor selection completed: 
<img width="900" height="527" alt="image" src="https://github.com/user-attachments/assets/4ebdbe6e-c93b-46b8-a2cd-c23a82f69655" />

<img width="1512" height="2016" alt="image" src="https://github.com/user-attachments/assets/0be0f956-cdce-4653-be31-7b9ceb061ce4" />

Full PCB Design and initial test chamber completed. Limited test time due to limitation on on-site testing on campus grounds. Would be worth going back to assess the longevity of the unit and full validation.

<img width="1649" height="963" alt="image" src="https://github.com/user-attachments/assets/5d89ea2a-71a1-4ed1-b716-e1602b6de3e8" />

Some updates that could be made for the future: 
- ESC inverter bridge was used as the Switching Device Under Load (DC-AC) in the interest of cost and time (https://www.amazon.com/dp/B00XKX5TBE?lv=shuf&channelId=500&plpRedirect=mhFallback.) Could move towards a VESC or a higher power switching device.
- Update PCB to have flush connection to sensor breakout boards or integrate into board design rather than conneciton through header pins
- Have an updated test chamber rig setup with new artifical humidity chamber settings
- Update the load type to be motor-to-motor with an automated current step up and down for an accurate profile.
- Develop a closer to real life curve for the Coffin-Manson Equation Prediction with a longer 24-hour run time.
- Update repo with eq. instituting humidity and additional modfiiers. 
