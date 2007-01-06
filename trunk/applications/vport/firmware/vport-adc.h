/**
 * Copyright 2006 Benedikt Sauter <sauter@ixbat.de>
 */


//struct fifo uartfifo;


void v_port_uart_command(char *buf);

/**
 * setup the interfaces
 */

void vport_uart_config(int port, int speed, int flags);

/**
 * send data from usb to rs232
 */
void vport_uart_write(int port, char * buf);


/**
 * write data from intern buffer to endpoint
 */

void vport_uart_read(int port);
