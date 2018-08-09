#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

int64_t get_time()
{
	struct timespec time;
	int64_t ms = 0;

	clock_gettime(CLOCK_MONOTONIC, &time);
	ms = time.tv_sec * 1000 + time.tv_nsec / 1000000;

	return ms;
}

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
	const int record_size = 8;
	const int buf_size = 1000 * record_size;
	char buf[buf_size];
	int cursor_position = 0;
	char *portname = "/dev/ttyUSB0";
	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	struct timespec time;
	// printf("This is\n""<B><U><M><B><L><E><B><E><E>\n");
	if (fd < 0) {
		printf("error %d opening %s: %s\n", errno, portname, strerror(errno));
		return -1;
	}

	set_interface_attribs(fd, B115200, 0x0); // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking(fd, true);                 // set no blocking

	// write(fd, "hello!\n", 7);                // send 7 character greeting
	// usleep((7 + 25) * 100);                  // sleep enough to transmit the 7 plus

	// receive 25:  approx 100 uS per char transmit
	do {
		clock_gettime(CLOCK_MONOTONIC, &time);
		printf("MONOTONIC_CLOCK: %ld.%ld s\n", time.tv_sec, time.tv_nsec / 1000000);
		// printf("MONOTONIC_CLOCK: %"PRId64" ms\n", get_time());
		for (cursor_position = 0; cursor_position < buf_size - 1; cursor_position += record_size) {
			int n = read(fd, buf + cursor_position, sizeof buf);       // read up to 3rd_arg characters if ready to read
			// printf("Bytes read: %d\n", n);
		}
		buf[cursor_position] = '\0';
		printf("%s\n", buf);
	} while (true);

	return 0;
}
