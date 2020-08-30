// For Scanning Insitu Exhale Parameters via nRFConnect or any BLE Service App
// Carbon-dioxide & Volatile OCs have been pseudo-mapped under Pressure & Rainfall respectively(ppm & ppb);)
#include <ArduinoBLE.h>
#include <Wire.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#define CCS811_ADDR 0x5B
#define PIN_NOT_WAKE 5
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

BLEService sensorService("181A");  // Environmental_Sensor-Service
// 180D HeartRate
// 181B BodyComposi.
BLEShortCharacteristic sensorLevelChar1("2A6E", BLERead | BLENotify);   // Temperature-Characteristic
BLEShortCharacteristic sensorLevelChar2("2A6F", BLERead | BLENotify);   // Humidity-Characteristic
BLELongCharacteristic sensorLevelChar3("2A6D", BLERead | BLENotify);    // PCo2-Characteristic
BLELongCharacteristic sensorLevelChar4("2A78", BLERead | BLENotify);    // PTVoC-Characteristic
// 2A37 HRmeasurement
// 2A2C Magdeclination
// 2A98 weight
// 2A6D Pressure
// 2A9D Recom.HRMax.
// 2A8E Height
// 2A8D Max.HR
// 2A8C Gender
// 2A00 DeviceName
// 2A38 bodysensorlocation
int oldSensorLevel1 = 0; 
int oldSensorLevel2 = 0;
int oldSensorLevel3 = 0;
int oldSensorLevel4 = 0;
long previousMillis = 0; 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(23, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("Bluetooth init. failed!");
    while (1);
  }
  Wire.begin();
  CCS811Core::status returnCode = myCCS811.begin();
  Serial.print("CCS811 begin exited with: ");
  printDriverError( returnCode );
  Serial.println();
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;
  delay(10);
  myBME280.begin();

  BLE.setLocalName("Mask.xME");
  BLE.setAdvertisedService(sensorService);
  sensorService.addCharacteristic(sensorLevelChar1);
  BLE.addService(sensorService);
  sensorLevelChar1.writeValue(oldSensorLevel1);

  sensorService.addCharacteristic(sensorLevelChar2);
  BLE.addService(sensorService);
  sensorLevelChar2.writeValue(oldSensorLevel2);

  sensorService.addCharacteristic(sensorLevelChar3);
  BLE.addService(sensorService);
  sensorLevelChar3.writeValue(oldSensorLevel3);

  sensorService.addCharacteristic(sensorLevelChar4);
  BLE.addService(sensorService);
  sensorLevelChar4.writeValue(oldSensorLevel4);

  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth active, waiting to be connected.....");
}

void loop() {
  // waiting for a BLE central device.
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(23, HIGH);  // indicate connection
    while (central.connected()) {
  if (myCCS811.dataAvailable())
  {
    myCCS811.readAlgorithmResults();
    printInfoSerial();
    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    printSensorError();
  }
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateSensorLevel1();
        updateSensorLevel2();
        updateSensorLevel3();
        updateSensorLevel4();
      }
    }
    digitalWrite(23, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateSensorLevel1() {
  float temperature = myBME280.readTempC();
  Serial.print("Temperature = ");
  Serial.print(temperature, 0);
  //float T = myBME280.readTempC();
  Serial.println(" °C");
  float sensorLevel1 = map(temperature, 0, 100, 0, 10000);
  sensorLevelChar1.writeValue(sensorLevel1);
  oldSensorLevel1 = sensorLevel1;
}

void updateSensorLevel2() {
  float humidity = myBME280.readFloatHumidity();
  Serial.print("Humidity = ");
  Serial.print(humidity, 0);
  Serial.println(" %");
  float sensorLevel2 = map(humidity, 0, 100, 0, 10000);
  sensorLevelChar2.writeValue(sensorLevel2); 
  oldSensorLevel2 = sensorLevel2; 
}

void updateSensorLevel3() {
  float CO2 = myCCS811.getCO2();  
  Serial.print("Co2 = ");
  Serial.print(CO2);
  Serial.println(" ppm");
  float sensorLevel3 = map(CO2, 0, 100, 0, 1000);
  sensorLevelChar3.writeValue(sensorLevel3); 
  oldSensorLevel3 = sensorLevel3; 
}

void updateSensorLevel4() {
  float VoC = myCCS811.getTVOC();
  Serial.print("TVoC = ");
  Serial.print(VoC);
  Serial.println(" ppb");
  float sensorLevel4 = map(VoC, 0, 100, 0, 100);
  sensorLevelChar4.writeValue(sensorLevel4); 
  oldSensorLevel4 = sensorLevel4; 
}

void printInfoSerial()
{
  Serial.println("CCS811 data:");
  Serial.print(" CO2 concentration : ");
  Serial.print(myCCS811.getCO2());
  Serial.println(" ppm");
  Serial.print(" TVOC concentration : ");
  Serial.print(myCCS811.getTVOC());
  Serial.println(" ppb");

  Serial.println("BME280 data:");
  Serial.print(" Temperature: ");
  Serial.print(myBME280.readTempC(), 2);
  Serial.println(" degrees C");
  Serial.print(" %RH: ");
  Serial.print(myBME280.readFloatHumidity(), 2);
  Serial.println(" %");
  Serial.println();
}

void printDriverError( CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();
  if ( error == 0xFF ) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
