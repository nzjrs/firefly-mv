#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"
#include "serial.h"
#include "ppzutils.h"

#define SERIAL_PORT "/dev/ttyUSB1"
#define SERIAL_BAUD 115200

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    unsigned int serial;
    pthread_t thread;
    PprzParser_t parser;
    char data[MESSAGE_LENGTH_EMAV_STATE];

    parser.debug = 0;
    parser.serial = serial_open(SERIAL_PORT, SERIAL_BAUD);
    serial_set_blocking(parser.serial);
    pthread_create( &thread, NULL, parse_pppz_thread, (void*) &parser);

    while(1)
    {
        sleep(1);
        //pthread_mutex_lock( &mutex );
        //memcpy(data, parser.data, MESSAGE_LENGTH_EMAV_STATE);      
        //pthread_mutex_unlock( &mutex );
    }

    // kill thread
    parser.finished = 1;
    pthread_join(thread, NULL);
    fclose(fp);
	return 0;
}

