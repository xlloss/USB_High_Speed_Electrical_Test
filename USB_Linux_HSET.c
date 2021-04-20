/*
©  [2018] Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and
any derivatives exclusively with Microchip products. It is your responsibility
to comply with third party license terms applicable to your use of third party
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS,
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE. IN
NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN
ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST
EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU
HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

Author: Connor Chilton <connor.chilton@microchip.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <libusb.h>
#include <semaphore.h>
#include <errno.h>

/* the buffer sizes can exceed the USB MTU */
#define MAX_CTL_XFER	64
#define MAX_BULK_XFER	512


/**
 * struct my_usb_device - struct that ties USB related stuff
 * @dev: pointer libusb devic
 * @dev_handle: device handle for USB devices
 * @ctx: context of the current session
 * @device_desc: structure that holds the device descriptor fields
 * @inbuf: buffer for USB IN communication
 * @outbut: buffer for USB OUT communication
 */

struct libusb_session {
	libusb_device **dev;
	libusb_device_handle *dev_handle;
	libusb_context *ctx;
	struct libusb_device_descriptor device_desc;
	unsigned char inbuf[MAX_CTL_XFER];
	unsigned char outbuf[MAX_BULK_XFER];
	uint16_t wIndex;
	int port_num;
};

static struct libusb_session session;

enum {
	TEST_J = 1,
	TEST_K,
	TEST_SE0_NAK,
	TEST_PACKET,
	TEST_FORCE_ENABLE,
};

int main(int argc, char **argv)
{
	int port, product_id = 0, vendor_id = 0x0424;
	int ret, count, test_mode, test_port;
	char port_select[] = "", test_select[] = "";
	char product_id_input[10];
	char hexset[] = "0123456789ABCDEFabcdef";
	unsigned char *data = 0;
	unsigned int timeout_set = 50000000;
	ssize_t cnt;
	uint8_t bmRequestType = 0x23, bRequest = 0x03;
	uint16_t wValue = 0x0015, wLength = 0x0000;;

INPUT:
	printf("The Microchip HUB Vendor ID : 0x%x\n", vendor_id);
	printf("Please enter the Microchip HUB 2.0 Product ID to Test:\n0x");
	ret = scanf("%s", product_id_input);
	if (strlen(product_id_input) != 4) {
		printf("Please enter 4 digit hex only\n");
		goto INPUT;
	}

	count = strspn(product_id_input, hexset);
	if (count != 4) {
		printf("Please enter 4 digit hex only\n");
		goto INPUT;
	}
	product_id = (int)strtol(product_id_input, NULL, 16);

	printf("Into USB 2.0 test mode on a port\n");
	printf("Press '1' for Test_J\n");
	printf("Press '2' for Test_K\n");
	printf("Press '3' for Test_SE0_NAK\n");
	printf("Press '4' for Test_Packet\n");
	printf("Press '5' for Test_Force_Enable\n");

	printf("Please enter Test Options:\n");
	ret = scanf("%s", test_select);
	if (!ret) {
		printf("Test options select fail, port number : %s\n", test_select);
		return 0;
	}
	test_mode = atoi(test_select);
	if (test_mode < TEST_J || test_mode > TEST_FORCE_ENABLE) {
		printf ("This Test Mode %d is not avaiable\n", test_mode);
		return 0;
	}

	printf("Please enter Test For USB Port Number (1-7):\n");
	ret = scanf("%s", port_select);
	if (!ret) {
		printf("port select fail, port number : %s\n", port_select);
		return 0;
	}

	test_port = atoi(port_select);
	if (test_port < 1 || test_mode > 7) {
		printf ("This Test Port %d is not avaiable\n", test_port);
		return 0;
	}

	session.port_num = test_port;
	session.wIndex = test_mode << 8 | test_port << 0;

	ret = libusb_init(&session.ctx);
	if (ret < 0) {
		printf("Init Error %i occourred.\n", ret);
		return -EIO;
	}

	/* set verbosity level to 3, as suggested in the documentation */
	/* libusb_set_debug(session.ctx, 3); */

	cnt = libusb_get_device_list(session.ctx, &session.dev);
	if (cnt < 0) {
		printf("no device found\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}

	/* open device w/ vendorID and productID */
	printf("Opening device ID %04x:%04x...", vendor_id, product_id);
	session.dev_handle = libusb_open_device_with_vid_pid(session.ctx,
		vendor_id, product_id);
	if (!session.dev_handle) {
		printf("failed/not in list\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}
	printf("ok\n");

	/* free the list, unref the devices in it */
	libusb_free_device_list(session.dev, 1);

	/* find out if a kernel driver is attached */
	if (libusb_kernel_driver_active(session.dev_handle, 0) == 1) {
		printf("Device has kernel driver attached.\n");
		/* detach it */
		if (!libusb_detach_kernel_driver(session.dev_handle, 0))
			printf("Kernel Driver Detached!\n");
	}

	/* Send Endpoint Reflector control transfer */
	ret = libusb_control_transfer(session.dev_handle,
						bmRequestType,
						bRequest,
						wValue,
						session.wIndex,
						data,
						wLength,
						timeout_set);
	if (!ret)
		printf("Port now in test mode!\n");
	else
		printf("Control transfer failed. Error: %d\n", ret);

	/* close the device we opened */
	libusb_close(session.dev_handle);
	libusb_exit(session.ctx);
	printf("Please Reset System for Next USB Test\n");
	return 0;
}
