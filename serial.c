#include <unistd.h>

#include "serial.h"

#define OLD_WAY 1
#define FORCE_REPEAT_READ_IF_EAGAIN 1

int serial_open(const char *port, unsigned int baud) {
    int fd, ok;

    //Unix serial port guide:
    //http://www.easysw.com/~mike/serial/serial.html
    struct termios config;

    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if ( fd  == -1 ) {
		return -1;
    }
    
    // flush serial port
    if (tcflush(fd, TCIFLUSH) != 0) {
        printf("Error flushing serial port\n");
        close(fd);
        return -1;
    }

    // set required port parameters 
    if ( tcgetattr( fd, &config ) != 0 ) {
		printf("Unable to read port settings\n");
		close(fd);
		return -1;
    }

#if OLD_WAY
    printf("Setting port options (old way)\n");

    cfmakeraw( &config );
    // http://www.gnu.org/software/libtool/manual/libc/Noncanonical-Input.html
    // cfmakeraw does exactly this:
    //   termios-p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    //   termios-p->c_oflag &= ~OPOST;
    //   termios-p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    //   termios-p->c_cflag &= ~(CSIZE|PARENB);
    //   termios-p->c_cflag |= CS8;

    // disable software flow control
    config.c_iflag &= ~(IXON|IXOFF|IXANY);

    // enable the receiver and set local mode (no other users)
    config.c_cflag |= (CLOCAL|CREAD);

    // disable hardware flow control
    config.c_cflag &= ~CRTSCTS;
    #ifdef CNEW_RTSCTS
    config.c_cflag &= ~CNEW_RTSCTS;
    #endif

    // 8N1 parity
    config.c_cflag &= ~CSTOPB;
    config.c_cflag |= CS8;
#else
    printf("Setting port options (new way)\n");
    // set to raw terminal type
    config.c_cflag = (CS8 | CLOCAL | CREAD);
    config.c_iflag = (IGNBRK | IGNPAR);
    config.c_oflag = 0;    
#endif

    if ( tcsetattr( fd, TCSANOW, &config ) != 0 ) {
        printf("Unable to update port setting\n");
		return -1;
    }

    ok = serial_set_baud(fd, baud);
    if (!ok)
        return -1;

    printf("Opened %s @ %u baud OK\n", port, baud);
    return fd;
}

unsigned int serial_set_baud(int fd, int baud) {
    struct termios config;
    speed_t speed = B9600;

    if ( tcgetattr( fd, &config ) != 0 ) {
    	printf("Unable to poll port settings\n");
	    return 0;
    }

    switch (baud) {
        case 300:
            speed = B300;
            break;
        case 1200:
        	speed = B1200;
            break;
        case 2400:
            speed = B2400;
            break;
        case 4800:
        	speed = B4800;
            break;
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
        	speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        case 230400:
            speed = B230400;
            break;
        default:
            printf("Unsupported baud rate %u\n", baud);
        	return 0;
    }
    
    if ( cfsetispeed( &config, speed ) != 0 ) {
    	printf("Problem setting input baud rate\n");
	    return 0;
    }

    if ( cfsetospeed( &config, speed ) != 0 ) {
        printf("Problem setting output baud rate\n");
	    return 0;
    }

    if ( tcsetattr( fd, TCSANOW, &config ) != 0 ) {
        printf("Unable to update port settings");
    	return 0;
    }

    return 1;
}

unsigned int serial_set_blocking(int fd) {
    struct termios config;

    // set required port parameters 
    if ( tcgetattr( fd, &config ) != 0 ) {
		printf("Unable to read port settings\n");
		return 0;
    }
    
    //block for at least one charater, 1s timeout
    config.c_cc[VMIN] = 1;
    config.c_cc[VTIME] = 10;

    if ( tcsetattr( fd, TCSANOW, &config ) != 0 ) {
	    printf("Unable to update port settings");
    	return 0;
    }
    
	return 1;
}

int serial_read_port(int fd, char *buf, int len) {
    int count = -1;

#if FORCE_REPEAT_READ_IF_EAGAIN
    // Required for usb-serial ports.....
    while(1) {
        count = read(fd, buf, len);
        if ((errno != EAGAIN) || (count >= 0))
            break;
//        else
//            printf("..................................................");
        usleep(100);
    }
#else
    count = read(fd, buf, len);
#endif

    if ( count < 0 ) {
        if (errno != EAGAIN) {
            printf("Serial I/O error: %s\n", strerror(errno));
        }
        buf[0] = '\0';
        return 0;
    } else
    	return count;
}

int serial_write_port(int fd, const char* buf, int len) {
    int count = write(fd, buf, len);

    if ( (int)count != len ) {
        printf("Serial I/O error: %s\n", strerror(errno));
	}

    return count;
}

