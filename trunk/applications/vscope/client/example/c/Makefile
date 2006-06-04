all:
	gcc -c ../../lib/vscopedevice.c
	gcc -o demo main.c -lusb

install:
	cp vscopec /usr/bin
	chmod o+rx /usr/bin/vscopec
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.vscope /etc/hotplug/usb/vscope
	chmod o+x /etc/hotplug/usb/vscope
	
clean:
	rm demo vscopedevice.o 
