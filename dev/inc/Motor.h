/*
 * Motor.h
 *
 *  Created on: 3 May 2018
 *      Author: Alex's Desktop
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_
#include "MotorPWM.h"

typedef struct Motor_t{

  PWMpin_t*         PWM_pin;
  GPIO_TypeDef*     INA_port;
  GPIO_TypeDef*     INB_port;
  uint8_t           INA_pin;
  uint8_t           INB_pin;
  int16_t           power;

}Motor_t;

Motor_t* getMotor(void);

void motorsInit(void);

#endif /* INC_MOTOR_H_ */
