/*
 * Tof.h
 *
 *  Created on: 26 Apr 2018
 *      Author: Alex's Desktop
 */

#ifndef INC_TOF_H_
#define INC_TOF_H_

#define ARM_MATH_CM3
#include "arm_math.h"

#define TOFDRIVER           &SD1

#define TOFFRAMEHEAD           38
#define TOFFRAMEEND            64

void tofInit(void);

#endif /* INC_TOF_H_ */
