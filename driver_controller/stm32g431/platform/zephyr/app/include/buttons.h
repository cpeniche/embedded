#pragma once

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