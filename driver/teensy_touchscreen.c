/* Code adapted from Essential Linux Device Drivers and
   linux/drivers/hid/usbhid/usbmouse.c (The probe, disconnect, open, and close
   calls have been copied and adapted from the usbhid mouse driver).
*/

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/input.h>
#include <linux/usb/input.h>

#include "teensy_touchscreen.h"
/* USB ID Table specifying the devices that this driver supports */
static struct usb_device_id teensy_ids[] = {
    { USB_DEVICE(TEENSY_TOUCHSCREEN_VENDOR_ID, TEENSY_TOUCHSCREEN_PRODUCT_ID) },
    { } /* Terminate */
};

MODULE_DEVICE_TABLE(usb, teensy_ids);

static void
teensy_touchscreen_irq(struct urb * urb)
{
}

static int
teensy_touchscreen_open(struct input_dev * dev)
{
    struct teensy_touchscreen * touchscreen = input_get_drvdata(dev);

    touchscreen->irq->dev = touchscreen->usbdev;

    /* Device was opened for use so start running the urb. */
    if (usb_submit_urb(touchscreen->irq, GFP_KERNEL))
        return -EIO;

    return 0;
}

static void
teensy_touchscreen_close(struct input_dev * dev)
{
    struct teensy_touchscreen * touchscreen = input_get_drvdata(dev);

    /* Device was closed so stop running the urb. */
    usb_kill_urb(touchscreen->irq);
}

static int
teensy_touchscreen_probe(struct usb_interface * intf,
        const struct usb_device_id * id)
{
    struct usb_device *dev = interface_to_usbdev(intf);
    struct usb_host_interface * interface;
    struct usb_endpoint_descriptor *endpoint;
    struct teensy_touchscreen * touchscreen;
    struct input_dev *input_dev;
    int pipe;
    int maxp;
    int error = -ENOMEM;

    /* A teensy touchscreen should have a single endpoint. */
    interface = intf->cur_altsetting;
    if (interface->desc.bNumEndpoints != 1){
        return -ENODEV;
    }

    /* This single endpoint should be an interrupt into the host. */
    endpoint = &interface->endpoint[0].desc;
    if (!usb_endpoint_is_int_in(endpoint)){
        return -ENODEV;
    }

    /* Create a piple using the endpoint address. */
    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    /* Determine max packet size. */
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
    printk("teensy_touchscreen: Max packet size is %d.\n", maxp & 0x07FF);

    /* Allocate device specific struct. */
    touchscreen = kzalloc(sizeof(struct teensy_touchscreen), GFP_KERNEL);
    input_dev = input_allocate_device();
    if (!touchscreen || !input_dev){
        goto fail1;
    }

    /* Allocate input buffer. */
    touchscreen->data = kzalloc(BUF_SIZE * sizeof(signed char), GFP_KERNEL);
    if (!touchscreen->data){
        goto fail1;
    }

    /* Allocate urb to receive data from the teensy. No isochronous transfers are used. */
    touchscreen->irq = usb_alloc_urb(0, GFP_KERNEL);
    if (!touchscreen->irq){
        goto fail2;
    }

    /* Tie the usb device and the input device to this driver specific struct instance. */
    touchscreen->usbdev = dev;
    touchscreen->dev = input_dev;

    /* Generate device name information from data reported by the teensy. */
    if (dev->manufacturer){
        strlcpy(touchscreen->name, dev->manufacturer, sizeof(touchscreen->name));
    }

    if (dev->product) {
        if (dev->manufacturer){
            strlcat(touchscreen->name, " ", sizeof(touchscreen->name));
        }
        strlcat(touchscreen->name, dev->product, sizeof(touchscreen->name));
    }

    /* Probably not necessary as firmware is currently implemented. */
    if (!strlen(touchscreen->name)){
        snprintf(touchscreen->name, sizeof(touchscreen->name), 
                "USB Teensy Touchscreen %04x:%04x",
                le16_to_cpu(dev->descriptor.idVendor),
                le16_to_cpu(dev->descriptor.idProduct));
    }

    usb_make_path(dev, touchscreen->phys, sizeof(touchscreen->phys));
    strlcat(touchscreen->phys, "/input0", sizeof(touchscreen->phys));
    printk("teensy_touchscreen: phys location %s.\n", touchscreen->phys);

    /* Build input device from touchscreen information. */
    input_dev->name = touchscreen->name;
    input_dev->phys = touchscreen->phys;
    usb_to_input_id(dev, &input_dev->id);
    input_dev->dev.parent = &intf->dev;

    /* Specify input device capabilities. */
    input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) |
    BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
    input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
    input_dev->relbit[0] |= BIT_MASK(REL_WHEEL);

    /* Tie this teensy touchscreen specific struct to the input device. */
    input_set_drvdata(input_dev, touchscreen);

    input_dev->open = teensy_touchscreen_open;
    input_dev->close = teensy_touchscreen_close;
    
    usb_fill_int_urb(touchscreen->irq, dev, pipe, touchscreen->data,
            (maxp > BUF_SIZE ? BUF_SIZE : maxp), teensy_touchscreen_irq,
            touchscreen, endpoint->bInterval);

    /* Register the teensy touchscreen as an input device. */
    error = input_register_device(touchscreen->dev);
    if (error){
        goto fail3;
    }

    /* Tie this teensy touchscreen instance to the usb interface. */
    usb_set_intfdata(intf, touchscreen);

    printk("teensy_touchscreen: Teensy touchscreen mouse device now connected.\n");
    return 0;

    /* Cleanup in the event of an error. */
fail3:
    usb_free_urb(touchscreen->irq);
fail2:
    kfree(touchscreen->data);
fail1:
    input_free_device(input_dev);
    kfree(touchscreen);
    return error;
}

static void
teensy_touchscreen_disconnect(struct usb_interface * intf)
{
    struct teensy_touchscreen * touchscreen = usb_get_intfdata(intf);

    /* We are removing the interface so invalidate it first. */
    usb_set_intfdata(intf, NULL);
    if (touchscreen) {
        /* Stop running the touchscreen usb interrupt urb. */
        usb_kill_urb(touchscreen->irq);
        /* Remove this device from the list of input devices. */
        input_unregister_device(touchscreen->dev);

        /* Free unneeded memory. */
        usb_free_urb(touchscreen->irq);
        kfree(touchscreen->data);
        kfree(touchscreen);
    }

    printk("teensy_touchscreen: Teensy touchscreen mouse device disconnected.\n");
    return;
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
    if(result == 0){
        printk("teensy_touchscreen: Teensy touchscreen mouse driver loaded.\n");
    }

    return result;
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

MODULE_AUTHOR ("Clark Boylan, Nate Georgeson, Chris Noonan, Abdelhalim Ragab");
MODULE_LICENSE ("GPL v2");
