/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "MotorPWM.h"
#include "Bluetooth.h"
#include "Tof.h"
#include "imu.h"

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palClearPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(10);
    palSetPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(10);
  }
}

PWMpin_t* pins;

int main(void) {


  halInit();
  chSysInit();

  bluetoothInit();

  motorpwmInit();

  imuInit();

  tofInit();

  pins = getPins();
  static uint8_t i = 0;

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  while (true) {

    for (i = 0; i < MOTORNUM; i++) {
      pins[i].dutycycle = pins[i].dutycycle >= 10000 ? 0 : pins[i].dutycycle + 200;
    }

    chThdSleepMilliseconds(20);

  }
}
