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
	{USB_DEVICE(TEENSY_TOUCHSCREEN_VENDOR_ID, TEENSY_TOUCHSCREEN_PRODUCT_ID)},
	{} /* Terminate */
};

MODULE_DEVICE_TABLE(usb, teensy_ids);

/* Callback for the interrupt driven URB. When called handle any errors,
   interpret firmware report, and resubmit the URB to handle future
   communication. */
static void
teensy_touchscreen_irq(struct urb *urb)
{
	struct teensy_touchscreen *touchscreen = urb->context;
	signed char *data = touchscreen->data;
	struct input_dev *dev = touchscreen->dev;
	int status;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

	input_report_key(dev, BTN_LEFT,   data[0] & 0x01);
	input_report_key(dev, BTN_RIGHT,  data[0] & 0x02);
	input_report_key(dev, BTN_MIDDLE, data[0] & 0x04);

	input_report_rel(dev, REL_X,	 data[1]);
	input_report_rel(dev, REL_Y,	 data[2]);
	input_report_rel(dev, REL_WHEEL, data[3]);

	input_sync(dev);
resubmit:
	status = usb_submit_urb (urb, GFP_ATOMIC);
	if (status)
		err ("can't resubmit intr, %s-%s/input0, status %d",
				touchscreen->usbdev->bus->bus_name,
				touchscreen->usbdev->devpath, status);
}

/* When the input device is opened enable the interrupt driven URB. */
static int
teensy_touchscreen_open(struct input_dev *dev)
{
	struct teensy_touchscreen *touchscreen = input_get_drvdata(dev);

	touchscreen->irq->dev = touchscreen->usbdev;

	if (usb_submit_urb(touchscreen->irq, GFP_KERNEL))
		return -EIO;

	return 0;
}

/* When the input device is closed disable the interrupt driven URB. */
static void
teensy_touchscreen_close(struct input_dev *dev)
{
	struct teensy_touchscreen *touchscreen = input_get_drvdata(dev);

	usb_kill_urb(touchscreen->irq);
}

/* When the teensy touchscreen device is probed setup a URB to talk to the
   device's single interrupt endpoint. Also configure the teensy touchscreen
   as an input device and register it as one. This exposed the device as a
   mouse input device and prepares to for communication. */
static int
teensy_touchscreen_probe(struct usb_interface *intf,
		const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct teensy_touchscreen *touchscreen;
	struct input_dev *input_dev;
	int pipe;
	int maxp;
	int error = -ENOMEM;

	interface = intf->cur_altsetting;
	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
	printk("teensy_touchscreen: Max packet size is %d.\n", maxp & 0x07FF);

	touchscreen = kzalloc(sizeof(*touchscreen), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!touchscreen || !input_dev)
		goto fail1;

	touchscreen->data = kzalloc(BUF_SIZE * sizeof(*touchscreen->data), GFP_KERNEL);
	if (!touchscreen->data)
		goto fail1;

	touchscreen->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!touchscreen->irq)
		goto fail2;

	touchscreen->usbdev = dev;
	touchscreen->dev = input_dev;

	if (dev->manufacturer)
		strlcpy(touchscreen->name, dev->manufacturer,
				sizeof(touchscreen->name));

	if (dev->product) {
		if (dev->manufacturer)
			strlcat(touchscreen->name, " ",
					sizeof(touchscreen->name));
		strlcat(touchscreen->name, dev->product,
				sizeof(touchscreen->name));
	}

	/* Probably not necessary as firmware is currently implemented. */
	if (!strlen(touchscreen->name))
		snprintf(touchscreen->name, sizeof(touchscreen->name),
				"USB Teensy Touchscreen %04x:%04x",
				le16_to_cpu(dev->descriptor.idVendor),
				le16_to_cpu(dev->descriptor.idProduct));

	usb_make_path(dev, touchscreen->phys, sizeof(touchscreen->phys));
	strlcat(touchscreen->phys, "/input0", sizeof(touchscreen->phys));
	printk("teensy_touchscreen: phys location %s.\n", touchscreen->phys);

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

	input_set_drvdata(input_dev, touchscreen);

	input_dev->open = teensy_touchscreen_open;
	input_dev->close = teensy_touchscreen_close;
	
	usb_fill_int_urb(touchscreen->irq, dev, pipe, touchscreen->data,
			(maxp > BUF_SIZE ? BUF_SIZE : maxp),
			teensy_touchscreen_irq, touchscreen,
			endpoint->bInterval);

	/* Register the teensy touchscreen as an input device. */
	error = input_register_device(touchscreen->dev);
	if (error)
		goto fail3;

	usb_set_intfdata(intf, touchscreen);

	printk("teensy_touchscreen: "
			"Teensy touchscreen mouse device now connected.\n");
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

/* Cleanup the teensy device when it is removed. First invalidate it so that
   nothing attempts to use it, then kill the URB, unregister the device as an
   input device, and free any allocated memory. */
static void
teensy_touchscreen_disconnect(struct usb_interface *intf)
{
	struct teensy_touchscreen *touchscreen = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	if (touchscreen) {
		usb_kill_urb(touchscreen->irq);
		input_unregister_device(touchscreen->dev);

		usb_free_urb(touchscreen->irq);
		kfree(touchscreen->data);
		kfree(touchscreen);
	}

	printk("teensy_touchscreen:"
			"Teensy touchscreen mouse device disconnected.\n");
	return;
}

static struct usb_driver teensy_touchscreen_driver =
{
	.name = "teensy_touchscreen",			/* Unique name */
	.probe = teensy_touchscreen_probe,
	.disconnect = teensy_touchscreen_disconnect,
	.id_table = teensy_ids,
};

/* Module Initialization */
static int __init
teensy_touchscreen_init(void)
{
	int result;

	/* Register with the USB core */
	result = usb_register(&teensy_touchscreen_driver);
	if(result == 0)
		printk("teensy_touchscreen:"
				"Teensy touchscreen mouse driver loaded.\n");

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
