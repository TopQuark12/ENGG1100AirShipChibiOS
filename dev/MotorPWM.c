/*
 * MotorPWM.c
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#include "ch.h"
#include "hal.h"
#include "MotorPWM.h"

static PWMpin_t motorPins[MOTORNUM];

static PWMConfig pwmcfg = {
  5000000,                                      /* 500 kHz PWM clock frequency.   */
  10000,                                        /* Initial PWM period 1S.       */
  //pwmpcb,
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0,
#if STM32_PWM_USE_ADVANCED
  0
#endif
};

PWMpin_t* getPins(void) {

  return motorPins;

}

void pinInit(PWMpin_t* pin, PWMDriver* PWMdriver, uint8_t PWMchannel) {

  pin->driver = PWMdriver;
  pin->channel = PWMchannel;

}

void motorpwmSet(PWMpin_t* pin) {

  pwmEnableChannel(pin->driver, pin->channel, PWM_PERCENTAGE_TO_WIDTH(pin->driver, pin->dutycycle));

}

static THD_WORKING_AREA(MotorThdwa, 128);
static THD_FUNCTION(MotorThd, arg) {

  (void)arg;

  static uint8_t i = 0;

  while (!chThdShouldTerminateX()) {

    for (i = 0; i < MOTORNUM; i++) {
      motorpwmSet(&motorPins[i]);
    }

    chThdSleepMilliseconds(1);
  }
}

void motorpwmInit(void) {

  pwmStart(&PWMD1, &pwmcfg);
  pwmStart(&PWMD3, &pwmcfg);

  memset((void*) motorPins, 0, sizeof(PWMpin_t) * MOTORNUM);

  pinInit(&motorPins[0], &PWMD1, 0);
  pinInit(&motorPins[1], &PWMD1, 1);
  pinInit(&motorPins[2], &PWMD1, 2);
  pinInit(&motorPins[3], &PWMD1, 3);
  pinInit(&motorPins[4], &PWMD3, 0);

  chThdCreateStatic(MotorThdwa, sizeof(MotorThdwa), NORMALPRIO + 10, MotorThd, NULL);

}
