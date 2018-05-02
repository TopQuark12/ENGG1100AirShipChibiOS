/*
 * Motor.c
 *
 *  Created on: 3 May 2018
 *      Author: Alex's Desktop
 */

#include "ch.h"
#include "hal.h"
#include "Motor.h"
#include "MotorPWM.h"

PWMpin_t* PWMPins;
Motor_t motors[MOTORNUM];

Motor_t* getMotor(void) {

  return motors;

}

void motorSetup(Motor_t* motor_setting, PWMpin_t* pwm_setting,
                GPIO_TypeDef* inaport, uint8_t inapin,
                GPIO_TypeDef* inbport, uint8_t inbpin) {

  motor_setting->PWM_pin = pwm_setting;
  motor_setting->INA_port = inaport;
  motor_setting->INA_pin = inapin;
  motor_setting->INB_port = inbport;
  motor_setting->INB_pin = inbpin;
  motor_setting->power = 0;

}

void motorSet(Motor_t* motor_setting) {

  if (motor_setting->power > 0) {
    palSetPad(motor_setting->INA_port, motor_setting->INA_pin);
    palClearPad(motor_setting->INB_port, motor_setting->INB_pin);
    motor_setting->PWM_pin->dutycycle = motor_setting->power < MAXPWR ?
                                        motor_setting->power : MAXPWR;
  } else {
    palClearPad(motor_setting->INA_port, motor_setting->INA_pin);
    palSetPad(motor_setting->INB_port, motor_setting->INB_pin);
    motor_setting->PWM_pin->dutycycle = -motor_setting->power < MAXPWR ?
                                        -motor_setting->power : MAXPWR;
  }

}

static THD_WORKING_AREA(MotorThdwa, 128);
static THD_FUNCTION(MotorThd, arg) {

  (void)arg;

  static uint8_t ii = 0;

  while (!chThdShouldTerminateX()) {

    for (ii = 0; ii < MOTORNUM; ii++) {
      motorSet(motors + ii);
    }

    chThdSleepMilliseconds(1);
  }
}

void motorsInit(void) {

  PWMPins = getPins();

  memset((void*) motors, 0, sizeof(Motor_t) * MOTORNUM);

  motorSetup(motors + 0, PWMPins + 0, GPIOA, 0, GPIOA, 1);
  motorSetup(motors + 1, PWMPins + 1, GPIOA, 4, GPIOA, 5);
  motorSetup(motors + 2, PWMPins + 2, GPIOA, 12, GPIOB, 13);
  motorSetup(motors + 3, PWMPins + 3, GPIOB, 14, GPIOB, 15);
  motorSetup(motors + 4, PWMPins + 4, GPIOB, 5, GPIOB, 8);

  chThdCreateStatic(MotorThdwa, sizeof(MotorThdwa), NORMALPRIO + 7, MotorThd, NULL);

}

