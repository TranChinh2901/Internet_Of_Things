# ESP32 BLE to MQTT Node-RED Integration

This project receives sensor data from BLE slaves via ESP32 and publishes it to an MQTT broker for visualization in Node-RED.

## Architecture

```
BLE Slave Device → ESP32 (BLE Client) → WiFi → MQTT Broker → Node-RED Dashboard
```

## Prerequisites

1. **ESP32 with WiFi capability**
2. **Node-RED** (already installed and running)
3. **Node-RED Dashboard** nodes installed
4. **Internet connection** (for MQTT broker access)

## Setup Instructions

### Step 1: Update WiFi Credentials

Edit `src/main.cpp` and update your WiFi credentials:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### Step 2: Build and Upload to ESP32

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor -b 115200
```

### Step 3: Verify ESP32 Connection

After uploading, check the serial monitor for:
- WiFi connection confirmation
- MQTT broker connection
- BLE connection to slave device
- Data being published to MQTT

Expected output:
```
Connecting to WiFi: YourSSID
WiFi connected!
IP address: 192.168.x.x
Attempting MQTT connection...connected!
Connected to 00:4b:12:3a:da:d6
Slave1CC: {"temp":29.70,"hum":65.0}
LoRa TX Slave1: {"temp":29.70,"hum":65.0}
MQTT: Data published successfully
```

### Step 4: Import Node-RED Flow

1. Open Node-RED in your browser: http://127.0.0.1:1880/
2. Click the menu (☰) in the top-right corner
3. Select **Import**
4. Click **select a file to import**
5. Choose `nodered-flow.json` from this directory
6. Click **Import**
7. Click **Deploy** (red button in top-right)

### Step 5: View Dashboard

1. Open the dashboard: http://127.0.0.1:1880/ui
2. You should see the "Sensor Dashboard" tab
3. The dashboard displays:
   - **Temperature Gauge** (0-50°C)
   - **Humidity Gauge** (0-100%)
   - **Temperature History Chart** (last 10 minutes)
   - **Humidity History Chart** (last 10 minutes)
   - **Current Values** (text display)

## Node-RED Flow Details

The flow consists of:

1. **MQTT Input Node**: Subscribes to `sensor/slave1/data` topic
2. **JSON Parser**: Converts JSON string to JavaScript object
3. **Debug Node**: Shows raw data in debug panel
4. **Function Node**: Splits temperature and humidity into separate streams
5. **Gauge Nodes**: Real-time circular gauges for temp and humidity
6. **Chart Nodes**: Line charts showing historical data
7. **Text Nodes**: Display current numerical values

## MQTT Configuration

- **Broker**: broker.emqx.io (public MQTT broker)
- **Port**: 1883
- **Topic**: sensor/slave1/data
- **Client ID**: ESP32_Master_BLE_Gateway
- **QoS**: 0 (At most once)

## Troubleshooting

### ESP32 Not Connecting to WiFi
- Verify SSID and password are correct
- Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Ensure WiFi network is reachable

### MQTT Connection Failed
- Check internet connectivity
- Verify broker address: broker.emqx.io
- Try pinging: `ping broker.emqx.io`
- Check firewall settings

### No Data in Node-RED Dashboard
- Verify ESP32 is publishing data (check serial monitor)
- Check MQTT broker connection in Node-RED (should show "connected")
- Ensure the topic matches: `sensor/slave1/data`
- Check Node-RED debug panel for incoming messages

### BLE Connection Issues
- Ensure BLE slave device is powered on and in range
- Verify BLE address is correct: `00:4b:12:3a:da:d6`
- Check service and characteristic UUIDs match

## Customization

### Change MQTT Topic
In `src/main.cpp`:
```cpp
#define MQTT_TOPIC "your/custom/topic"
```

In Node-RED flow, update the MQTT input node's topic.

### Adjust Gauge Ranges
In Node-RED, double-click the gauge nodes and modify:
- **min/max values**
- **color segments**
- **warning thresholds**

### Chart Time Window
In Node-RED chart nodes, modify:
- **removeOlder**: Duration to keep data (default: 10 minutes)
- **removeOlderUnit**: Time unit (60 = minutes)

### Add Multiple Slaves
Uncomment and configure additional slaves in `src/main.cpp`:
```cpp
#define SLAVE2_ADDR "24:6F:28:AB:CD:02"
#define SLAVE3_ADDR "24:6F:28:AB:CD:03"
```

Create separate MQTT topics for each slave and duplicate the Node-RED flow.

## Data Format

The ESP32 publishes JSON data in this format:
```json
{
  "temp": 29.70,
  "hum": 65.0
}
```

- **temp**: Temperature in degrees Celsius
- **hum**: Relative humidity in percentage

## Files

- `src/main.cpp`: ESP32 firmware with BLE, WiFi, and MQTT
- `platformio.ini`: PlatformIO project configuration
- `nodered-flow.json`: Node-RED flow for data visualization
- `README_NodeRED_Setup.md`: This file

## Dependencies

### ESP32 Libraries
- `WiFi.h`: WiFi connectivity
- `PubSubClient.h`: MQTT client
- `BLEDevice.h`: BLE functionality
- `ArduinoJson.h`: JSON parsing

### Node-RED Packages
- `node-red-dashboard`: UI components (gauges, charts, text)
- MQTT broker node (built-in)

## Support

For issues or questions:
1. Check serial monitor output for error messages
2. Verify all connections (WiFi, MQTT, BLE)
3. Check Node-RED debug panel
4. Review MQTT broker status

## License

This project is provided as-is for educational and development purposes.
