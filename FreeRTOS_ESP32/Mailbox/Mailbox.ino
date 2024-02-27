// Define a mailbox queue and task handles
QueueHandle_t xMailbox;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

void setup() {
  Serial.begin(115200); // Initialize serial communication
  xMailbox = xQueueCreate(1, sizeof(int32_t)); // Create a mailbox queue with size 1

  // Create tasks for updating and reading the mailbox
  xTaskCreate(vUpdateMailbox, "Sender", 1000, NULL, 1, &TaskHandle_1);
  xTaskCreate(vReadMailbox, "Receiver", 1000, NULL, 1, &TaskHandle_2);
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}

// Task function to update the mailbox
void vUpdateMailbox(void *Parameters) {
  int32_t ulNewValue = 1;
  while(1) {
    xQueueOverwrite(xMailbox, &ulNewValue); // Overwrite the mailbox with the new value
    Serial.println("Ghi gia tri vao MailBox:"); // Print message indicating writing to mailbox
    Serial.println("\n");
    ulNewValue++; // Increment the value
    if(ulNewValue > 100) ulNewValue = 0; // Reset value if it exceeds 100
    vTaskDelay(1000); // Delay for 1000 ms
  }
}

// Task function to read the mailbox
void vReadMailbox(void *Parameters) {
  int value_received;
  while(1) {
    xQueuePeek(xMailbox, &value_received, portMAX_DELAY); // Peek the value from the mailbox
    Serial.print("Gia tri doc duoc tu mailbox ="); // Print message indicating reading from mailbox
    Serial.print(value_received); // Print the value read from the mailbox
    Serial.println("\n");
    vTaskDelay(500); // Delay for 500 ms
  }
}
