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
TaskHandle_t xHandleAutoMode = NULL; // Task handle for AutoMode
TaskHandle_t xHandleManualMode = NULL; // Task handle for ManualMode
TaskHandle_t xHandleReadSensor = NULL; // Task handle for ReadSensor

// Variable
bool buttonState = false; 
bool pumpState = false;

DHT dht(DHTPIN, DHTTYPE);

void IRAM_ATTR ISR_ChooseMode() {
    buttonState = !buttonState;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
    xQueueOverwriteFromISR(xQueueBUTTON_STATE, &buttonState, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();  
    }
}

// Function prototypes
void TaskChooseMode(void *param);
void TaskAutoMode(void *param);
void TaskManualMode(void *param);
void TaskReadSensor(void *param);

void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");

    pinMode(BTNMODE, INPUT);
    pinMode(BTNPUMP, INPUT);
    pinMode(PUMP, OUTPUT);
    dht.begin(); // Initialize DHT11
    
    attachInterrupt(digitalPinToInterrupt(BTNMODE), ISR_ChooseMode, FALLING);

    Mutex = xSemaphoreCreateMutex(); // Create Mutex
    xQueueBUTTON_STATE = xQueueCreate(1, sizeof(bool)); // Create Queue for button state
    xQueueTemp = xQueueCreate(1, sizeof(float)); // Create Queue for temperature
    
    // Create tasks
    xTaskCreatePinnedToCore(TaskChooseMode, "TaskChooseMode", 10000, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadSensor, "TaskReadSensor", 10000, NULL, 1, &xHandleReadSensor, 1);
    
    Serial.println("Setup completed");
}

void loop() {
    // Empty loop since FreeRTOS tasks are used
}

void TaskChooseMode(void *params) {
    Serial.println("TaskChooseMode started");
    bool receivedButtonState;
    while (1) {
        if (xQueueBUTTON_STATE != NULL) {
            if (xQueueReceive(xQueueBUTTON_STATE, &receivedButtonState, portMAX_DELAY) == pdPASS) {
                Serial.println("Received BUTTON_STATE change");
                
                if (xHandleAutoMode != NULL) {
                    vTaskDelete(xHandleAutoMode);
                    xHandleAutoMode = NULL;
                    Serial.println("Deleted TaskAutoMode");
                }
                if (xHandleManualMode != NULL) {
                    vTaskDelete(xHandleManualMode);
                    xHandleManualMode = NULL;
                    Serial.println("Deleted TaskManualMode");
                }

                if (receivedButtonState == 0) {
                    xTaskCreatePinnedToCore(TaskAutoMode, "TaskAutoMode", 10000, NULL, 1, &xHandleAutoMode, 1);
                    Serial.println("AUTO Mode");
                } else {
                    xTaskCreatePinnedToCore(TaskManualMode, "TaskManualMode", 10000, NULL, 1, &xHandleManualMode, 1);
                    Serial.println("MANUAL Mode");
                }
            } 
        } 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskReadSensor(void *params) {
    Serial.println("TaskReadSensor started");
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
    float temperature;
    while (1) {
        if (xQueueTemp != NULL) {
            if (xQueueReceive(xQueueTemp, &temperature, portMAX_DELAY) == pdPASS) {
                Serial.println("In AUTO Mode");
                if (temperature >= 35) {
                    digitalWrite(PUMP, HIGH);
                } else {
                    digitalWrite(PUMP, LOW);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for demonstration
    }
}

void TaskManualMode(void *param) {
    Serial.println("TaskManualMode started");
    digitalWrite(PUMP, LOW);
    while (1) {
        if (digitalRead(BTNPUMP) == HIGH) {
            pumpState = !pumpState;
            digitalWrite(PUMP, pumpState ? HIGH : LOW);
            vTaskDelay(pdMS_TO_TICKS(1000)); // Debounce delay
        }
    }
}
