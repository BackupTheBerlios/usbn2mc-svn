
void UARTInit(void);
void UARTInitDynamic(char baud, char flags);
void UARTPutChar(unsigned char sign);
unsigned char UARTGetChar(void);
void UARTWrite(char* msg);

unsigned char AsciiToHex(unsigned char high,unsigned char low);
void SendHex(unsigned char hex);

