/*
 * imu.c
 *
 *  Created on: 27 Apr 2018
 *      Author: Alex's Desktop
 */

#include "ch.h"
#include "hal.h"
#include "imu.h"

#define FLUSH_I_QUEUE(sdp)      \
    chSysLock();                \
    chIQResetI(&(sdp)->iqueue); \
    chSysUnlock();                          //Flush serial in queue

#define LEAST_SET_BIT(x)        x&(-x)      //Clear all but least set bit

#define ACQTIME                 2           //Milliseconds

#define SERIAL_EVT_MASK         1

static imu_t imu;

static uint8_t foundheader = 0;
static uint8_t datalength = 0;
static uint8_t sdrxbuf[SERIAL_BUFFERS_SIZE];
static uint8_t sdtxbuf[SERIAL_BUFFERS_SIZE];

static const SerialConfig SerialCfg = {
  115200,               //Baud Rate
  USART_CR1_UE,         //CR1 Register
  USART_CR2_LINEN,      //CR2 Register
  0                     //CR3 Register
};

uint8_t getchecksum(uint8_t* data, uint8_t n) {

  static i = 0;
  static uint8_t sum;
  sum = 0;
  for(i = 0; i < n; i++) {
    sum += *data;
    data++;
  }

  return sum;

}

void imudecode(void) {

  static uint8_t i;

  i = 0;
  while(i < datalength) {

    if ((sdrxbuf[i] == IMURXFRAMEHEAD) &&
        (sdrxbuf[i + 1] == IMURXFRAMEHEAD) &&
        (getchecksum(&sdrxbuf[i], 4 + sdrxbuf[i + 3]) == sdrxbuf[sdrxbuf[i + 3] + 4])) {

          switch (sdrxbuf[i + 2]) {

          case OUT_RAW_ACC:
            memcpy(&imu.acc, &sdrxbuf[i + 3], 6);
            break;

          case OUT_RAW_GYRO:
            memcpy(&imu.gyro, &sdrxbuf[i + 3], 6);
            break;

          case OUT_RAW_MAG:
            memcpy(&imu.mag, &sdrxbuf[i + 3], 6);
            break;

          case OUT_EULER:
            memcpy(&imu.euler, &sdrxbuf[i + 3], 6);
            break;

          case OUT_QUAD:
            memcpy(&imu.quad, &sdrxbuf[i + 3], 8);
            break;

          default:
            break;

          }

          i += 10;

        } else {

          i++;

        }

  }

}


void initEuler(void) {

  memset((void*)sdtxbuf, 0, sizeof(sdtxbuf));

  sdtxbuf[0] = IMUTXFRAMEHEAD;
  sdtxbuf[1] = OUT_EULER;
  sdtxbuf[2] = getchecksum(sdtxbuf, 2);

  sdWriteTimeout(IMUDRIVER, sdtxbuf, 3, 20);

}

static THD_WORKING_AREA(imuThd_wa, 512);
static THD_FUNCTION(imuThd, arg) {

  (void)arg;

  memset((void*) sdrxbuf, 0, SERIAL_BUFFERS_SIZE);

  static const eventflags_t serial_wkup_flags =                     //Partially from SD driver
    CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR |       //Partially inherited from IO queue driver
    SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR |
    SD_BREAK_DETECTED;

  event_listener_t serial_listener;
  static eventflags_t pending_flags;
  static eventflags_t current_flag;
  chEvtRegisterMaskWithFlags(chnGetEventSource(IMUDRIVER), &serial_listener,
                             SERIAL_EVT_MASK, serial_wkup_flags);   //setup event listening

  while (!chThdShouldTerminateX()) {

    if (chEvtWaitAnyTimeout(1,1000)) {                                                //wait for selected serial events
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
                      datalength = sdAsynchronousRead(IMUDRIVER, &sdrxbuf,
                                                     (size_t)SERIAL_BUFFERS_SIZE);  //Non-blocking data read
                      imudecode();
                  }

                  FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case CHN_DISCONNECTED:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case SD_NOISE_ERROR:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case SD_PARITY_ERROR:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case SD_FRAMING_ERROR:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case SD_OVERRUN_ERROR:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              case SD_BREAK_DETECTED:
              FLUSH_I_QUEUE(IMUDRIVER);
                  break;

              default:
                  break;

          }

      } while (pending_flags && !foundheader);

      FLUSH_I_QUEUE(IMUDRIVER);
      memset((void*)sdrxbuf, 0, SERIAL_BUFFERS_SIZE);               //Flush RX buffer

    } else {

      initEuler();

    }
  }

}

void imuInit(void) {

  sdStart(IMUDRIVER, &SerialCfg);

  memset((void*) &imu, 0, sizeof(imu_t));

  chThdCreateStatic(imuThd_wa, sizeof(imuThd_wa),
                    NORMALPRIO + 4, imuThd, NULL);

}

