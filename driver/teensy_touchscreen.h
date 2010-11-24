#define TEENSY_TOUCHSCREEN_VENDOR_ID  0x16C0    /* Manufacturer’s Vendor ID */
#define TEENSY_TOUCHSCREEN_PRODUCT_ID 0x0FFF    /* Device’s Product ID */

#define BUF_SIZE 8

struct teensy_touchscreen {
    char name[128];
    char phys[64];
    struct usb_device * usbdev;
    struct input_dev * dev;
    struct urb * irq;
    signed char * data;
}

