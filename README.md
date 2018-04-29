# ENGG1100AirShipChibiOS
Code written by Alex Wong Tat Hang for HKUST ENGG1100 AirShip project. Programmed with Chibios for the STM32F103 BluePill Board. Everything is open-source, fell free to build upon my work.  

This code is built upon the STM32F103 NUCLEO board example code, with heavy modifications on the hardware level, e.g. clock
settings, GPIO, Serial driver etc. The code is now configured to run on the common and cheap 'Blue Pill Board' at the max 72MHz.  

To manage everything efficiently, the code utilises the ChibiOS Real Time Operating System Module to realise multi-threading capability.  

In the final product, I hope to implement the following controls:  
- Feed-forward with feed-back PID trim to stabilize airship height, using time-of-flight sensor as feed-back loop input.  
- Feed-back PID loop to stabalize airship yaw-axis with IMU as input.  
- If there is enough computational resources left over, rotor speed will be controlled with a feedback loop as well.  
  This function will use a reflectance switch as sensor, hardware interupt or timer input capture unit as counter.  
  This is done to remove nonlinearity between PWM output and rotor speed. With this implementated, the above two control
  loop will cascade upon this loop. Otherwise, rotor speed will be controled with a look up table.
