all:
	gcc -o vlabc vlabc.c -lusb

install:
	cp vlabc /usr/bin
	chmod o+rx /usr/bin/vlabc
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.vlab /etc/hotplug/usb/vlab
	chmod o+x /etc/hotplug/usb/vlab
	
clean:
	rm vlab
