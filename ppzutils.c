#include "ppzutils.h"
#include "serial.h"

#include <stdio.h>
#include <pthread.h>

#define UPDATE_CHECKSUM(_msg, _x)           \
    _msg->ck_a += _x;                       \
    _msg->ck_b += _msg->ck_a;               \

#define ADD_CHAR(_msg, _x)                  \
    _msg->payload[_msg->idx] = _x;          \
    _msg->idx++;

void ppz_parse_serial (PprzParser_t *parser)
{
    uint8_t c;
    int i;

    i = serial_read_port(parser->serial, &c, 1);
    //if (i != 1)
    //    printf("---------------------------");

    if (i) {
        if (parser->debug)
            printf("%x %d\n", c, parser->status);

        //Adapted from pprz_transport.h
        switch (parser->status) {
            case STATE_UNINIT:
                if (c == STX)
                    parser->status++;
                    parser->ck_a = STX;
                    parser->ck_b = STX;
                break;
            case STATE_GOT_STX:
                if (parser->pprz_msg_received) {
                    parser->pprz_ovrn++;
                    goto error;
                }
                /* Counting STX, LENGTH, ACID, MSGID, CRC1 and CRC2 */
                parser->pprz_payload_len =  c - COMM_NUM_NON_PAYLOAD_BYTES;
                parser->idx = 0;
                UPDATE_CHECKSUM(parser, c)
                parser->status++;
                break;
            case STATE_GOT_LENGTH:
                parser->acid = c;
                UPDATE_CHECKSUM(parser, c)
                parser->status++;
                break;
            case STATE_GOT_ACID:
                parser->msgid = c;
                UPDATE_CHECKSUM(parser, c)
                if (parser->pprz_payload_len == 0)
                    parser->status = STATE_GOT_PAYLOAD;
                else
                    parser->status++;
                break;
            case STATE_GOT_MSGID:
                ADD_CHAR(parser, c)
                UPDATE_CHECKSUM(parser, c)
                if (parser->idx == parser->pprz_payload_len)
                    parser->status++;
                break;
            case STATE_GOT_PAYLOAD:
                if (c != parser->ck_a)
                    goto error;
                parser->status++;
                break;
            case STATE_GOT_CRC1:
                if (c != parser->ck_b)
                    goto error;
                parser->pprz_msg_received = 1;
                goto restart;
            default:
                //printf(".............................");
                break;
        }
    return;
    error:
        parser->pprz_error++;
        //printf("!");
    restart:
        parser->status = STATE_UNINIT;
    }
    return;
}

/* FIXME... */
extern mutex;

void *parse_pppz_thread(void *ptr)
{
    PprzParser_t *parser = (PprzParser_t *)ptr;

    parser->status = 0;
    parser->pprz_msg_received = 0;
    parser->finished = 0;
    parser->num = 0;
    parser->ignored = 0;
    parser->pprz_ovrn = 0;
    parser->pprz_error = 0;

    if (parser->serial == -1)
        return 0;

    while(!parser->finished)
    {
        ppz_parse_serial (parser);
        if (parser->pprz_msg_received) {
            parser->num++;
            parser->pprz_msg_received = 0;

            if (parser->msgid ==  MESSAGE_ID_EMAV_STATE && parser->pprz_payload_len == MESSAGE_LENGTH_EMAV_STATE) {
                parser_print(parser);
                pthread_mutex_lock( &mutex );
                memcpy(parser->data, parser->payload, parser->pprz_payload_len);
                pthread_mutex_unlock( &mutex );
            } else {
                parser->ignored++;
            }
        }
    }

    close(parser->serial);
    return 0;
}

void parser_print(PprzParser_t *parser)
{
    printf("R: %2.2f P: %2.2f Y: %2.2f Az: %2.2f Gq: %2.2f\n",
        (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_phi(parser->payload) * 0.0139882),
        (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_theta(parser->payload) * 0.0139882),
        (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_psi(parser->payload) * 0.0139882),
        (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_az(parser->payload) * 0.0009766),
        (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gr(parser->payload) * 0.0139882));
}
