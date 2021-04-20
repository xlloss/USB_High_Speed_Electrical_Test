#Simple makefile for libusb application
# this is a proper pound sign! jolly good!

PKG_CONFIG=${OECORE_NATIVE_SYSROOT}/usr/bin/pkg-config

OBJ=USB_Linux_HSET.o
USB_Linux_HSET: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB_Linux_HSET `${PKG_CONFIG} --libs --cflags libusb-1.0`

USB_Linux_HSET.o: USB_Linux_HSET.c
	$(CC) -O2 -c USB_Linux_HSET.c `${PKG_CONFIG} --libs --cflags libusb-1.0`

clean :
	rm *.o USB_Linux_HSET
