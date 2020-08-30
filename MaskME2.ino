#include <ArduinoBLE.h>
//Trace and update on the Mask.ME parameters within iBeacon Protocol; including Contact-ID, (Insitu.)Temp. and Humidity
#include <BeaconNano.h>
#include <Arduino_HTS221.h>
BeaconNano bn;

void setup() {  
  Serial.begin(9600);
  while (!Serial);
  if (!HTS.begin()) {
    while (1);
  }
  HTS.begin();
  pinMode(3, OUTPUT);
  //bn.setUuid("99368260704bb0483b0ae75004121993");
  bn.setUuid("1D003FFFFFFFFFFFFFFFFFFFFFFF1234");
  int temperature = HTS.readTemperature();
  int humidity = HTS.readHumidity();
  bn.setMajor(humidity);
  bn.setMinor(temperature);
  bn.setTx(-60);
  bn.startBeacon();
  
}

void loop() {
    //bn.stopBeacon();
      //delay(5000);
      //bn.startBeacon();
      // Pull-down reset line for timed sensor update inside iBeacon packet.
      //digitalWrite(3, LOW);     
  }
