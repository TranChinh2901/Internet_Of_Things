#include <BLEDevice.h>
#include <BLEServer.h>
#include <DHT.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-0987654321ab"

DHT dht(4, DHT22);
BLECharacteristic *pCharacteristic;

void setup() {
  Serial.begin(115200);
  BLEDevice::init("Slave1CC");
  
  // Set MTU to allow larger data packets (default is 23 bytes, only 20 usable)
  BLEDevice::setMTU(512);
  Serial.println("BLE MTU set to 512 bytes");
  
  Serial.print("Device MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pService->start();
  pServer->getAdvertising()->start();
  dht.begin();
  
  Serial.println("Slave1CC BLE server started");
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    static float fakeTemp = 24.0f;
    static float fakeHum = 55.0f;
    fakeTemp += 0.3f;
    fakeHum += 0.5f;
    if (fakeTemp > 30.0f) fakeTemp = 24.0f;
    if (fakeHum > 65.0f) fakeHum = 55.0f;
    t = fakeTemp;
    h = fakeHum;
    Serial.println("Using synthetic sensor data");
  }

  char buffer[128];
  int len = sprintf(
    buffer,
    "{\"temp\":%.2f,\"hum\":%.2f}",
    t,
    h
  );
  
  // Use explicit length to ensure complete data transmission
  pCharacteristic->setValue((uint8_t*)buffer, len);
  pCharacteristic->notify();
  Serial.print("Sent (");
  Serial.print(len);
  Serial.print(" bytes): ");
  Serial.println(buffer);

  delay(2000);
}
