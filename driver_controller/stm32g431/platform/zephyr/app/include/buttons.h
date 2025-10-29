#pragma once

extern "C" struct k_queue *getQueue(void);

class Buttons{

public:
    Buttons(){}
    ~Buttons(){}
    int8_t Read(uint8_t *, size_t);
    void Task(void);           
    void SetDriver(LIN *driver){Driver=driver;}
    uint8_t *getData(){return Driver->getRxBuffer();}
    bool getDataReady();
      
private:  
  LIN *Driver;
  
};