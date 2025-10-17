// Needed for atoi()
#include <stdlib.h>

// Use only core 1 for this exercise
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

//Pins
static const int led_pin = LED_BUILTIN;
int serial_in = 0;
// Settings
static const uint8_t buf_len = 20;

// task_1: Serial in blink duration
void blinkIn(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  //clear whole buffer
  memset(buf,0,buf_len);

  while(1)
  {
    if(Serial.available() > 0)
    {
      c = Serial.read();
      // Update delay variable and reset buffer if we get a newline character
      if(c == '\n')
      {
        serial_in = atoi(buf); // generic (non-Arduino) way of calculating integers from strings
        Serial.println("Updated Led delay to ");
        Serial.println(serial_in);
        memset(buf, 0, buf_len);
        idx = 0;
      }
      else
      {
        // Only append if index is not over message limit
        if(idx < buf_len -1)
        {
          buf[idx] = c;
          idx++;
        }
      }
    }
   }
}

// task_2: blink an LED
void toggleLed(void *parameter)
{
  while(1)
  {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(serial_in / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(serial_in / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);
  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Input the time for the blink in milliseconds");

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanialla FreeRTOS
              toggleLed,    // Function to be called
              "Toggle LED",// Name of the task
              1024,         // stack size (bytes in ESP32, words in FREERTOS)
              NULL,         // Parameter to pass to the function
              1,            // Task Priority (0 to configMAX_PRIORITIES -1)
              NULL,         //  Task handle
              app_cpu);     // Run on one core

   xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanialla FreeRTOS
              blinkIn,    // Function to be called
              "Serial blink in",// Name of the task
              1024,         // stack size (bytes in ESP32, words in FREERTOS)
              NULL,         // Parameter to pass to the function
              1,            // Task Priority (0 to configMAX_PRIORITIES -1)
              NULL,         //  Task handle
              app_cpu);     // Run on one core

    // delete the task containing the setup() and loop() functions. This will prevent loop() from running!
    vTaskDelete(NULL);

}

void loop() {
  // put your main code here, to run repeatedly:

}
