void Task_Print1(void *param);
void Task_Print2(void *param);

TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

int counter = 0; // Global counter variable

void setup() {
  Serial.begin(115200); // Initialize serial communication

  // Create tasks
  xTaskCreate(Task_Print1, "Task1", 1024, NULL, 1, &TaskHandle_1);
  xTaskCreate(Task_Print2, "Task2", 1024, NULL, 1, &TaskHandle_2);
}

void loop() {
  // There is no code in the loop function because FreeRTOS handles task execution
}

// Task_Print1 function definition
void Task_Print1(void *param) {
  (void)param; // Unused parameter

  TickType_t getTick; // Variable to hold tick count
  getTick = xTaskGetTickCount(); // Get current tick count

  while (1) { // Infinite loop for the task
    Serial.println("TASK1\n"); // Print message for Task 1
    counter++; // Increment counter
    Serial.print("count : "); // Print message to indicate count
    Serial.println(counter); // Print current count value
    if (counter == 15) { // If counter reaches 15
      vTaskResume(TaskHandle_2); // Resume Task 2
    }
    vTaskDelayUntil(&getTick, 1000 / portTICK_PERIOD_MS); // Delay for 1000 ms
  }
}

// Task_Print2 function definition
void Task_Print2(void *param) {
  (void)param; // Unused parameter

  while (1) { // Infinite loop for the task
    Serial.println("TASK2\n"); // Print message for Task 2
    counter++; // Increment counter
    Serial.print("count : "); // Print message to indicate count
    Serial.println(counter); // Print current count value
    if (counter == 10) { // If counter reaches 10
      vTaskSuspend(TaskHandle_2); // Suspend Task 2
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 ms
  }
}
