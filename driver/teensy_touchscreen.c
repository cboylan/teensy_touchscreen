#define TEENSY_TOUCHSCREEN_VENDOR_ID  0x16C0    /* Manufacturer’s Vendor ID */
#define TEENSY_TOUCHSCREEN_PRODUCT_ID 0x0FFF    /* Device’s Product ID */

/* USB ID Table specifying the devices that this driver supports */
static struct usb_device_id teensy_ids[] = {
    { USB_DEVICE(TEENSY_TOUCHSCREEN_VENDOR_ID, TEENSY_TOUCHSCREEN_PRODUCT_ID) },
    { } /* Terminate */
};

MODULE_DEVICE_TABLE(usb, tele_ids);

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
usb_tele_init(void)
{
    /* Register with the USB core */
    result = usb_register(&tele_driver);

    /* ... */

    return 0;
}

/* Module Exit */
static void __exit
usb_tele_exit(void)
{
    /* Unregister from the USB core */
    usb_deregister(&tele_driver);

    return;
}

module_init(usb_tele_init);
module_exit(usb_tele_exit);
