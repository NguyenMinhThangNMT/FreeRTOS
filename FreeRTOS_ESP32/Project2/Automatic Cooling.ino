#include "DHT.h"

#define DHTPIN  15
#define BTNMODE 4
#define BTNPUMP 16
#define PUMP    2
#define DHTTYPE DHT11 

// Semaphore
SemaphoreHandle_t Mutex;

// QueueHandle
QueueHandle_t xQueueBUTTON_STATE;
QueueHandle_t xQueueTemp;

// TaskHandle
TaskHandle_t xHandle1 = NULL; // Task handle for TaskAutoMode
TaskHandle_t xHandle2 = NULL; // Task handle for TaskManualMode
TaskHandle_t xHandle3 = NULL; // Task handle for readSensor

// Variable
bool BUTTON_STATE = false; 
bool PUMP_STATE = false;
bool temp; 

DHT dht(DHTPIN, DHTTYPE);

void IRAM_ATTR choosemode() {
    BUTTON_STATE = !BUTTON_STATE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
    xQueueOverwriteFromISR(xQueueBUTTON_STATE, &BUTTON_STATE, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();  
    }
}

// Prototype
void TaskButton(void *param);
void TaskChooseMode(void *param);
void TaskAutoMode(void *param);
void TaskManualMode(void *param);
void readSensor(void *param);

void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");

    pinMode(BTNMODE, INPUT);
    pinMode(BTNPUMP, INPUT);
    pinMode(PUMP, OUTPUT);
    dht.begin(); // Initialize DHT11
    
    attachInterrupt(digitalPinToInterrupt(BTNMODE), choosemode, FALLING);

    Mutex = xSemaphoreCreateMutex(); // Create Mutex
    xQueueBUTTON_STATE = xQueueCreate(1, sizeof(bool)); // Create Queue
    xQueueTemp = xQueueCreate(1, sizeof(float)); // Create Queue for temperature
    
    // Create the TaskChooseMode task
    xTaskCreatePinnedToCore(TaskChooseMode, "TaskChooseMode", 10000, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(readSensor, "readSensor", 10000, NULL, 1, &xHandle3, 1);
    
    Serial.println("Setup completed");
}

void loop() {
}

void TaskChooseMode(void *params) {
  Serial.println("TaskChooseMode started");
  while (1) {
    if (xQueueBUTTON_STATE != NULL) {
      // Get value from queue
      if (xQueueReceive(xQueueBUTTON_STATE, &temp, portMAX_DELAY) == pdPASS) {
        Serial.println("Received BUTTON_STATE change");
        // Use to delete Task
        if (xHandle1 != NULL) {
          vTaskDelete(xHandle1);
          xHandle1 = NULL;
          Serial.println("Deleted TaskAutoMode");
        }
        if (xHandle2 != NULL) {
          vTaskDelete(xHandle2);
          xHandle2 = NULL;
          Serial.println("Deleted TaskManualMode");
        }
        // Create task AutoMode and ManualMode
        if (temp == 0) {
          xTaskCreatePinnedToCore(TaskAutoMode, "TaskAutoMode", 10000, NULL, 1, &xHandle1, 1);
          Serial.println("AUTO");
        } else {
          xTaskCreatePinnedToCore(TaskManualMode, "TaskManualMode", 10000, NULL, 1, &xHandle2, 1);
          Serial.println("MANUAL");
        }
      } 
    } 
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void readSensor(void *params) {
    Serial.println("readSensor started");
    float temperature;
 
    while (1) {
        xSemaphoreTake(Mutex, portMAX_DELAY); // Take Mutex
        temperature = dht.readTemperature(); // Read temperature from DHT11
        Serial.print("Temperature: ");
        Serial.println(temperature);
        xSemaphoreGive(Mutex); // Give Mutex
        xQueueSendToBack(xQueueTemp, &temperature, 0); // Save value to queue
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskAutoMode(void *param) {
    Serial.println("TaskAutoMode started");
    float buff;
  
    while (1) {
      xSemaphoreTake(Mutex,portMAX_DELAY);
      xQueueReceive(xQueueTemp,&buff,portMAX_DELAY);//Get value Sensor
      Serial.println("In AUTO Mode");
      if(buff>=35){
        digitalWrite(PUMP ,HIGH);
      }
      else{
        digitalWrite(PUMP ,LOW);
      }
      xSemaphoreGive(Mutex);
      vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for demonstration
    }
}

void TaskManualMode(void *param) {
    Serial.println("TaskManualMode started");
     digitalWrite(PUMP ,LOW);
    while(1){
      if (digitalRead(BTNPUMP) == HIGH) {
        PUMP_STATE = !PUMP_STATE;
        if(PUMP_STATE==1){
          digitalWrite(PUMP ,HIGH);
        }
        else{ 
          digitalWrite(PUMP ,LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for demonstration
      }
    }
}
