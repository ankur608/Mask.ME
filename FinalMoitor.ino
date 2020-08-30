#include <Arduino_HTS221.h>
#include <Wire.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#define CCS811_ADDR 0x5B
#define PIN_NOT_WAKE 5
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;
#define MAXIMWIRE_EXTERNAL_PULLUP
#include <MaximWire.h>
#define PIN_BUS 2
MaximWire::Bus bus(PIN_BUS);
MaximWire::DS18B20 device;

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  if (!HTS.begin()) {
  while (1);
  }
  Serial.println();
  Serial.println("Apply BME280 data to CCS811 for compensation.");

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
}
void loop()
{
  float temperature = HTS.readTemperature();
  float humidity    = HTS.readHumidity();
  //delay(1000);

      MaximWire::Discovery discovery = bus.Discover();
    do {
        MaximWire::Address address;
        if (discovery.FindNextDevice(address)) {
            if (address.GetModelCode() == MaximWire::DS18B20::MODEL_CODE) {
                //Serial.print(" (DS18B20)");
                MaximWire::DS18B20 device(address);
                float temp = device.GetTemperature<float>(bus);
                Serial.print("Thermal Mesh Temperature: ");
                Serial.print(temp);
                Serial.print(" °C");
                Serial.println();
                Serial.print("Ambient Temperature: ");
                Serial.print(temperature);
                Serial.println(" °C");
                Serial.print("Relative Humidity: ");
                Serial.print(humidity);
                Serial.println(" %");
                Serial.println();
                device.Update(bus);
            } else {
                Serial.println();
            }
        }
    } while (discovery.HaveMore());
    //delay(1000);
  
  if (myCCS811.dataAvailable())
  {
    myCCS811.readAlgorithmResults();
    printInfoSerial();
    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();
    Serial.print("Applying new values (deg C, %): ");
    Serial.print(BMEtempC);
    Serial.print(",");
    Serial.println(BMEhumid);
    Serial.println();
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    printSensorError();
  }
  delay(2000); //Wait for next reading
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
