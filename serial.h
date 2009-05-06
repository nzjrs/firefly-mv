#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int serial_open(const char *port, unsigned int baud);
unsigned int serial_set_baud(int fd, int baud);
unsigned int serial_set_blocking(int fd);
int serial_read_port(int fd, char *buf, int len);
int serial_write_port(int fd, const char* buf, int len);

#endif //_SERIAL_H_
