#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "esp_system.h"
#include <ArduinoJson.h>
char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
char* CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
char* WIFI_ID = "Faten";
char* WIFI_PASS = "70735987-*";
char* BACKEND_HISTORY_API = "https://profjector-back.onrender.com/api/Hystoriques";
std::string json= "";
StaticJsonDocument<384> doc;
char* professorToken = "";
char* startDate= "";
BLEAdvertising* pAdvertising = NULL;
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // NOTHING
    }

    void onDisconnect(BLEServer* pServer) {
      if (pAdvertising)pAdvertising->start();
    }
};

BLECharacteristic* pCharacteristic = NULL;
void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32");
  Serial.println("Connecting Bluetooth...");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Connected to Bluetooth"); 
  WiFi.begin(WIFI_ID, WIFI_PASS);
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  json = pCharacteristic->getValue();
  Serial.println(json.c_str());
  if(!json.empty()){
      DeserializationError error = deserializeJson(doc, json);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      int projectorId = doc["projectorId"];
      int professorId = doc["professorId"];
      const char* professorToken = doc["professorToken"];
      const char* startDate = doc["startDate"];
      if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(BACKEND_HISTORY_API); // Replace with your desired API endpoint
      http.addHeader("Content-Type", "application/json");  // Set the content type to JSON
      http.addHeader("x-access-token",professorToken);
      std::string body = "{\"proj_id\":" + std::to_string(projectorId) + ",\"user_id\":" + std::to_string(professorId) + ",\"start_date\":\"" + startDate + "\"}";
      Serial.println(body.c_str());
      int httpCode = http.POST(body.c_str());

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("HTTP response code: " + String(httpCode));
        Serial.println("Response data: " + payload);
      } else {
        Serial.println("HTTP POST request failed");
      }
    http.end();
  }
  json = "";
  pCharacteristic->setValue("");
  }
  delay(2000);
}

     