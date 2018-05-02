/*
 * Bluetooth.c
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#include "ch.h"
#include "hal.h"
#include "MotorPWM.h"
#include "Bluetooth.h"

#define FLUSH_I_QUEUE(sdp)      \
    chSysLock();                \
    chIQResetI(&(sdp)->iqueue); \
    chSysUnlock();                          //Flush serial in queue

#define LEAST_SET_BIT(x)        x&(-x)      //Clear all but least set bit

#define ACQTIME                 15           //Milliseconds

#define SERIAL_EVT_MASK         1

static int16_t motorPwrCommand[MOTORNUM];

static uint8_t foundheader = 0;
static uint8_t datalength = 0;
static uint8_t sdrxbuf[SERIAL_BUFFERS_SIZE];

static const SerialConfig SerialCfg = {
  9600,                 //Baud Rate
  USART_CR1_UE,         //CR1 Register
  USART_CR2_LINEN,      //CR2 Register
  0                     //CR3 Register
};

int16_t* getBluetoothCommand(void) {

  return &motorPwrCommand;

}

void bluetoothdecode(void) {

  static bool negative;
  static int16_t power;
  static uint8_t motornum;
  static uint8_t i;

  i = 0;
  while(i < datalength) {                                                   //Scan for frame head
    if ((sdrxbuf[i] == FRAMEHEAD) &&                                  //Verify first byte value
        (sdrxbuf[i + 2] == FRAMEINDEXER)) {

      power = 0;
      negative = 0;
      i++;
      motornum = sdrxbuf[i] - 48;
      i++;
      i++;
      while((sdrxbuf[i] != FRAMEEND) && (i < datalength)) {
        if (sdrxbuf[i] == NEGATIVECHAR) {
          negative = true;
        }
        else {
          power = power * 10 + (sdrxbuf[i] - 48);
        }
        i++;
      }
      power = negative ? -power : power;
      motorPwrCommand[motornum] = power;
    } else {

        i++;

    }
  }

  i = 0;

}

static THD_WORKING_AREA(BluetoothThd_wa, 128);
static THD_FUNCTION(BluetoothThd, arg) {

  (void)arg;

  memset((void*) sdrxbuf, 0, SERIAL_BUFFERS_SIZE);

  static const eventflags_t serial_wkup_flags =                     //Partially from SD driver
    CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR |       //Partially inherited from IO queue driver
    SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR |
    SD_BREAK_DETECTED;

  event_listener_t serial_listener;
  static eventflags_t pending_flags;
  static eventflags_t current_flag;
  chEvtRegisterMaskWithFlags(chnGetEventSource(BLUETOOTHDRIVER), &serial_listener,
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
                    datalength = sdAsynchronousRead(BLUETOOTHDRIVER, &sdrxbuf,
                                                   (size_t)SERIAL_BUFFERS_SIZE);  //Non-blocking data read
                    bluetoothdecode();
                }

                FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case CHN_DISCONNECTED:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case SD_NOISE_ERROR:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case SD_PARITY_ERROR:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case SD_FRAMING_ERROR:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case SD_OVERRUN_ERROR:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            case SD_BREAK_DETECTED:
            FLUSH_I_QUEUE(BLUETOOTHDRIVER);
                break;

            default:
                break;

        }

    } while (pending_flags && !foundheader);

    FLUSH_I_QUEUE(BLUETOOTHDRIVER);
    memset((void*)sdrxbuf, 0, SERIAL_BUFFERS_SIZE);               //Flush RX buffer

  }

}

void bluetoothInit(void) {

  sdStart(BLUETOOTHDRIVER, &SerialCfg);

  memset((void*) motorPwrCommand, 0, sizeof(motorPwrCommand));

  chThdCreateStatic(BluetoothThd_wa, sizeof(BluetoothThd_wa),
                    NORMALPRIO + 5, BluetoothThd, NULL);

}
