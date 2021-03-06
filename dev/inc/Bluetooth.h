/*
 * Bluetooth.h
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#define BLUETOOTHDRIVER     &SD2

#define FRAMEHEAD           64
#define FRAMEINDEXER        58
#define FRAMEEND            38
#define NEGATIVECHAR        45

int16_t* getBluetoothCommand(void);

void bluetoothInit(void);

#endif /* INC_BLUETOOTH_H_ */
