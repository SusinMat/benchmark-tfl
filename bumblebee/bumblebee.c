#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int set_interface_attribs(int fd, int speed, int parity)
{
	struct termios tty;
	if (tcgetattr(fd, &tty) != 0) {
		printf("error %d from tcgetattr\n", errno);
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars

	// disable IGNBRK for mismatched speed tests; otherwise receive break as \000 chars
	tty.c_iflag &= ~IGNBRK;     // disable break processing
	tty.c_lflag = 0;            // no signaling chars , no echo,
	                            // no canonical processing
	tty.c_oflag = 0;            // no remapping, no delays
	tty.c_cc[VMIN] = 0;	    // read doesn't block
	tty.c_cc[VTIME] = 0;        // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls, enable readingA

	tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("error %d from tcsetattr\n", errno);
		return -1;
	}


	return 0;
}

void set_blocking(int fd, bool should_block)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		printf("error %d from tcgetattr\n", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
		printf("error %d setting term attributes\n", errno);
}

int main(int argc, char **argv)
{
	printf("This is\n"
			"<B><U><M><B><L><E><B><E><E\n");

	char *portname = "/dev/ttyUSB1";
	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		printf("error %d opening %s: %s\n", errno, portname, strerror(errno));
		return -1;
	}

	set_interface_attribs(fd, B115200, 0x0); // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking(fd, false);                 // set no blocking

	// write(fd, "hello!\n", 7);                // send 7 character greeting

	// usleep((7 + 25) * 100);                  // sleep enough to transmit the 7 plus
	// receive 25:  approx 100 uS per char transmit
	char buf[100];
	int n = read(fd, buf, sizeof buf);       // read up to 100 characters if ready to read
	return 0;
}
