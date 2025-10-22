#include <Arduino.h>
#include <stdlib.h>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 255;
static char *msg_ptr = NULL;
static volatile uint8_t msg_flag = 0;

// Task_1: Read serial data, store in heap and notify Task_2
void serialMsg(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  memset(buf, 0, buf_len);
  // Print the core on which this task is running
  //Serial.print("serialMsg running on core ");
  //Serial.println(xPortGetCoreID());

  printf("Task: %s running on core %d\n", pcTaskGetName(NULL), xPortGetCoreID());


  while (1)
  {
    if (Serial.available() > 0)
    {
      c = Serial.read();

      if (idx < buf_len - 1)
      {
        buf[idx++] = c;
      }

      if (c == '\n')
      {
        // replace '\n' with '\0'
        if (idx > 0) buf[idx - 1] = '\0';

        if (msg_flag == 0)
        {
          // allocate idx+1 to include null terminator
          msg_ptr = (char *)pvPortMalloc((size_t)(idx + 1) * sizeof(char));
          if (msg_ptr == NULL)
          {
            Serial.println("Malloc failed!");
          }
          else
          {
            // copy including terminating null
            memcpy(msg_ptr, buf, (size_t)(idx + 1));
            msg_flag = 1;
          }
        }

        // reset receive buffer and index
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }

    // yield to other tasks / avoid watchdog
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Task_2: listen to Task_1 and print on serial
void serialWrite(void *parameter)
{
  // Print the core on which this task is running
  //Serial.print("serialWrite running on core ");
  //Serial.println(xPortGetCoreID());

  printf("Task: %s running on core %d\n", pcTaskGetName(NULL), xPortGetCoreID());


  while (1)
  {
    if (msg_flag == 1 && msg_ptr != NULL)
    {
      Serial.println(msg_ptr);
      Serial.print("Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());

      // free and clear
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
    }

    // yield to other tasks / avoid watchdog
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Enter a string");

  // Increase stack size to 4096 (safer with local buffers)
  const uint32_t stackSize = 4096;

  // Start Task1 pinned to app_cpu
  xTaskCreatePinnedToCore(serialMsg,
                          "Read Serial",
                          stackSize,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Start Task2 pinned to the other core
  xTaskCreatePinnedToCore(serialWrite,
                          "Write Serial",
                          stackSize,
                          NULL,
                          1,
                          NULL,
                          (BaseType_t)(1 - app_cpu)); // run on different core than task_1

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {}
