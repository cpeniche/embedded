
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#ifdef __cplusplus
extern "C" {
#endif

void vtestTask(void *pvParameters);




void app_main() 
{

  xTaskCreate(vtestTask,"test",4096, NULL,tskIDLE_PRIORITY,NULL);

}


void vtestTask(void *pvParameters)
{
  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000)); 

  }

}

#ifdef __cplusplus
}
#endif
