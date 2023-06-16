#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "BluetoothSerial.h"


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Hardware declaration
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //-1 is OLED_RESET
BluetoothSerial SerialBT;

#define BLE_SERVICE_UUID "0000FFFF-0000-1000-8000-00805F9B34FB"
#define BLE_CHARACTERISTIC_UUID "0000FF01-0000-1000-8000-00805F9B34FB"

bool isPairingMode = false;
bool isBluetoothConnected = false;
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    isBluetoothConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    isBluetoothConnected = false;
  }
};

void startBluetooth() {
  BLEDevice::init("NerfGun");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID));
  pCharacteristic = pService->createCharacteristic(
    BLEUUID(BLE_CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("NerfGun");
  pService->start();
  pServer->getAdvertising()->start();
}
namespace logger{
  template<typename T>
  void println(const T& x){
    Serial.println(x);
    SerialBT.println(x);
  }

  template <typename T>
  void print(const T& x){
    Serial.print(x);
    SerialBT.print(x);
  }
};


void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void setup() {

  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //startBluetooth();
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true); 
  display.setCursor(0,10);
  
  
  if(!SerialBT.begin("NerfGun")){
   Serial.println("An error occurred initalizing Bluetooth");
    ESP.restart();
    return;
  } else{
    Serial.println("Bluetooth Initialized");
    display.println("PHONE CONNECTED!2");
  }

  SerialBT.register_callback(btCallback);
  Serial.println("==========================================================");
  Serial.println("The device started, now you can pair it with bluetooth-001");
  Serial.println("==========================================================");

  
  display.display();

}



void loop() {
  // if (SerialBT.available()) {
  //   display.clearDisplay();
  //   display.println(SerialBT.read());
  //   display.display();
  // }
}



void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected!");
  }
  else if(event == ESP_SPP_DATA_IND_EVT){
    Serial.printf("ESP_SPP_DATA_IND_EVT len=%d handle=%d\n", param->data_ind.len, param->data_ind.handle);
    int dataLen = param->data_ind.len + 1;
    char textArray[dataLen];
    strncpy(textArray, (const char*)param->data_ind.data, dataLen);
    textArray[dataLen - 1] = 0;
    String textString = textArray;
    logger::println<String>(textString);
    display.clearDisplay();
    display.setCursor(0,10);
    display.println(textString);
    display.display();
  }
}