
#include "BLEAdvertisedDevice.h"
#include "BLEDevice.h"
#include "BLEScan.h"
#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "FreeRTOS.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

#define GPIO_DEEP_SLEEP_DURATION     2  // sleep x seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

BLEAdvertising *pAdvertising;   // BLE Advertisement type
struct timeval now;

#define BEACON_UUID "87b99b2c-90fd-11e9-bc42-526af7764f64" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

const int PIN = 2;
const int CUTOFF = -43+-2;


void setup() {
  Serial.begin(115200);
  pinMode(PIN, OUTPUT);
  BLEDevice::init("");
  gettimeofday(&now, NULL);
  Serial.printf("start ESP32 %d\n", bootcount++);
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
  last = now.tv_sec;

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    taskOne,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */                  
  delay(10); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    taskTwo,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */
    delay(10); 
}
void loop() {

}

void taskOne( void * parameter )
{
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(1);
  int best = CUTOFF;
  Serial.print("Setup: Executing on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    int rssi = device.getRSSI();
    if (rssi > best) {
      best = rssi; 
    }    
}
    Serial.println(best);
    digitalWrite(PIN, best > CUTOFF ? HIGH : LOW);
    delay(100);
    Serial.println("Ending task 1");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
    } 
}
 
void taskTwo( void * parameter)
{

 
 for(;;){
  // Create the BLE Device
  BLEDevice::init("ESP32 as iBeacon");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer(); 
  pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();
  setBeacon();
  pAdvertising->start();
  Serial.print("Setup: Executing on core ");
  Serial.println(xPortGetCoreID());
  Serial.println("Advertizing started...");
  delay(100);
  pAdvertising->stop();
  }
}

void setBeacon() {
  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  std::string strServiceData = "";
  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

 
