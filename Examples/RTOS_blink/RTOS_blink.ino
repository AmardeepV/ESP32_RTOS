// Use only core 1 for this exercise
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

//Pins
static const int led_pin = LED_BUILTIN;

// task: blink an LED

void toggleLed(void *parameter)
{
  while(1)
  {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanialla FreeRTOS
              toggleLed,    // Function to be called
              "Toggle LED",// Name of the task
              1024,         // stack size (bytes in ESP32, words in FREERTOS)
              NULL,         // Parameter to pass to the function
              1,            // Task Priority (0 to configMAX_PRIORITIES -1)
              NULL,         //  Task handle
              app_cpu);     // Run on one core

    // If this was vaniall FreeRTOS , you'd want to call vTask StartScheduler() in 
    // main after setting up your tasks.

}

void loop() {
  // put your main code here, to run repeatedly:

}
