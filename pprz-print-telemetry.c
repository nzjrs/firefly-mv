#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <glib.h>

#include "utils.h"
#include "serial.h"
#include "ppzutils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    unsigned int    serial;
    pthread_t       thread;
    PprzParser_t    parser;
    char            data[MESSAGE_LENGTH_EMAV_STATE];

    char            *serial_port;
    int             serial_baud;

    /* Option parsing */
    GError          *error = NULL;
    GOptionContext  *context;
    GOptionEntry    entries[] =
    {
      GOPTION_ENTRY_SERIAL_SETUP_ARGUMENTS(&serial_port, &serial_baud),
      { NULL }
    };

    context = g_option_context_new("- Print telemetry");
    g_option_context_add_main_entries (context, entries, NULL);

    /* Defaults */
    serial_port = DEFAULT_SERIAL_PORT;
    serial_baud = DEFAULT_SERIAL_BAUD;

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        printf( "Error: %s\n%s", 
                error->message, 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (serial_port == NULL || serial_baud < 0)
    {
        printf( "Error: You must supply a serial port and speed\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }

    parser.debug = 0;
    parser.serial = serial_open(serial_port, serial_baud);
    serial_set_blocking(parser.serial);
    pthread_create( &thread, NULL, parse_pppz_thread, (void*) &parser);

    while(1)
    {
        sleep(1);
    }

    // kill thread
    parser.finished = 1;
    pthread_join(thread, NULL);
	return 0;
}

