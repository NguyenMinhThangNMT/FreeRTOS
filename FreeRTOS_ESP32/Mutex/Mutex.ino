SemaphoreHandle_t xMutex; // Define a mutex semaphore

void setup() {
  Serial.begin(115200); // Initialize serial communication
  xMutex = xSemaphoreCreateMutex(); // Create a mutex semaphore

  // Create tasks
  xTaskCreate(lowPriorityTask, "lowPriorityTask", 1000, "Low priority task", 1, NULL);
  delay(500); // Delay to ensure lowPriorityTask starts first
  xTaskCreate(highPriorityTask, "highPriorityTask", 1000, "High priority task", 4, NULL);
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}

// Low priority task function
void lowPriorityTask(void *parameter) {
  Serial.println((char *)parameter); // Print task name
  while(1) {
    Serial.println("lowPriorityTask gains key"); // Print message indicating task acquiring mutex
    xSemaphoreTake(xMutex, portMAX_DELAY); // Take mutex semaphore
    delay(2000); // Simulate task execution
    Serial.println("lowPriorityTask releases key"); // Print message indicating task releasing mutex
    xSemaphoreGive(xMutex); // Give back mutex semaphore
  }
  vTaskDelete(NULL); // Delete the task (shouldn't reach here)
}

// High priority task function
void highPriorityTask(void *parameter) {
  Serial.println((char *)parameter); // Print task name
  while(1) {
    Serial.println("highPriorityTask gains key"); // Print message indicating task acquiring mutex
    xSemaphoreTake(xMutex, portMAX_DELAY); // Take mutex semaphore
    Serial.println("highPriorityTask is running"); // Print message indicating task execution
    Serial.println("highPriorityTask releases key"); // Print message indicating task releasing mutex
    xSemaphoreGive(xMutex); // Give back mutex semaphore
    delay(1000); // Simulate task execution
  }
  vTaskDelete(NULL); // Delete the task (shouldn't reach here)
}
