#pragma once

#define  maskLOCK           0x01
#define  maskUNLOCK         0x02
#define  maskRIGHWINDOWUP   0x04
#define  maskRIGHWINDOWDWON 0x08
#define  maskLEFTWINDOWUP   0x10
#define  maskLEFTWINDOWDWON 0x20s
#define  maskMIRRORUP       0x40
#define  maskMIRRORDOWN     0x80

struct k_queue *getQueue(void);

class Buttons{

public:
    Buttons(){}
    ~Buttons(){}
    int8_t Read(uint8_t *, size_t);           
    void SetDriver(LIN *driver){Driver=driver;}
      
private:  
  LIN *Driver;  
};