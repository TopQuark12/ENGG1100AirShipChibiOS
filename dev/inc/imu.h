/*
 * imu.h
 *
 *  Created on: 27 Apr 2018
 *      Author: Alex's Desktop
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#define IMUDRIVER       &SD3

#define IMUTXFRAMEHEAD  0xA5
#define IMURXFRAMEHEAD  0x5A

#define TOGGLE_ACC      0x51
#define TOGGLE_GYRO     0x52
#define TOGGLE_MAG      0x53

#define CAL_IMU         0x57
#define CAL_MAG         0x58
#define CAL_RESET       0x59

#define RATE_50HZ       0xA4
#define RATE_100HZ      0xA5
#define RATE_200HZ      0xA6

#define OUT_RAW_ACC     0x15
#define OUT_RAW_GYRO    0x25
#define OUT_RAW_MAG     0x35
#define OUT_EULER       0x45
#define OUT_EULER_CHAR  0x55
#define OUT_QUAD        0x65

typedef struct euler_t{

  int16_t roll_deg;
  int16_t pit_deg;
  int16_t yaw_deg;

}__attribute__((packed)) euler_t;

typedef struct three_dimension_t{

  int16_t x;
  int16_t y;
  int16_t z;

}__attribute__((packed)) three_dimension_t;

typedef struct quad_t{

  int16_t q0;
  int16_t q1;
  int16_t q2;
  int16_t q3;

}__attribute__((packed)) quad_t;

typedef struct imu_t{

  three_dimension_t acc;
  three_dimension_t gyro;
  three_dimension_t mag;
  euler_t euler;
  quad_t quad;

}__attribute__((packed)) imu_t;

void imuInit(void);

#endif /* INC_IMU_H_ */
