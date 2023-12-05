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
std::string BACKEND_HISTORY_API = "https://profjector-back.onrender.com/api/Hystoriques/";
std::string json= "";
StaticJsonDocument<420> doc;
char* professorToken = "";
char* startDate= "";
char url[300];
std::string response;
BLECharacteristic* pCharacteristic = NULL;
BLEAdvertising* pAdvertising = NULL;
void readInChunks(const std::string& inputString, int chunkSize) {
    int totalLength = inputString.length();
    int offset = 0;

    while (offset < totalLength) {
        // Get the next chunk (up to 20 characters)
        std::string chunk = inputString.substr(offset, chunkSize);

        // Process the chunk (you can replace this with your own logic)
        pCharacteristic->setValue(chunk);
        pCharacteristic->notify();
        // Move to the next chunk
        offset += chunkSize;
    }
}
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // NOTHING
    }

    void onDisconnect(BLEServer* pServer) {
      if (pAdvertising)pAdvertising->start();
    }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("PROFJECTOR_ESP32");
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

  if(!json.empty()){
      Serial.println(json.c_str());
      DeserializationError error = deserializeJson(doc, json);
      if (error) 
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      bool rent = doc["rent"];
      if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.setTimeout(20000); 
      if(!rent)
      {

        int projectorId = doc["projectorId"];
        int professorId = doc["professorId"];
        std::string startDate = doc["startDate"];
        std::string professorToken = doc["professorToken"];
        Serial.println(professorToken.c_str());
        http.begin(BACKEND_HISTORY_API.c_str()); // Replace with your desired API endpoint
        http.addHeader("x-access-token",professorToken.c_str());    
        http.addHeader("Content-Type", "application/json");  // Set the content type to JSON   
        std::string body = "{\"proj_id\":" + std::to_string(projectorId) + ",\"user_id\":" + std::to_string(professorId) + ",\"start_date\":\"" + startDate + "\"}";
        Serial.println(body.c_str());
        bool apiConsumed = false;
        while(!apiConsumed)
        {
          int httpCode = http.POST(body.c_str());
          if (httpCode > 0) 
          {
            std::string payload = std::string(http.getString().c_str());
            Serial.println("HTTP response code: " + String(httpCode));
            response= "{\"response\":"+payload+",\"status\":"+std::to_string(httpCode)+"}#";
            if(httpCode == 200) 
              apiConsumed = true;
          } 
          else 
          {
            response= "{\"response\":{\"message\":\"Some Error happened\"},\"status\":"+std::to_string(httpCode)+"}#";
            Serial.println(http.errorToString(httpCode).c_str());
          }
        }
      }
    else
    {
        int projectorId = doc["projectorId"];
        std::string endDate = doc["endDate"];
        std::string professorToken = doc["professorToken"];
        http.addHeader("Content-Type", "application/json");  // Set the content type to JSON
        Serial.println(professorToken.c_str());        
    
        Serial.println(projectorId);
        std::string idHyst = std::to_string(projectorId);
        std::string api_endpoint = BACKEND_HISTORY_API + idHyst;
        http.begin(api_endpoint.c_str());
        http.addHeader("Content-Type", "application/json");  // Set the content type to JSON  
        http.addHeader("x-access-token",professorToken.c_str());   
        std::string body = "{\"end_date\":\""+endDate+"\"}";
        bool apiConsumed = false;
        while(!apiConsumed)
        {
          int httpCode = http.PUT(body.c_str());
          if (httpCode > 0) 
          {
            std::string payload = std::string(http.getString().c_str());
            response= "{\"response\":"+payload+",\"status\":"+std::to_string(httpCode)+"}#";
            if(httpCode == 200) 
              apiConsumed = true;
          } 
          else 
          {
            response= "{\"response\":{\"message\":\"Some Error happened\"},\"status\":"+std::to_string(httpCode)+"}#";
            Serial.println(http.errorToString(httpCode).c_str());
          }
        }
    }
    http.end();
  }
  readInChunks(response,20);
  json = "";
  pCharacteristic->setValue("");
  }
  delay(2000);
}
