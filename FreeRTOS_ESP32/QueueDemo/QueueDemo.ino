// Function prototypes for tasks
void task1(void *param);
void task2(void *param);

// Task handles and queue handle
TaskHandle_t TaskHandle_1 = NULL;
TaskHandle_t TaskHandle_2 = NULL;
QueueHandle_t Queue;

// Task 1 function
void task1(void *p) {
  char myTxBuff[30];
  
  // Create a queue with a capacity of 5 elements, each of size sizeof(myTxBuff)
  Queue = xQueueCreate(5, sizeof(myTxBuff));

  // Send three messages to the queue
  sprintf(myTxBuff, "Hello");
  xQueueSend(Queue, (void *)myTxBuff, (TickType_t)0);
  sprintf(myTxBuff, "FreeRTOS");
  xQueueSend(Queue, (void *)myTxBuff, (TickType_t)0);
  sprintf(myTxBuff, "ESP32");
  xQueueSend(Queue, (void *)myTxBuff, (TickType_t)0);

  // Print queue information
  Serial.print("Data waiting to be read: ");
  Serial.print(uxQueueMessagesWaiting(Queue));
  Serial.print("\n");
  Serial.print("Available space: ");
  Serial.print(uxQueueSpacesAvailable(Queue));
  Serial.print("\n");

  while (1) {
    // Task 1 does not perform any further actions after initializing the queue and sending messages
    // It stays in an infinite loop
  }
}

// Task 2 function
void task2(void *p) {
  char myRxBuff[30];

  while (true) {
    if (Queue != 0) { // Check if the queue has been initialized
      if (xQueueReceive(Queue, (void *)myRxBuff, (TickType_t)5)) { // Try to receive a message from the queue with a timeout of 5 ticks
        Serial.print("Data received: ");
        Serial.print(myRxBuff);
        Serial.print("\n");
      }
    }
  }
}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  
  // Create tasks, pinned to core 1
  xTaskCreatePinnedToCore(task1, "task1", 1024, NULL, 1, &TaskHandle_1, 1);
  xTaskCreatePinnedToCore(task2, "task2", 1024, NULL, 1, &TaskHandle_2, 1);
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}
