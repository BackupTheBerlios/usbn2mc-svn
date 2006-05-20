all:
	gcc -o usbndemo usbndemo.c -lusb

install:
	cp usbndemo /usr/bin
	chmod o+rx /usr/bin/usbndemo
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.usbndemo /etc/hotplug/usb/usbndemo
	chmod o+x /etc/hotplug/usb/usbndemo
	
clean:
	rm usbndemo
