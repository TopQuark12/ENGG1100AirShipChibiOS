/*
 * Tof.c
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#include "ch.h"
#include "hal.h"
#include "Tof.h"
#define ARM_MATH_CM3
#include <arm_math.h>
#include "imu.h"

#define FLUSH_I_QUEUE(sdp)      \
    chSysLock();                \
    chIQResetI(&(sdp)->iqueue); \
    chSysUnlock();                          //Flush serial in queue

#define LEAST_SET_BIT(x)        x&(-x)      //Clear all but least set bit

#define ACQTIME                 1           //Milliseconds

#define SERIAL_EVT_MASK         1

static uint16_t distance = 0;

static float32_t height = 0;

static imu_t* imu_data;

static uint8_t foundheader = 0;
static uint8_t datalength = 0;
static uint8_t sdrxbuf[SERIAL_BUFFERS_SIZE];

static const SerialConfig SerialCfg = {
  115200,               //Baud Rate
  USART_CR1_UE,         //CR1 Register
  USART_CR2_LINEN,      //CR2 Register
  0                     //CR3 Register
};

void tofdecode(void) {

  static int16_t temp_dis;
  static uint8_t i;
  i = 0;
  while(i < datalength) {                                                   //Scan for frame head
    if ((sdrxbuf[i] == TOFFRAMEHEAD)) {
      temp_dis = 0;
      i++;
      while((sdrxbuf[i] != TOFFRAMEEND) && (i < datalength)) {
        temp_dis = temp_dis * 10 + (sdrxbuf[i] - 48);
        i++;
      }
      distance = temp_dis < 2000 ? temp_dis : distance;
    } else {
      i++;
    }
  }

  height = ((float32_t)distance * arm_cos_f32(imu_data->euler.pit_deg / 360.0 * 2 * PI)) *
           (arm_cos_f32(imu_data->euler.roll_deg / 360.0 * 2 * PI));
           //(arm_cos_f32(imu_data->euler.yaw_deg / 360.0 * 2 * PI));

}

static THD_WORKING_AREA(TofThd_wa, 128);
static THD_FUNCTION(TofThd, arg) {

  (void)arg;

  memset((void*) sdrxbuf, 0, SERIAL_BUFFERS_SIZE);

  memset((void*) distance, 0, sizeof(distance));

  static const eventflags_t serial_wkup_flags =                     //Partially from SD driver
    CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR |       //Partially inherited from IO queue driver
    SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR |
    SD_BREAK_DETECTED;

  event_listener_t serial_listener;
  static eventflags_t pending_flags;
  static eventflags_t current_flag;
  chEvtRegisterMaskWithFlags(chnGetEventSource(TOFDRIVER), &serial_listener,
                             SERIAL_EVT_MASK, serial_wkup_flags);   //setup event listening

  while (!chThdShouldTerminateX()) {

    chEvtWaitAny(1);                                                //wait for selected serial events
    chSysLock();
    pending_flags = chEvtGetAndClearFlagsI(&serial_listener);       //get event flag
    chSysUnlock();
    foundheader = false;

    do {

        current_flag = LEAST_SET_BIT(pending_flags);                  //isolates single flag to work on
        pending_flags &= ~current_flag;                               //removes isolated flag

        switch(current_flag) {

            case CHN_INPUT_AVAILABLE:                                     //Serial data available
                chThdSleep(MS2ST(ACQTIME));                            //Acquire data packet, release CPU
                if((!pending_flags)) {
                    datalength = sdAsynchronousRead(TOFDRIVER, &sdrxbuf,
                                                   (size_t)SERIAL_BUFFERS_SIZE);  //Non-blocking data read
                    tofdecode();
                }

                FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case CHN_DISCONNECTED:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case SD_NOISE_ERROR:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case SD_PARITY_ERROR:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case SD_FRAMING_ERROR:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case SD_OVERRUN_ERROR:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            case SD_BREAK_DETECTED:
            FLUSH_I_QUEUE(TOFDRIVER);
                break;

            default:
                break;

        }

    } while (pending_flags && !foundheader);

    FLUSH_I_QUEUE(TOFDRIVER);
    memset((void*)sdrxbuf, 0, SERIAL_BUFFERS_SIZE);               //Flush RX buffer

  }

}

void tofInit(void) {

  sdStart(TOFDRIVER, &SerialCfg);

  distance = 0;

  memset((void*) &height, 0, sizeof(height));

  imu_data = getIMU();

  chThdCreateStatic(TofThd_wa, sizeof(TofThd_wa),
                    NORMALPRIO + 3, TofThd, NULL);

}
