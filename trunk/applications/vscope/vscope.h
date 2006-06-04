
int togl;


typedef struct vscope VScope;
struct vscope {
  uint8_t state;
  uint8_t mode;
};


void VscopeSendData(char *buf);
void VScopeCommand(char *buf);

