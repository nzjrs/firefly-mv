#ifndef PPZ_UTILS_H
#define PPZ_UTILS_H

#include <inttypes.h>

#define DL_BOOZ2_EMAV_STATE_ax(_payload) ((int32_t)(*((uint8_t*)_payload+2)|*((uint8_t*)_payload+2+1)<<8|((uint32_t)*((uint8_t*)_payload+2+2))<<16|((uint32_t)*((uint8_t*)_payload+2+3))<<24))
#define DL_BOOZ2_EMAV_STATE_ay(_payload) ((int32_t)(*((uint8_t*)_payload+6)|*((uint8_t*)_payload+6+1)<<8|((uint32_t)*((uint8_t*)_payload+6+2))<<16|((uint32_t)*((uint8_t*)_payload+6+3))<<24))
#define DL_BOOZ2_EMAV_STATE_az(_payload) ((int32_t)(*((uint8_t*)_payload+10)|*((uint8_t*)_payload+10+1)<<8|((uint32_t)*((uint8_t*)_payload+10+2))<<16|((uint32_t)*((uint8_t*)_payload+10+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gp(_payload) ((int32_t)(*((uint8_t*)_payload+14)|*((uint8_t*)_payload+14+1)<<8|((uint32_t)*((uint8_t*)_payload+14+2))<<16|((uint32_t)*((uint8_t*)_payload+14+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gq(_payload) ((int32_t)(*((uint8_t*)_payload+18)|*((uint8_t*)_payload+18+1)<<8|((uint32_t)*((uint8_t*)_payload+18+2))<<16|((uint32_t)*((uint8_t*)_payload+18+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gr(_payload) ((int32_t)(*((uint8_t*)_payload+22)|*((uint8_t*)_payload+22+1)<<8|((uint32_t)*((uint8_t*)_payload+22+2))<<16|((uint32_t)*((uint8_t*)_payload+22+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_phi(_payload) ((int32_t)(*((uint8_t*)_payload+26)|*((uint8_t*)_payload+26+1)<<8|((uint32_t)*((uint8_t*)_payload+26+2))<<16|((uint32_t)*((uint8_t*)_payload+26+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_theta(_payload) ((int32_t)(*((uint8_t*)_payload+30)|*((uint8_t*)_payload+30+1)<<8|((uint32_t)*((uint8_t*)_payload+30+2))<<16|((uint32_t)*((uint8_t*)_payload+30+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_psi(_payload) ((int32_t)(*((uint8_t*)_payload+34)|*((uint8_t*)_payload+34+1)<<8|((uint32_t)*((uint8_t*)_payload+34+2))<<16|((uint32_t)*((uint8_t*)_payload+34+3))<<24))

#define STX 0x99
#define AHRS_MSG_ID 0xBC
#define AHRS_PAYLOAD_LEN (2+4+4+4+4+4+4+4+4+4)

#define CSV_HEADER "ax, ay, az, gp, gq, gr, body_phi, body_theta, body_psi\n"
#define CSV_FORMAT "%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f\n"

#endif /* PPZ_UTILS_H */
