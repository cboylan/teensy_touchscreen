/*  Driver for the teensy touchscreen firmware.
    Copyright (C) 2010 Clark Boylan, Nate Georgeson, Chris Noonan, Abdelhalim
    Ragab.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Code adapted from linux/drivers/hid/usbhid/usbmouse.c (the probe,
    disconnect, open, and close calls have been copied and adapted from the
    usbhid mouse driver). */

#define TEENSY_TOUCHSCREEN_VENDOR_ID  0x16C0	/* Manufacturer’s Vendor ID */
#define TEENSY_TOUCHSCREEN_PRODUCT_ID 0x0FFF	/* Device’s Product ID */

#define BUF_SIZE 8

struct teensy_touchscreen {
	char name[128];
	char phys[64];
	struct usb_device * usbdev;
	struct input_dev * dev;
	struct urb * irq;
	signed char * data;
};
