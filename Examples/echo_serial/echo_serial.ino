#include <stdlib.h>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 255;
static char *msg_ptr = NULL;
static volatile uint8_t  msg_flag = 0;

// Task_1: Read serial data, store in heap and Notify Task_2
void serialMsg(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  // clear whole buffer
  memset(buf,0,buf_len);

  //Loop forever
  while(1)
  {
    if(Serial.available() > 0)
    {
      c = Serial.read();

      //Store the received character to the buffer
      if(idx < buf_len -1)
      {
        buf[idx] = c;
        idx++;
      }
      //Create a message buffer for the print task
      if(c == '\n')
      {
        // The last characterr in the string is '\n' so we need to repace 
        // it with '\0' to make it null terminated
        buf[idx -1] = '\0';

        // try to allocate memory and copy over message. If message buffer is 
        // still in use, ignore the entire message.
        if(msg_flag ==0)
        {
          msg_ptr = ( char *)pvPortMalloc(idx * sizeof(char));

          //If malloc returns 0(out of memeory), throw an erroe and reset
          configASSERT(msg_ptr);

          //copy message
          memcpy(msg_ptr, buf, idx);

          //Notify other task that message is ready
          msg_flag  = 1;
        }

        //Rest receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }
  }
}

//Task_2: listean to the task1 and print on serial

void serailWrite(void *parameter)
{
  while(1)
  {
    //wait for the flag to be set and print the message
    if(msg_flag == 1)
    {
      Serial.println(msg_ptr);

      Serial.print("Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());

      //Free buffer, set pointer to Null and clear the flag
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
    }
  }
}

void setup() {
 
 Serial.begin(115200);

 // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Enter a string");

  //Start Task1
  xTaskCreatePinnedToCore(serialMsg,
                      "Read Serial",
                      1500,
                      NULL,
                      1,
                      NULL,
                      app_cpu);

  //Start Task2
  xTaskCreatePinnedToCore(serailWrite,
                      "Write to Serial",
                      1500,
                      NULL,
                      1,
                      NULL,
                      app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {

}
