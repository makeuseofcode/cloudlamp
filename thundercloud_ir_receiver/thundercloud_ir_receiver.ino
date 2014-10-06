/* 
Qyuick and dirty IR signal over i2c rebroadcast for
Lighting Cloud Mood Lamp By James Bruce
View the full tutorial and build guide at http://www.makeuseof.com/

Used to get around the limitations of having two libraries that both require exact timings to work right!
*/
#include <Wire.h>
#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Wire.begin(); // Start I2C Bus as Master
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    
    if(results.value != 0xFFFFFFFF){
      Serial.println(results.value,HEX);
      //only transmit if its not a repeat value. theyre useless. you may need to 
      // adjust for your own remote repeat values
      Wire.beginTransmission(9);
      Wire.write(results.value);
      Wire.endTransmission();
    }
    irrecv.enableIRIn();
    irrecv.resume(); // Receive the next value
  }
}
