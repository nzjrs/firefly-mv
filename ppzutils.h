#ifndef PPZ_UTILS_H
#define PPZ_UTILS_H

#include <inttypes.h>

#define DL_BOOZ2_AHRS_EULER_imu_phi(_payload) ((int32_t)(*((uint8_t*)_payload+2)|*((uint8_t*)_payload+2+1)<<8|((uint32_t)*((uint8_t*)_payload+2+2))<<16|((uint32_t)*((uint8_t*)_payload+2+3))<<24))
#define DL_BOOZ2_AHRS_EULER_imu_theta(_payload) ((int32_t)(*((uint8_t*)_payload+6)|*((uint8_t*)_payload+6+1)<<8|((uint32_t)*((uint8_t*)_payload+6+2))<<16|((uint32_t)*((uint8_t*)_payload+6+3))<<24))
#define DL_BOOZ2_AHRS_EULER_imu_psi(_payload) ((int32_t)(*((uint8_t*)_payload+10)|*((uint8_t*)_payload+10+1)<<8|((uint32_t)*((uint8_t*)_payload+10+2))<<16|((uint32_t)*((uint8_t*)_payload+10+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_phi(_payload) ((int32_t)(*((uint8_t*)_payload+14)|*((uint8_t*)_payload+14+1)<<8|((uint32_t)*((uint8_t*)_payload+14+2))<<16|((uint32_t)*((uint8_t*)_payload+14+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_theta(_payload) ((int32_t)(*((uint8_t*)_payload+18)|*((uint8_t*)_payload+18+1)<<8|((uint32_t)*((uint8_t*)_payload+18+2))<<16|((uint32_t)*((uint8_t*)_payload+18+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_psi(_payload) ((int32_t)(*((uint8_t*)_payload+22)|*((uint8_t*)_payload+22+1)<<8|((uint32_t)*((uint8_t*)_payload+22+2))<<16|((uint32_t)*((uint8_t*)_payload+22+3))<<24))

#define STX 0x99
#define AHRS_MSG_ID 156
#define AHRS_PAYLOAD_LEN 26

#define CSV_HEADER "imu_phi, imu_theta, imu_psi, body_phi, body_theta, body_psi\n"
#define CSV_FORMAT "%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f\n"

#endif /* PPZ_UTILS_H */
