#ifndef PPZ_UTILS_H
#define PPZ_UTILS_H

#include <inttypes.h>
#include <glib.h>

/* FIXME: Should include the generated file directly */

#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_ax(_payload) (int32_t)(*((uint8_t*)_payload+0)|*((uint8_t*)_payload+0+1)<<8|((uint32_t)*((uint8_t*)_payload+0+2))<<16|((uint32_t)*((uint8_t*)_payload+0+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_ay(_payload) (int32_t)(*((uint8_t*)_payload+4)|*((uint8_t*)_payload+4+1)<<8|((uint32_t)*((uint8_t*)_payload+4+2))<<16|((uint32_t)*((uint8_t*)_payload+4+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_az(_payload) (int32_t)(*((uint8_t*)_payload+8)|*((uint8_t*)_payload+8+1)<<8|((uint32_t)*((uint8_t*)_payload+8+2))<<16|((uint32_t)*((uint8_t*)_payload+8+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gp(_payload) (int32_t)(*((uint8_t*)_payload+12)|*((uint8_t*)_payload+12+1)<<8|((uint32_t)*((uint8_t*)_payload+12+2))<<16|((uint32_t)*((uint8_t*)_payload+12+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gq(_payload) (int32_t)(*((uint8_t*)_payload+16)|*((uint8_t*)_payload+16+1)<<8|((uint32_t)*((uint8_t*)_payload+16+2))<<16|((uint32_t)*((uint8_t*)_payload+16+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gr(_payload) (int32_t)(*((uint8_t*)_payload+20)|*((uint8_t*)_payload+20+1)<<8|((uint32_t)*((uint8_t*)_payload+20+2))<<16|((uint32_t)*((uint8_t*)_payload+20+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_phi(_payload) (int32_t)(*((uint8_t*)_payload+24)|*((uint8_t*)_payload+24+1)<<8|((uint32_t)*((uint8_t*)_payload+24+2))<<16|((uint32_t)*((uint8_t*)_payload+24+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_theta(_payload) (int32_t)(*((uint8_t*)_payload+28)|*((uint8_t*)_payload+28+1)<<8|((uint32_t)*((uint8_t*)_payload+28+2))<<16|((uint32_t)*((uint8_t*)_payload+28+3))<<24)
#define MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_psi(_payload) (int32_t)(*((uint8_t*)_payload+32)|*((uint8_t*)_payload+32+1)<<8|((uint32_t)*((uint8_t*)_payload+32+2))<<16|((uint32_t)*((uint8_t*)_payload+32+3))<<24)

#define STX 0x99
#define MESSAGE_ID_EMAV_STATE 50
#define MESSAGE_LENGTH_EMAV_STATE (0+4+4+4+4+4+4+4+4+4)
#define COMM_NUM_NON_PAYLOAD_BYTES 6

#define CSV_HEADER "ax, ay, az, gp, gq, gr, body_phi, body_theta, body_psi\n"
#define CSV_FORMAT "%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f\n"

#define DEFAULT_SERIAL_PORT "/dev/ttyUSB0"
#define DEFAULT_SERIAL_BAUD 115200


typedef enum {
    STATE_UNINIT,
    STATE_GOT_STX,
    STATE_GOT_LENGTH,
    STATE_GOT_ACID,
    STATE_GOT_MSGID,
    STATE_GOT_PAYLOAD,
    STATE_GOT_CRC1
} ParseState_t;

typedef struct __parser
{
    ParseState_t status;
    int num;
    int ignored;
    uint8_t pprz_payload_len;
    uint8_t idx;
    uint8_t pprz_msg_received;
    int pprz_ovrn;
    int pprz_error;
    char payload[256];
    uint8_t acid;
    uint8_t msgid;
    uint8_t ck_a;
    uint8_t ck_b;

    /* Thread communication */
    uint8_t finished;
    char data[MESSAGE_LENGTH_EMAV_STATE];
    int serial;

    uint8_t debug;

    uint64_t timestamp;
} PprzParser_t;

typedef struct __record
{
    char data[MESSAGE_LENGTH_EMAV_STATE];
    uint64_t    timestamp;
} PprzRecord_t;

#define PPRZ_RECORD_SIZE sizeof(PprzRecord_t)

/**
 * Function and macro to setup camera from GOption command line arguments
 */
#define GOPTION_ENTRY_SERIAL_SETUP_ARGUMENTS(_port, _speed)                                     \
      { "port", 'p', 0, G_OPTION_ARG_STRING, _port, "Serial port", "/dev/ttyUSB0" },            \
      { "speed", 's', 0, G_OPTION_ARG_INT, _speed, "Serial baud", "115200" }

void ppz_parse_serial (PprzParser_t *parser);
void *parse_pppz_thread(void *ptr);
void parser_print_buffer(char *data);
void parser_print(PprzParser_t *parser);

#endif /* PPZ_UTILS_H */
