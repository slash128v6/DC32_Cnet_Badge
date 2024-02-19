#include <Adafruit_NeoPixel.h>

int outputValue = 0;
int rememberOutputValue;
int randNumber;
int counter = 0;
int loopCounter = 0;

#define SENSORPIN 34
#define LEDPIN 16
#define NUMPIXELS 8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

#define PLAYPIN 4
#define REWPIN 33
#define FWDPIN 15
#define VUPPIN 13
#define VDNPIN 12
#define MUTEPIN 14

#define PLAY 1
#define PAUSE 0
bool playState = PAUSE;
const int touchThreshold = 25;
const int touchDelay = 750;
int currVolume = 128;
int prevVolume = 0;
const int volStep = 10;
#define UNMUTED 1
#define MUTED 0
bool muteState = UNMUTED;

void readButtons();
void getButtons();
uint32_t wheel(byte wheelPos);
void vuMeter();

void setup() {
  pixels.begin();
  randomSeed(analogRead(0));
  Serial.begin(115200);

  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 27,
    .ws_io_num = 26,
    .data_out_num = 25,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.start("FONY BITMAN");
  a2dp_sink.set_volume(currVolume);
}

void loop() {
  //readButtons();
  getButtons();
  vuMeter();
}

void readButtons() {
  Serial.print("PlayPin = ");
  Serial.println(touchRead(PLAYPIN));

  Serial.print("RewPin = ");
  Serial.println(touchRead(REWPIN));

  Serial.print("FwdPin = ");
  Serial.println(touchRead(FWDPIN));

  Serial.print("VUpPin = ");
  Serial.println(touchRead(VUPPIN));

  Serial.print("VDnPin = ");
  Serial.println(touchRead(VDNPIN));

  Serial.print("MutePin = ");
  Serial.println(touchRead(MUTEPIN));

  delay(touchDelay);
}

void getButtons() {
  if (playState == PAUSE) {
    if (touchRead(PLAYPIN) < touchThreshold) {
      a2dp_sink.play();
      playState = PLAY;
      delay(touchDelay);
    }
  }

  if (playState == PLAY) {
    if (touchRead(PLAYPIN) < touchThreshold) {
      a2dp_sink.pause();
      playState = PAUSE;
      delay(touchDelay);
    }
  }

  if (touchRead(REWPIN) < touchThreshold) {
    a2dp_sink.previous();
  }

  if (touchRead(FWDPIN) < touchThreshold) {
    a2dp_sink.next();
    delay(touchDelay);
  }

  if (touchRead(VUPPIN) < touchThreshold) {
    currVolume += volStep;
    a2dp_sink.set_volume(currVolume);
    if (currVolume > 255) {
      currVolume = 255;
    }
    delay(touchDelay);
  }

  if (touchRead(VDNPIN) < touchThreshold) {
    currVolume -= volStep;
    a2dp_sink.set_volume(currVolume);
    if (currVolume < 0) {
      currVolume = 0;
    }
    delay(touchDelay);
  }

  if (muteState == UNMUTED) {
    if (touchRead(MUTEPIN) < touchThreshold) {
      prevVolume = a2dp_sink.get_volume();
      currVolume = 0;
      a2dp_sink.set_volume(currVolume);
      muteState = MUTED;
      delay(touchDelay);
    }
  }

  if (muteState == MUTED) {
    if (touchRead(MUTEPIN) < touchThreshold) {
      currVolume = prevVolume;
      a2dp_sink.set_volume(currVolume);
      muteState = UNMUTED;
      delay(touchDelay);
    }
  }
}

uint32_t wheel(byte wheelPos) {
  wheelPos = 255 - wheelPos;
  if (wheelPos < 85) {
    return pixels.Color(255 - wheelPos * 3, 0, wheelPos * 3, 0);
  }
  if (wheelPos < 170) {
    wheelPos -= 85;
    return pixels.Color(0, wheelPos * 3, 255 - wheelPos * 3, 0);
  }
  wheelPos -= 170;
  return pixels.Color(wheelPos * 3, 255 - wheelPos * 3, 0, 0);
}

void vuMeter() {
  int sensorValue;
  counter = 0;

  for (int i = 0; i < 100; i++) {
    sensorValue = map(analogRead(SENSORPIN), 0, 2048, 0, 100);
    if (sensorValue > 100) counter++;
  }

  if (map(counter, 10, 60, 80, 80) > outputValue) outputValue = map(counter, 00, 40, 0, 40);
  else if (loopCounter % 2 == 0) outputValue -= 1;

  if (outputValue < 0) outputValue = 0;
  if (outputValue > 60) outputValue = 60;

  if (loopCounter % 100 == 0) randNumber = random(255);
  loopCounter++;

  for (int i = 0; i < NUMPIXELS; i++) {

    pixels.setPixelColor(i, pixels.Color(0, 0, 0, 255));
  }

  if (rememberOutputValue != outputValue) {
    for (int i = 60; i > (60 - outputValue) || (outputValue == 20 && i == 0); i--) {
      pixels.setPixelColor(i, wheel((randNumber)&255));
    }

    pixels.show();
  }
  rememberOutputValue = outputValue;
}