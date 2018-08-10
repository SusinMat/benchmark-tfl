#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define MILLION 1000000
#define BILLION 1000000000

sem_t sem_r, sem_w;
struct timespec time_shared, time_shared2;
char *buffer_shared, *buffer_shared2;
bool alternate = false;

int64_t get_time()
{
	struct timespec time;
	int64_t ms = 0;

	clock_gettime(CLOCK_MONOTONIC, &time);
	ms = time.tv_sec * 1000 + time.tv_nsec / MILLION;

	return ms;
}

void *writer_thread(void *v)
{
	while (true) {
		sem_wait(&sem_w);
		if(!alternate) {
			printf("MONOTONIC_CLOCK: %010ld.%03ld s\n", time_shared.tv_sec, time_shared.tv_nsec / MILLION);
			printf("%s\n", buffer_shared);
		} else {
			printf("MONOTONIC_CLOCK: %010ld.%03ld s\n", time_shared2.tv_sec, time_shared2.tv_nsec / MILLION);
			printf("%s\n", buffer_shared2);
		}
		sem_post(&sem_r);
	}
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
	const int record_size = 7;
	const int record_max = 1000;
	const int buf_size = record_max * record_size;
	char buf[buf_size];
	char buf2[buf_size];
	buffer_shared = buf;
	buffer_shared2 = buf2;
	int cursor_position = 0;
	char *portname = "/dev/ttyUSB0";
	int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
	struct timespec time;
	pthread_t writer;

	// printf("      This      is\n""<B><U><M><B><L><E><B><E><E>\n");
	if (fd < 0) {
		printf("error %d opening %s: %s\n", errno, portname, strerror(errno));
		return -1;
	}

	set_interface_attribs(fd, B115200, 0x0); // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking(fd, true);                  // set no blocking

	// write(fd, "hello!\n", 7);                // send 7 character greeting
	// usleep((7 + 25) * 100);                  // sleep enough to transmit the 7 plus
	// receive 25:  approx 100 uS per char transmit

	sem_init(&sem_w, 0, 0);
	sem_init(&sem_r, 0, 1);
	pthread_create(&writer, NULL, writer_thread, NULL);

	for (;;) {
		if(alternate) {
			clock_gettime(CLOCK_MONOTONIC, &time_shared);
			for (cursor_position = 0; cursor_position < buf_size - record_size * 2; cursor_position += record_size) {
				int bytes_read = 0;
				while (bytes_read < record_size) {
					// read up to 3rd_arg characters if ready to read
					int n = read(fd, buf + cursor_position + bytes_read, record_size - bytes_read);
					bytes_read += n;
				}
			}
			buf[cursor_position] = '\0';
		} else {
			clock_gettime(CLOCK_MONOTONIC, &time_shared2);
			for (cursor_position = 0; cursor_position < buf_size - record_size * 2; cursor_position += record_size) {
				int bytes_read = 0;
				while (bytes_read < record_size) {
					// read up to 3rd_arg characters if ready to read
					int n = read(fd, buf2 + cursor_position + bytes_read, record_size - bytes_read);
					bytes_read += n;
				}
			}
			buf2[cursor_position] = '\0';
		}
		sem_wait(&sem_r);
		alternate = !alternate;
		sem_post(&sem_w);
	}

	return 0;
}

// vim: set ts=4 sts=4 sw=4 noet :
