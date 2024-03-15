#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>

//Define GPIO
#define BTN_MODE   12
#define BTN_FAN    14
#define FAN        2
#define BMP_SCK    18
#define BMP_MISO   19
#define BMP_MOSI   23
#define BMP_CS     5


//SemaphoreHandle
SemaphoreHandle_t BinarySemaphore;
SemaphoreHandle_t Mutex;

//QueueHandle
QueueHandle_t xQueueBUTTON_STATE;
QueueHandle_t xQueueTemp;

//TaskHandle
TaskHandle_t xHandle1 = NULL; // Task handle for TaskAutoMode
TaskHandle_t xHandle2 = NULL; // Task handle for TaskManualMode
TaskHandle_t xHandle3;

//Variable
int BUTTON_STATE;
int FAN_STATE;
int temp;

//Prototype
void TaskButton(void *param);
void TaskChooseMode(void *param);
void TaskAutoMode(void *param);
void TaskManualMode(void *param);

//Setup for BMP280,LCD-I2C
Adafruit_BMP280 bmp(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI); // Khai báo đối tượng bmp với giao tiếp SPI
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


void setup() {
  Serial.begin(115200);
  pinMode(BTN_MODE, INPUT);
  pinMode(BTN_FAN,INPUT);
  pinMode(FAN ,OUTPUT);
  digitalWrite(FAN ,LOW);

  
  BinarySemaphore = xSemaphoreCreateBinary();//Create Semaphore
  Mutex = xSemaphoreCreateMutex();//Create Mutex
  
  //BMP280 and lcd
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID); // Khởi động BMP280
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Chế độ hoạt động. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Lấy mẫu nhiệt độ */
                  Adafruit_BMP280::SAMPLING_X16,    /* Lấy mẫu áp suất */
                  Adafruit_BMP280::FILTER_X16,      /* Bộ lọc. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Thời gian chờ. */


  lcd.init();
  lcd.backlight();

  //Create Task
  xTaskCreatePinnedToCore(TaskButton, "TaskButton", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(TaskChooseMode, "TaskChooseMode", 10000, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(readSensor, "readSensor", 10000, NULL, 3, &xHandle3, 1);
  

  //Give Semaphore
  xSemaphoreGive(BinarySemaphore);
}

void loop() {
}

void readSensor(void *params){
 float temperature;
 float pressure;
 xSemaphoreTake(Mutex,portMAX_DELAY);//Take Mutex
  while(1){
    //Serial print Temperature
    Serial.print(F("Temperature = "));
    temperature = bmp.readTemperature();
    Serial.print(temperature);
    Serial.println(" *C");

    xSemaphoreGive(Mutex);//Give Mutex
    xQueueSendToBack(xQueueTemp,&temperature,0);//save value to queue
    
    //display on lcd
    lcd.setCursor(0, 1); 
    lcd.print("Nhiet do: ");
    lcd.print(temperature);
    lcd.print(" *C");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void TaskButton(void *param) {
  xSemaphoreTake(BinarySemaphore, portMAX_DELAY);//Take Semaphore
  while (1) {
    if (digitalRead(BTN_MODE) == HIGH) {//Read value Button
      BUTTON_STATE = !BUTTON_STATE;
      xQueueOverwrite(xQueueBUTTON_STATE, &BUTTON_STATE);//save value Button to Queue
    }
    Serial.println(BUTTON_STATE);
    xSemaphoreGive(BinarySemaphore);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}



void TaskChooseMode(void *params) {
  while (1) {
    if (xQueueBUTTON_STATE != NULL) {
      //Get value for queue 
      if (xQueueReceive(xQueueBUTTON_STATE, &temp, portMAX_DELAY) == pdPASS) {
        //use to delete Task
        if (xHandle1 != NULL) {
          vTaskDelete(xHandle1);
          xHandle1 = NULL;
        }
        if (xHandle2 != NULL) {
          vTaskDelete(xHandle2);
          xHandle2 = NULL;
        }
        //Create task AutoMode and ManualMode
        if (temp == 0) {
          xTaskCreatePinnedToCore(TaskAutoMode, "TaskAutoMode", 10000, NULL, 1, &xHandle1, 1);
          Serial.println("AUTO");
        } else {
          xTaskCreatePinnedToCore(TaskManualMode, "TaskManualMode", 10000, NULL, 1, &xHandle2, 1);
          Serial.println("MANUAL");
        }
      } 
    } 
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskAutoMode(void *params) {
  xSemaphoreTake(Mutex,portMAX_DELAY);
  float buff;
  lcd.clear();
  while(1){
      xQueueReceive(xQueueTemp,&buff,portMAX_DELAY);//Get value Sensor
      Serial.println("In AUTO Mode");
      if(buff>=32){
        digitalWrite(FAN ,HIGH);
      }
      else{
        digitalWrite(FAN ,LOW);
      }
      //Display on lcd
      lcd.setCursor(2, 0); 
      lcd.print("MODE : AUTO ");
      lcd.setCursor(0, 1); 
      lcd.print("Nhiet do: ");
      lcd.print(buff);
      lcd.print(" *C");

      xSemaphoreGive(Mutex);
      vTaskDelay(4000);    
  }
}

void TaskManualMode(void *param){
  digitalWrite(FAN ,LOW);
  while(1){
    if (digitalRead(BTN_FAN) == HIGH) {
      FAN_STATE = !FAN_STATE;
      if(FAN_STATE==1){
        digitalWrite(FAN ,HIGH);
        lcd.setCursor(2, 0);
        lcd.print("MODE : MANUAL ");
        lcd.setCursor(2, 1); 
        lcd.print("FAN  : ON ");
      }
      else{ 
      digitalWrite(FAN ,LOW);
      lcd.setCursor(2, 0); 
      lcd.print("MODE : MANUAL ");
      lcd.setCursor(2, 1); 
      lcd.print("FAN  : OFF ");
      }
    } 
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
