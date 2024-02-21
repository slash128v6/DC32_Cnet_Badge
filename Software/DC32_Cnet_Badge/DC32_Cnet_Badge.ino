#include <Adafruit_NeoPixel.h>

int outputValue = 0;
int rememberOutputValue;
int randNumber;
int counter = 0;
int loopCounter = 0;

#define SENSORPIN 34
#define LEDPIN 16
#define NUMPIXELS 16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

#include "BluetoothA2DPSink.h"
BluetoothA2DPSink a2dp_sink;


#define PLAYPIN 4
#define RWDPIN 33
#define FWDPIN 15
#define VDNPIN 12
#define VUPPIN 13
#define MUTEPIN 14


#define PLAY 1
#define PAUSE 0
bool playState = PAUSE;
int audioState = 0;
const int touchThreshold = 25;
const int touchDelay = 250;
int currVolume = 0;
int prevVolume = 0;
const int volStep = 10;
#define UNMUTED 1
#define MUTED 0
bool muteState = UNMUTED;
bool btConnected = false;


#define CASSPIN_A 21
#define CASSPIN_B 22
#define CASSPIN_C 23
int prevCassPin = 21;
int currCassPin = 23;


uint32_t currMillis = 0;
uint32_t prevMillis = 0;


void setVolume();
void readButtons();
void getButtons();
uint32_t colorPicker(byte colorPos);
void ledBars(int currVolLevel);
void cassetteSAOPlay();
void cassetteSAORewind();


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
	
	pinMode(CASSPIN_A, OUTPUT);
	pinMode(CASSPIN_B, OUTPUT);
	pinMode(CASSPIN_C, OUTPUT);

}


void loop() {
	
	readButtons();  // for debugging touch button thresholds
	setVolume();
	getButtons();
	ledBars(currVolume);
	cassetteSAOPlay();
}


void setVolume() {
	
	btConnected = a2dp_sink.is_connected();
	Serial.println("btConnected = " + String(btConnected));
	if(btConnected) {
		currVolume = a2dp_sink.get_volume();
		a2dp_sink.set_volume(currVolume);
	}

}


void readButtons() {
	
	Serial.println("PlayPin = " + String(touchRead(PLAYPIN)));
	Serial.println("RwdPin = " + String(touchRead(RWDPIN)));
	Serial.println("FwdPin = " + String(touchRead(FWDPIN)));
	Serial.println("VDnPin = " + String(touchRead(VDNPIN)));
	Serial.println("VUpPin = " + String(touchRead(VUPPIN)));
	Serial.println("MutePin = " + String(touchRead(MUTEPIN)));
	
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

	if (touchRead(RWDPIN) < touchThreshold) {
		a2dp_sink.previous();
	}

	if (touchRead(FWDPIN) < touchThreshold) {
		a2dp_sink.next();
		delay(touchDelay);
	}

	if (touchRead(VDNPIN) < touchThreshold) {
		currVolume -= volStep;
		if (currVolume < 0) {
			currVolume = 0;
		}
		a2dp_sink.set_volume(currVolume);
		delay(touchDelay);
	}

	if (touchRead(VUPPIN) < touchThreshold) {
		currVolume += volStep;
		if (currVolume > 255) {
			currVolume = 255;
		}
		a2dp_sink.set_volume(currVolume);
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


uint32_t colorPicker(byte colorPos) {
	colorPos = 255 - colorPos;
	if (colorPos < 85) {
		return pixels.Color(255 - colorPos * 3, 0, colorPos * 3, 0);
	}
	if (colorPos < 170) {
		colorPos -= 85;
		return pixels.Color(0, colorPos * 3, 255 - colorPos * 3, 0);
	}
	colorPos -= 170;
	return pixels.Color(colorPos * 3, 255 - colorPos * 3, 0, 0);
}


void ledBars(int currVolLevel) {
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
			pixels.setPixelColor(i, colorPicker((randNumber)&255));
		}
		pixels.show();
	}
	
	rememberOutputValue = outputValue;
}


void cassetteSAOPlay() {
	
	audioState = a2dp_sink.get_audio_state();
	Serial.println("audioState = " + String(audioState)); // for debugging play/pause state
	uint32_t cassMoveLEDsMillis = 250;
	currMillis = millis();
	if((currMillis - prevMillis > cassMoveLEDsMillis) && (audioState == 2)) {
		
		digitalWrite(prevCassPin, LOW);
		digitalWrite(currCassPin, HIGH);

		prevCassPin--;
		if(prevCassPin < 21) {
			prevCassPin = 23;
		}

		currCassPin--;
		if(currCassPin < 21) {
			currCassPin = 23;
		}

		prevMillis = currMillis;
	}

}


void cassetteSAORewind() {
	
	audioState = a2dp_sink.get_audio_state();
	Serial.println("audioState = " + String(audioState)); // for debugging play/pause state
	uint32_t cassMoveLEDsMillis = 100;
	currMillis = millis();
	if((currMillis - prevMillis > cassMoveLEDsMillis) && (audioState == 2)) {
		
		digitalWrite(prevCassPin, LOW);
		digitalWrite(currCassPin, HIGH);
		prevCassPin++;
		if(prevCassPin > 23) {
			prevCassPin = 21;
		}
		currCassPin++;
		if(currCassPin > 23) {
			currCassPin = 21;
		}
		
		prevMillis = currMillis;
	}

}