#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>

#define BTN_MODE   12
#define BTN_PUMP   14
#define PUMP       2
#define BMP_SCK    18
#define BMP_MISO   19
#define BMP_MOSI   23
#define BMP_CS     5

SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xMutex;

QueueHandle_t xQueueBUTTON_STATE;
QueueHandle_t xQueueTemp;

TaskHandle_t xHandle1;
TaskHandle_t xHandle2;
TaskHandle_t xHandle3;


void taskButton(void *params);
void taskChooseMode(void *params);
void taskAutoMode(void *params);
void taskManualMode(void *params);

static int BUTTON_STATE;



//Setup for BMP280,LCD-I2C
Adafruit_BMP280 bmp(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI); // Khai báo đối tượng bmp với giao tiếp SPI
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


void setup() {
  Serial.begin(115200);

  pinMode(BTN_MODE, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(BTN_PUMP,INPUT);
  digitalWrite(PUMP, LOW);

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
  //Create Queue
  xQueueTemp=xQueueCreate(1, sizeof(float));
  xQueueBUTTON_STATE = xQueueCreate(1, sizeof(int));
  //Create Binary Semaphore and Mutex
  xBinarySemaphore = xSemaphoreCreateBinary();
  xMutex = xSemaphoreCreateMutex();  

  //Create task
  xTaskCreatePinnedToCore(readSensor, "readSensor", 10000, NULL, 3, &xHandle3, 1);
  xTaskCreatePinnedToCore(taskButton, "taskButton", 12288, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskChooseMode, "taskChooseMode", 12288, NULL, 3, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}

//BMP280 and LCD
void readSensor(void *params){
 float temperature;
 float pressure;

  while(1){
    
    Serial.print(F("Temperature = "));
    temperature = bmp.readTemperature();
    Serial.print(temperature);
    Serial.println(" *C");

    xSemaphoreGive(xMutex);
    xQueueSendToBack(xQueueTemp,&temperature,0);
    
    lcd.setCursor(0, 1); 
    lcd.print("Nhiet do: ");
    lcd.print(temperature);
    lcd.print(" *C");

    vTaskDelay(1000);
  }
}

void taskButton(void *params) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  xTaskCreatePinnedToCore(taskAutoMode, "taskAutoMode", 10000, NULL, 5, &xHandle1, 1);
  while(1) {  
    if (digitalRead(BTN_MODE) == HIGH) {
      BUTTON_STATE = !BUTTON_STATE;//0->1,1->0;
      xQueueOverwrite(xQueueBUTTON_STATE, &BUTTON_STATE);
    }
  }
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100);
}

void taskChooseMode(void *params) {
  static int temp;
  while (1) {  
    xQueueReceive(xQueueBUTTON_STATE, &temp, portMAX_DELAY);
    Serial.print(temp);
    if (temp == 0) {
      Serial.println("AUTO");
      vTaskDelete(xHandle2);
      xTaskCreatePinnedToCore(taskAutoMode, "taskAutoMode", 10000, NULL, 3, &xHandle1, 1);
    }     
    else { 
      Serial.println("MANUAL");
      vTaskDelete(xHandle1);
      xTaskCreatePinnedToCore(taskManualMode, "taskManualMode", 10000, NULL, 3, &xHandle2, 1);
      Serial.println("MANUAL");
    }
  }
  
}

void taskAutoMode(void *params) {
  
  xSemaphoreTake(xMutex,portMAX_DELAY);
  float buff;
  lcd.clear();

  while(1){  
      xQueueReceive(xQueueTemp,&buff,portMAX_DELAY);
      Serial.println("In AUTO Mode");
      if(buff>=32){
        xTaskCreatePinnedToCore(taskPumpOn,"TurnOn",4896,NULL,4,NULL,0);
      }
      else{
        xTaskCreatePinnedToCore(taskPumpOff,"TurnOff",4896,NULL,4,NULL,0);
      }
      lcd.setCursor(2, 0); 
      lcd.print("MODE : AUTO ");
      lcd.setCursor(0, 1); 
      lcd.print("Nhiet do: ");
      lcd.print(buff);
      lcd.print(" *C");


      xSemaphoreGive(xMutex);
      vTaskDelay(400);    
  }
}

void taskManualMode(void *params){
  int PUMP_STATE;
  lcd.clear();
  delay(1000);
  while(1) {
    if (digitalRead(BTN_PUMP) == HIGH) {
      PUMP_STATE = !PUMP_STATE;    
    }
    if(PUMP_STATE==0){
      xTaskCreatePinnedToCore(taskPumpOff,"TurnOff",4896,NULL,4,NULL,0);
      lcd.setCursor(2, 0);
      lcd.print("MODE : MANUAL ");
      lcd.setCursor(2, 1); 
      lcd.print("PUMP : ON ");
    }
    else{
      xTaskCreatePinnedToCore(taskPumpOn,"TurnOn",4896,NULL,4,NULL,0);
      lcd.setCursor(2, 0); // Di chuyển con trỏ đến hàng 1, cột 1
      lcd.print("MODE : MANUAL ");
      lcd.setCursor(2, 1); // Di chuyển con trỏ đến hàng 1, cột 1
      lcd.print("PUMP : OFF ");
    }
    vTaskDelay(400); 
  }
}

void taskPumpOn(void *params){
  Serial.print("Pump_On");
  digitalWrite(PUMP, 0);
}

void taskPumpOff(void *params){
  Serial.print("Pump_Off");
  digitalWrite(PUMP, 1);
}
