### Automatic Makefile generation by 'genmake' script        ####
### Copyright, Jerry Cooperstein, coop@linuxfoundation.org 2/2003 - 11/2009 ####
### License: GPLv2 ###

obj-m +=  teensy_touchscreen.o

export KROOT=/lib/modules/$(shell uname -r)/build

allofit:   modules
modules:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules
modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules_install
kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(PWD) clean

clean: kernel_clean
	rm -rf   Module.symvers modules.order
