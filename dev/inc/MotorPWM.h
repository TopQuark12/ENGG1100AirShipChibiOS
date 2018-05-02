/*
 * MotorPWM.h
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#ifndef INC_MOTORPWM_H_
#define INC_MOTORPWM_H_

#define MOTORNUM        5
#define MAXPWR          10000

typedef struct PWMpin_t{

  PWMDriver*        driver;
  uint8_t           channel;

  uint16_t          dutycycle;

} PWMpin_t;

PWMpin_t* getPins(void);

void motorpwmInit(void);

#endif /* INC_MOTORPWM_H_ */
