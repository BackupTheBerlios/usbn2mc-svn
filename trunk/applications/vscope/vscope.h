#define MODE_NONE         0x00
#define MODE_COUNTER      0x01
#define MODE_LOGIC        0x02
#define MODE_1CHANNELAD   0x03


#define STATE_DONOTHING   0x01
#define STATE_RUNNING     0x02
#define STATE_TRIGGER     0x03


#define CMD_SETMODE       0x01
#define CMD_STARTSCOPE    0x02
#define CMD_STOPSCOPE     0x03
#define CMD_GETSCOPEMODE  0x04
#define CMD_GETSCOPESTATE 0x05
#define CMD_GETFIFOLOAD   0x06

int togl;

typedef struct {
  uint8_t state;
  uint8_t mode;
} vscope_t;


void VScopeSendScopeData();
void VScopeCommand(char *buf);
void VScopePingPongTX1();
void VScopePingPongTX2();

