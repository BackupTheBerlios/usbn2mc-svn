#include "fifo.h"

#define MODE_NONE         0x00
#define MODE_COUNTER      0x01
#define MODE_LOGIC        0x02
#define MODE_1CHANNELAD   0x03
#define MODE_LOGICINTERN  0x04


#define TRIGGER_OFF	  0x01
#define TRIGGER_EDGE	  0x02
#define TRIGGER_PATTERN	  0x03

#define STATE_DONOTHING   0x01
#define STATE_RUNNING     0x02
#define STATE_TRIGGER     0x03

#define SAMPLERATE_5US	  0x01
#define SAMPLERATE_10US	  0x02
#define SAMPLERATE_50US	  0x03
#define SAMPLERATE_100US  0x04
#define SAMPLERATE_1MS	  0x05
#define SAMPLERATE_10MS	  0x06
#define SAMPLERATE_100MS  0x07

#define CMD_SETMODE       0x01
#define CMD_STARTSCOPE    0x02
#define CMD_STOPSCOPE     0x03
#define CMD_GETSCOPEMODE  0x04
#define CMD_GETSCOPESTATE 0x05
#define CMD_GETFIFOLOAD   0x06
#define CMD_SETSAMPLERATE 0x07
#define CMD_GETDATA	  0x08

#define CMD_SETEDGETRIG	  0x09
#define CMD_SETPATTRIG	  0x0A
#define CMD_DEACTIVTRIG	  0x0B
#define CMD_GETSNAPSHOT	  0x0C

int togl;

int8_t fifobuffer[1000];

typedef struct {
  uint8_t state;
  uint8_t mode;
  uint8_t samplerate;
  fifo_t fifo;
  uint8_t trigger;
  uint8_t trigger_value;
  uint8_t trigger_ignore;
  uint8_t trigger_channel;
  uint8_t trigger_last;
  uint8_t tx;
} vscope_t;

volatile vscope_t vscope;

void VScopeSendScopeData();
void VScopeCommand(char *buf);
void VScopePingPongTX1();
void VScopePingPongTX2();
