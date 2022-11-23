#define USE_ARDUINO_INTERRUPTS true 
#include <PulseSensorPlayground.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2,1,0,4,5,6,7,3, POSITIVE);
 

const int PulseWire = 0; 
const int LED13 = 13; 
int Threshold = 550; 


 
PulseSensorPlayground pulseSensor; 
void setup() {
 
Serial.begin(9600); 
lcd.begin(16,2);
 

pulseSensor.analogInput(PulseWire);
pulseSensor.blinkOnPulse(LED13); 
pulseSensor.setThreshold(Threshold);
 

if (pulseSensor.begin()) {
lcd.print(" Monitoreo de Ritmo Cardiaco");
 
}
}
 
void loop() {
 
int myBPM = pulseSensor.getBeatsPerMinute(); 

if (pulseSensor.sawStartOfBeat()) { 
Serial.println("♥ Pulso Cardiaco detectado ! "); 
Serial.print("BPM: "); 
Serial.println(myBPM); 

lcd.print("Pulso Cardiaco detectado !"); 
lcd.print("BPM: "); 
lcd.print(myBPM);
}
delay(20); 
}