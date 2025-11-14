#include <ArduinoJson.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <cstdio>

// WiFi credentials - UPDATE THESE WITH YOUR WIFI DETAILS
#define WIFI_SSID "chuanef"
#define WIFI_PASSWORD "chiuchiu"

// MQTT Broker settings
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC "sensor/slave1/data"
#define MQTT_CLIENT_ID "ESP32_Master_BLE_Gateway"

#define SLAVE1_ADDR "00:4b:12:3a:da:d6"
// #define SLAVE2_ADDR "24:6F:28:AB:CD:02"
// #define SLAVE3_ADDR "24:6F:28:AB:CD:03"

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-0987654321ab"

BLEClient* client1;
BLERemoteCharacteristic* c1;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String d1; // Data from slave 1
unsigned long lastMqttAttempt = 0;
const long mqttReconnectInterval = 5000;

// --- Callback when slave sends notification ---
void notifyCallback1(BLERemoteCharacteristic* c, uint8_t* data, size_t len, bool isNotify)
{
    // Properly construct string with exact length (data is not null-terminated)
    d1 = String((char*)data, len);
    Serial.print("Slave1CC: ");
    Serial.println(d1);
}

// --- Connect to WiFi ---
void setupWiFi()
{
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("WiFi connection failed!");
    }
}

// --- Connect to MQTT Broker ---
void reconnectMQTT()
{
    // Don't attempt to reconnect too frequently
    if (millis() - lastMqttAttempt < mqttReconnectInterval) {
        return;
    }
    lastMqttAttempt = millis();

    if (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect(MQTT_CLIENT_ID)) {
            Serial.println("connected!");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" - will retry in 5 seconds");
        }
    }
}

// --- Connect to one slave ---
bool connectToSlave(BLEClient* client, const char* addr, void (*cb)(BLERemoteCharacteristic*, uint8_t*, size_t, bool))
{
    BLEAddress bleAddr(addr);
    if (!client->connect(bleAddr)) {
        Serial.print("Failed to connect to ");
        Serial.println(addr);
        return false;
    }
    Serial.print("Connected to ");
    Serial.println(addr);

    // Request larger MTU to handle full JSON payload (default is 23 bytes, only 20 usable)
    client->setMTU(512);
    Serial.println("MTU set to 512 bytes");

    BLERemoteService* pService = client->getService(SERVICE_UUID);
    if (pService == nullptr) {
        Serial.println("Failed to find service");
        client->disconnect();
        return false;
    }

    BLERemoteCharacteristic* pChar = pService->getCharacteristic(CHARACTERISTIC_UUID);
    if (pChar == nullptr) {
        Serial.println("Failed to find characteristic");
        client->disconnect();
        return false;
    }

    pChar->registerForNotify(cb);
    Serial.println("Notification registered!");
    return true;
}

void setup()
{
    Serial.begin(115200);

    // Initialize WiFi
    setupWiFi();

    // Initialize MQTT
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

    // Initialize BLE
    BLEDevice::init("");
    client1 = BLEDevice::createClient();

    connectToSlave(client1, SLAVE1_ADDR, notifyCallback1);
    delay(200);
    // connectToSlave(client2, SLAVE2_ADDR, notifyCallback2);
    // delay(200);
    // connectToSlave(client3, SLAVE3_ADDR, notifyCallback3);
}

void loop()
{
    // Ensure MQTT is connected
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();

    Serial.println("LoRa TX Slave1: " + d1);
    // Process and publish sensor data
    if (d1.length() > 0) {
        Serial.println("LoRa TX Slave1: " + d1);

        // Publish to MQTT if connected
        if (mqttClient.connected()) {
            if (mqttClient.publish(MQTT_TOPIC, d1.c_str())) {
                Serial.println("MQTT: Data published successfully");
            } else {
                Serial.println("MQTT: Failed to publish data");
            }
        } else {
            Serial.println("MQTT: Not connected, skipping publish");
        }

        d1 = "";
    }

    delay(2000);
}
