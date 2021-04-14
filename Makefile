#Simple makefile for libusb application
# this is a proper pound sign! jolly good!

PKG_CONFIG=/media/slash/project/home/slash/project/DFI/IMX8MP/sdk/5.4-zeus/sysroots/x86_64-pokysdk-linux/usr/bin/pkg-config

#CC=gcc
#INC=/usr/local/include/libusb-1.0/
OBJ=USB_Linux_HSET.o

USB_Linux_HSET: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB_Linux_HSET `${PKG_CONFIG} --libs --cflags libusb-1.0`

USB_Linux_HSET.o: USB_Linux_HSET.c
	$(CC) -O2 -c USB_Linux_HSET.c `${PKG_CONFIG} --libs --cflags libusb-1.0`

clean :
	-rm *.o $(objects) *.exe USB_Linux_HSET
