// Define the number of tasks
const int nTasks = 4;

// Create a counting semaphore to act as a barrier
SemaphoreHandle_t barrierSemaphore = xSemaphoreCreateCounting(nTasks, 0);

// Task function
void genericTask(void *parameter) {
  int taskNumber = (int)parameter;
  String taskMessage = "Task number: ";
  taskMessage += taskNumber;
  Serial.println(taskMessage);

  // Wait at the barrier
  xSemaphoreGive(barrierSemaphore);

  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Starting to launch tasks...");

  for (int i = 0; i < nTasks; i++) {
    xTaskCreate(genericTask, "genericTask", 10000, (void*)i, 1, NULL);
  }

  Serial.println("Tasks launched and semaphore passed...");
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}
