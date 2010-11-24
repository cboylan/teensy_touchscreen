#include <linux/module.h>
#include <linux/usb.h>

#define TEENSY_TOUCHSCREEN_VENDOR_ID  0x16C0    /* Manufacturer’s Vendor ID */
#define TEENSY_TOUCHSCREEN_PRODUCT_ID 0x0FFF    /* Device’s Product ID */

/* USB ID Table specifying the devices that this driver supports */
static struct usb_device_id teensy_ids[] = {
    { USB_DEVICE(TEENSY_TOUCHSCREEN_VENDOR_ID, TEENSY_TOUCHSCREEN_PRODUCT_ID) },
    { } /* Terminate */
};

MODULE_DEVICE_TABLE(usb, teensy_ids);

static int
teensy_touchscreen_probe(struct usb_interface * interface,
        const struct usb_device_id * id)
{
    return 0;
}

static void
teensy_touchscreen_disconnect(struct usb_interface * interface)
{
}

/* The usb_driver structure for this driver */
static struct usb_driver teensy_touchscreen_driver =
{
    .name = "teensy_touchscreen",                   /* Unique name */
    .probe = teensy_touchscreen_probe,              /* See Listing 11.3 */
    .disconnect = teensy_touchscreen_disconnect,    /* See Listing 11.3 */
    .id_table = teensy_ids,                         /* See above */
};

/* Module Initialization */
static int __init
teensy_touchscreen_init(void)
{
    int result;

    /* Register with the USB core */
    result = usb_register(&teensy_touchscreen_driver);

    /* ... */

    return 0;
}

/* Module Exit */
static void __exit
teensy_touchscreen_exit(void)
{
    /* Unregister from the USB core */
    usb_deregister(&teensy_touchscreen_driver);

    return;
}

module_init(teensy_touchscreen_init);
module_exit(teensy_touchscreen_exit);
