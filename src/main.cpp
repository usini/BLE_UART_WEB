#include <NimBLEDevice.h>

NimBLECharacteristic *pTxCharacteristic;
NimBLEServer *pServer;

NimBLECharacteristic *pSoftwareVersionCharacteristic;
NimBLECharacteristic *pBatteryLevelCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
String data_string = "TEST";
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
  /*******************************************************************/
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);

      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup()
{
  Serial.begin(115200);

  // Create the BLE Device
  NimBLEDevice::init("NimBLE");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setMTU(256);
  // Create the BLE Server
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  Serial.println("Create Service");
  // Create the BLE Service
  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  Serial.println("Create Characteristic");
  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      /******* Enum Type NIMBLE_PROPERTY now *******
          BLECharacteristic::PROPERTY_NOTIFY
          );
      **********************************************/
      NIMBLE_PROPERTY::NOTIFY);

  /***************************************************
   NOTE: DO NOT create a 2902 descriptor
   it will be created automatically if notifications
   or indications are enabled on a characteristic.

   pCharacteristic->addDescriptor(new BLE2902());
  ****************************************************/

  NimBLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      /******* Enum Type NIMBLE_PROPERTY now *******
              BLECharacteristic::PROPERTY_WRITE
              );
      *********************************************/
      NIMBLE_PROPERTY::WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  Serial.println("Service starting...");
  // Start the service
  pService->start();
  Serial.println("Service started...");

  Serial.println("Start Advertising");
  // Start advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  Serial.println("Advertising started");

  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop()
{

  if (deviceConnected)
  {
    pTxCharacteristic->setValue(data_string);
    Serial.println("Notify");
    pTxCharacteristic->notify();
    delay(300); // bluetooth stack will go into congestion, if too many packets are sent
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}