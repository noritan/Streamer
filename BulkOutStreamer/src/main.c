/*
 * main.c
 *
 *  Created on: Jun 17, 2012
 *      Author: cyj
 */

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <time.h>

static const int 	ENDPOINT 		= 0x01;
static const int	VID				= 0x04b4;
static const int	PID				= 0x00f1;
static const int 	BUFFER_SIZE 	= 256*1024;
static const int	TRANSFER_COUNT	= 16*1024;

long long get_elapsed_usec(struct timeval* end, struct timeval* start) {
	long long elapsed_usec;
	elapsed_usec = (end->tv_sec - start->tv_sec) * 1000000l;
	if (end->tv_usec >= start->tv_usec) {
		elapsed_usec += end->tv_usec - start->tv_usec;
	} else {
		elapsed_usec -= start->tv_usec - end->tv_usec;
	}
	return elapsed_usec;
}

void process_transfer(libusb_device_handle* dev_handle) {
	int nFail = 0;
	int nPackets = 0;
	long long nBytes = 0;
	unsigned char buffer[BUFFER_SIZE];
	int	i;
	struct timeval start;
	struct timeval end;

	gettimeofday(&start, 0);
	for (i = 0; i < TRANSFER_COUNT; i++) {
		int length;
		int status = libusb_bulk_transfer(dev_handle, ENDPOINT, buffer, sizeof buffer,
				&length, 1000);
		if (status < 0) {
			nFail++;
			printf("libusb_bulk_transfer: failed with code=%s\n",
					libusb_error_name(status));
		} else {
			nPackets++;
			nBytes += length;
		}
	}
	gettimeofday(&end, 0);
	long long elapsed_usec = get_elapsed_usec(&end, &start);
	double transfer_rate = (double)nBytes / elapsed_usec;
	printf("Gotten %d packets, %lld Bytes in %lld usec.\n", nPackets, nBytes, elapsed_usec);
	printf("Transfer rate %f MBytes/sec\n", transfer_rate);
}

int main(int ac, char **av) {
	libusb_context *context;
	libusb_device_handle *dev_handle;

	int status = libusb_init(&context);
	if (status) {
		printf ("libusb_init: failed with code=%s\n", libusb_error_name(status));
	} else {
		dev_handle = libusb_open_device_with_vid_pid(context, VID, PID);
		if (!dev_handle) {
			printf ("libusb_open_device_with_vid_pid: failed with code=%s\n", libusb_error_name(status));
		} else {
			process_transfer(dev_handle);
			libusb_close(dev_handle);
		}
		libusb_exit(context);
	}
	return 0;
}
