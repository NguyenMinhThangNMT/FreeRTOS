// Define SemaphoreHandle_t and LED pin
SemaphoreHandle_t InterruptSemaphore;
#define LED 2

void setup() {
  Serial.begin(115200); // Initialize serial communication
  pinMode(2, OUTPUT); // Set pin 2 (LED) as an output

  // Create tasks for LED control
  xTaskCreate(Taskledon, "Ledon", 1024, NULL, 0, NULL);
  xTaskCreate(Taskledoff, "Ledoff", 1024, NULL, 0, NULL);

  // Create a binary semaphore
  InterruptSemaphore = xSemaphoreCreateBinary();
  if(InterruptSemaphore != NULL){
    // Attach interrupt to pin 23 with InterruptHandler as the ISR
    attachInterrupt(23, InterruptHandler, RISING);
  }
}

void loop() {
  // No code inside loop() as tasks handle the functionality
}

// Interrupt handler function
void InterruptHandler() {
  Serial.println("Semaphore is given");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(InterruptSemaphore, &xHigherPriorityTaskWoken);
}

// Task to turn LED on
void Taskledon(void *parameters) {
  (void)parameters;
  while(1) {
    if(xSemaphoreTake(InterruptSemaphore, portMAX_DELAY) == pdPASS) {
      Serial.println("Taskledon Received Semaphore ");
      digitalWrite(LED, HIGH); // Turn LED on
      delay(100);
    }
  }
}

// Task to turn LED off
void Taskledoff(void *parameters) {
  (void)parameters;
  while(1) {
    if(xSemaphoreTake(InterruptSemaphore, portMAX_DELAY) == pdPASS) {
      Serial.println("Taskledoff Received Semaphore ");
      digitalWrite(LED, LOW); // Turn LED off
      delay(100);
    }
  }
}
