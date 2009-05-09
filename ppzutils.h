#ifndef PPZ_UTILS_H
#define PPZ_UTILS_H

#include <inttypes.h>

#define DL_BOOZ2_AHRS_EULER_imu_phi(_payload) ((int32_t)(*((uint8_t*)_payload+2)|*((uint8_t*)_payload+2+1)<<8|((uint32_t)*((uint8_t*)_payload+2+2))<<16|((uint32_t)*((uint8_t*)_payload+2+3))<<24))
#define DL_BOOZ2_AHRS_EULER_imu_theta(_payload) ((int32_t)(*((uint8_t*)_payload+6)|*((uint8_t*)_payload+6+1)<<8|((uint32_t)*((uint8_t*)_payload+6+2))<<16|((uint32_t)*((uint8_t*)_payload+6+3))<<24))
#define DL_BOOZ2_AHRS_EULER_imu_psi(_payload) ((int32_t)(*((uint8_t*)_payload+10)|*((uint8_t*)_payload+10+1)<<8|((uint32_t)*((uint8_t*)_payload+10+2))<<16|((uint32_t)*((uint8_t*)_payload+10+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_phi(_payload) ((int32_t)(*((uint8_t*)_payload+14)|*((uint8_t*)_payload+14+1)<<8|((uint32_t)*((uint8_t*)_payload+14+2))<<16|((uint32_t)*((uint8_t*)_payload+14+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_theta(_payload) ((int32_t)(*((uint8_t*)_payload+18)|*((uint8_t*)_payload+18+1)<<8|((uint32_t)*((uint8_t*)_payload+18+2))<<16|((uint32_t)*((uint8_t*)_payload+18+3))<<24))
#define DL_BOOZ2_AHRS_EULER_body_psi(_payload) ((int32_t)(*((uint8_t*)_payload+22)|*((uint8_t*)_payload+22+1)<<8|((uint32_t)*((uint8_t*)_payload+22+2))<<16|((uint32_t)*((uint8_t*)_payload+22+3))<<24))

#if 0
 <message name="BOOZ2_EMAV_STATE" id="188">   
   <field name="ax"    type="int32" alt_unit="m/s2" alt_unit_coef="0.0009766"/>
   <field name="ay"    type="int32" alt_unit="m/s2" alt_unit_coef="0.0009766"/>
   <field name="az"    type="int32" alt_unit="m/s2" alt_unit_coef="0.0009766"/>
   <field name="gp"    type="int32" alt_unit="deg/s" alt_unit_coef="0.0139882"/>
   <field name="gq"    type="int32" alt_unit="deg/s" alt_unit_coef="0.0139882"/>
   <field name="gr"    type="int32" alt_unit="deg/s" alt_unit_coef="0.0139882"/>
   <field name="body_phi"   type="int32" alt_unit="degres" alt_unit_coef="0.0139882"/>
   <field name="body_theta" type="int32" alt_unit="degres" alt_unit_coef="0.0139882"/>
   <field name="body_psi"   type="int32" alt_unit="degres" alt_unit_coef="0.0139882"/>
 </message>

#define DOWNLINK_SEND_BOOZ2_EMAV_STATE(ax, ay, az, gp, gq, gr, body_phi, body_theta, body_psi){ \
	if (DownlinkCheckFreeSpace(DownlinkSizeOf(0+4+4+4+4+4+4+4+4+4))) {\
	  DownlinkCountBytes(DownlinkSizeOf(0+4+4+4+4+4+4+4+4+4)); \
	  DownlinkStartMessage("BOOZ2_EMAV_STATE", DL_BOOZ2_EMAV_STATE, 0+4+4+4+4+4+4+4+4+4) \
	  DownlinkPutInt32ByAddr((ax)); \
	  DownlinkPutInt32ByAddr((ay)); \
	  DownlinkPutInt32ByAddr((az)); \
	  DownlinkPutInt32ByAddr((gp)); \
	  DownlinkPutInt32ByAddr((gq)); \
	  DownlinkPutInt32ByAddr((gr)); \
	  DownlinkPutInt32ByAddr((body_phi)); \
	  DownlinkPutInt32ByAddr((body_theta)); \
	  DownlinkPutInt32ByAddr((body_psi)); \
	  DownlinkEndMessage() \
	} else \
	  DownlinkOverrun(); \
}

#define DL_BOOZ2_EMAV_STATE_ax(_payload) ((int32_t)(*((uint8_t*)_payload+2)|*((uint8_t*)_payload+2+1)<<8|((uint32_t)*((uint8_t*)_payload+2+2))<<16|((uint32_t)*((uint8_t*)_payload+2+3))<<24))
#define DL_BOOZ2_EMAV_STATE_ay(_payload) ((int32_t)(*((uint8_t*)_payload+6)|*((uint8_t*)_payload+6+1)<<8|((uint32_t)*((uint8_t*)_payload+6+2))<<16|((uint32_t)*((uint8_t*)_payload+6+3))<<24))
#define DL_BOOZ2_EMAV_STATE_az(_payload) ((int32_t)(*((uint8_t*)_payload+10)|*((uint8_t*)_payload+10+1)<<8|((uint32_t)*((uint8_t*)_payload+10+2))<<16|((uint32_t)*((uint8_t*)_payload+10+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gp(_payload) ((int32_t)(*((uint8_t*)_payload+14)|*((uint8_t*)_payload+14+1)<<8|((uint32_t)*((uint8_t*)_payload+14+2))<<16|((uint32_t)*((uint8_t*)_payload+14+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gq(_payload) ((int32_t)(*((uint8_t*)_payload+18)|*((uint8_t*)_payload+18+1)<<8|((uint32_t)*((uint8_t*)_payload+18+2))<<16|((uint32_t)*((uint8_t*)_payload+18+3))<<24))
#define DL_BOOZ2_EMAV_STATE_gr(_payload) ((int32_t)(*((uint8_t*)_payload+22)|*((uint8_t*)_payload+22+1)<<8|((uint32_t)*((uint8_t*)_payload+22+2))<<16|((uint32_t)*((uint8_t*)_payload+22+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_phi(_payload) ((int32_t)(*((uint8_t*)_payload+26)|*((uint8_t*)_payload+26+1)<<8|((uint32_t)*((uint8_t*)_payload+26+2))<<16|((uint32_t)*((uint8_t*)_payload+26+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_theta(_payload) ((int32_t)(*((uint8_t*)_payload+30)|*((uint8_t*)_payload+30+1)<<8|((uint32_t)*((uint8_t*)_payload+30+2))<<16|((uint32_t)*((uint8_t*)_payload+30+3))<<24))
#define DL_BOOZ2_EMAV_STATE_body_psi(_payload) ((int32_t)(*((uint8_t*)_payload+34)|*((uint8_t*)_payload+34+1)<<8|((uint32_t)*((uint8_t*)_payload+34+2))<<16|((uint32_t)*((uint8_t*)_payload+34+3))<<24))
#endif

#define STX 0x99
#define AHRS_MSG_ID 156
#define AHRS_PAYLOAD_LEN 26

#define CSV_HEADER "imu_phi, imu_theta, imu_psi, body_phi, body_theta, body_psi\n"
#define CSV_FORMAT "%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f\n"

#endif /* PPZ_UTILS_H */
