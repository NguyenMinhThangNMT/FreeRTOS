// Define two timer handles as static variables
static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;

// Callback function for timer expiration
void myTimerCallback(TimerHandle_t xTimer) {
  // Check if the timer ID is 0 (one-shot timer)
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0) {
    Serial.println("One-shot timer expired");
  }
  // Check if the timer ID is 1 (auto-reload timer)
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1) {
    Serial.println("Auto-reload timer expired");
  }
}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second

  // Create a one-shot timer
  one_shot_timer = xTimerCreate("One-shot timer", 2000 / portTICK_PERIOD_MS, pdFALSE, (void *)0, myTimerCallback);

  // Create an auto-reload timer
  auto_reload_timer = xTimerCreate("Auto-reload timer", 2000 / portTICK_PERIOD_MS, pdTRUE, (void *)1, myTimerCallback);

  // Check if timer creation was successful
  if (one_shot_timer == NULL || auto_reload_timer == NULL) {
    Serial.println("Could not create one of the timers");
  } else {
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    Serial.println("Starting timers...");
    // Start timers (max block time if command queue is full)
    xTimerStart(one_shot_timer, portMAX_DELAY);
    xTimerStart(auto_reload_timer, portMAX_DELAY);
  }
  vTaskDelete(NULL); // Delete the setup task
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}
