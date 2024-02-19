#include <Adafruit_NeoPixel.h>
#ifdef AVR
#include <avr/power.h>
#endif

int outputValue=0;
int rememberOutputValue;
int randNumber;
int counter = 0;
int loopCounter = 0;

#define PIN            15
#define NUMPIXELS      8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pixels.begin();
  randomSeed(analogRead(0));
  Serial.begin(9600);
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}


void loop() {

   int sensorValue; 
   counter = 0;
   
   for (int i=0; i < 100; i++){
    sensorValue = analogRead(A0);
    if(sensorValue > 100) counter++;
   }
  
  
  if(map(counter, 10, 60, 80, 80) > outputValue)outputValue = map(counter, 00, 40, 0, 40);
  else if(loopCounter %2 == 0)outputValue-=1;
  
  if(outputValue < 0) outputValue = 0;
  if(outputValue > 60) outputValue = 60;

 
  if(loopCounter % 100 == 0)randNumber = random(255);
  loopCounter++;

 for(int i=0;i < NUMPIXELS;i++){

    pixels.setPixelColor(i, pixels.Color(0,0,0, 255)); 
    
  }

 if(rememberOutputValue != outputValue){    
  for(int i=60;i > (60-outputValue) || (outputValue == 20 && i == 0);i--){
  pixels.setPixelColor(i, Wheel((randNumber) & 255));
 }
  
   pixels.show(); 
}   
   rememberOutputValue = outputValue;
}