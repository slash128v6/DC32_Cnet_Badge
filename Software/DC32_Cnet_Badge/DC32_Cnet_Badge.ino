/*
@slash128
@compunetbiz
DEF CON 32
*/

//#include <BLEDevice.h>
#include <esp_bt.h>

#include <Adafruit_NeoPixel.h>
#define LEDPIN 16
#define NUMPIXELS 16
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);


#include "BluetoothA2DPSink.h"
BluetoothA2DPSink a2dp_sink;

#define SENSORPIN 34
#define PLAYPIN 4
#define RWDPIN 33
#define FWDPIN 15
#define VDNPIN 12
#define VUPPIN 13
#define MUTEPIN 14
#define PLAY 1
#define PAUSE 0
bool playButton = 0;
bool rwdButton = 0;
bool fwdButton = 0;
bool vdnButton = 0;
bool vupButton = 0;
bool muteButton = 0;
bool playState = PAUSE;
int audioState = 0;
const int touchThresholdPlay = 15;
const int touchThresholdRwd = 15;
const int touchThresholdFwd = 15;
const int touchThresholdVdn = 15;
const int touchThresholdVup = 15;
const int touchThresholdMute = 15;
const int touchDelay = 250;
int currVolume = 0;
int prevVolume = 0;
const int volStep = 10;
#define UNMUTED 1
#define MUTED 0
bool muteState = UNMUTED;
bool btConnected = false;
#define PRESSED 1
#define NOTPRESSED 0


#define CASSPIN_A 21
#define CASSPIN_B 22
#define CASSPIN_C 23
int prevCassPin = 21;
int currCassPin = 23;


uint32_t prevMillis = 0;
uint32_t lastPlayDebounceTime = 0;
uint32_t lastRwdDebounceTime = 0;
uint32_t lastFwdDebounceTime = 0;
uint32_t lastVdnDebounceTime = 0;
uint32_t lastVupDebounceTime = 0;
uint32_t lastMuteDebounceTime = 0;

uint8_t touchSumPlay = 0;
uint8_t touchAvgPlay = 0;
uint8_t countPlay = 0;
uint8_t touchSumRwd = 0;
uint8_t touchAvgRwd = 0;
uint8_t countRwd = 0;
uint8_t touchSumFwd = 0;
uint8_t touchAvgFwd = 0;
uint8_t countFwd = 0;
uint8_t touchSumVdn = 0;
uint8_t touchAvgVdn = 0;
uint8_t countVdn = 0;
uint8_t touchSumVup = 0;
uint8_t touchAvgVup = 0;
uint8_t countVup = 0;
uint8_t touchSumMute = 0;
uint8_t touchAvgMute = 0;
uint8_t countMute = 0;



int outputValue = 0;
int rememberOutputValue;
int randNumber;
int counter = 0;
int loopCounter = 0;


void getTouchButtons();
uint32_t colorPicker(byte colorPos);
void ledBars(int currVolLevel);
void cassetteSAOPlay();
void cassetteSAORewind();
void debug();


void setup() {
	pixels.begin();
	randomSeed(analogRead(0));
	Serial.begin(921600);

	i2s_pin_config_t my_pin_config = {
		.bck_io_num = 27,
		.ws_io_num = 26,
		.data_out_num = 25,
		.data_in_num = I2S_PIN_NO_CHANGE
	};

	a2dp_sink.set_pin_config(my_pin_config);
	a2dp_sink.start("FONY BITMAN");

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);

	pinMode(PLAYPIN, INPUT_PULLUP);
	pinMode(RWDPIN, INPUT_PULLUP);
	pinMode(FWDPIN, INPUT_PULLUP);
	pinMode(VDNPIN, INPUT_PULLUP);
	pinMode(VUPPIN, INPUT_PULLUP);
	pinMode(MUTEPIN, INPUT_PULLUP);

	pinMode(CASSPIN_A, OUTPUT);
	pinMode(CASSPIN_B, OUTPUT);
	pinMode(CASSPIN_C, OUTPUT);

}


void loop() {

	getTouchButtons();
	ledBars(currVolume); // is there a way to alter the LED bars behavior based on currVolume - challenge anyone?
	cassetteSAOPlay();
	debug();  // for debugging - duh :)

}




void getTouchButtons() {

	unsigned long currMillis = millis();
	
	if (touchRead(PLAYPIN) < touchThresholdPlay) {
		touchSumPlay += touchRead(PLAYPIN);
		countPlay += 1;
		if (currMillis - lastPlayDebounceTime > touchDelay) {
			lastPlayDebounceTime = currMillis;
			touchAvgPlay = touchSumPlay / countPlay;
			touchSumPlay = 0;
			countPlay = 0;
			if (touchAvgPlay < touchThresholdPlay) {
				
				// If it's still below the threshold, set the boolean flag
				playButton = PRESSED;
				
				} else {
				playButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			playButton = NOTPRESSED;
		}
	}


	if (playState == PAUSE && playButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		a2dp_sink.set_volume(currVolume);
		a2dp_sink.play();
		playState = PLAY;
		playButton = NOTPRESSED;
	}

	if (playState == PLAY && playButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		a2dp_sink.set_volume(currVolume);
		a2dp_sink.pause();
		playState = PAUSE;
		playButton = NOTPRESSED;
	}


	if (touchRead(RWDPIN) < touchThresholdRwd) {
		touchSumRwd += touchRead(RWDPIN);
		countRwd += 1;
		if (currMillis - lastRwdDebounceTime > touchDelay) {
			lastRwdDebounceTime = currMillis;
			touchAvgRwd = touchSumRwd / countRwd;
			touchSumRwd = 0;
			countRwd = 0;
			if (touchAvgRwd < touchThresholdRwd) {
				
				// If it's still below the threshold, set the boolean flag
				rwdButton = PRESSED;
				
				} else {
				rwdButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			rwdButton = NOTPRESSED;
		}
	}

	if (rwdButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		a2dp_sink.set_volume(currVolume);
		a2dp_sink.previous();
		rwdButton = NOTPRESSED;
	}


	if (touchRead(FWDPIN) < touchThresholdFwd) {
		touchSumFwd += touchRead(FWDPIN);
		countFwd += 1;
		if (currMillis - lastFwdDebounceTime > touchDelay) {
			lastFwdDebounceTime = currMillis;
			touchAvgFwd = touchSumFwd / countFwd;
			touchSumFwd = 0;
			countFwd = 0;
			if (touchAvgFwd < touchThresholdFwd) {
				
				// If it's still below the threshold, set the boolean flag
				fwdButton = PRESSED;
				
				} else {
				fwdButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			fwdButton = NOTPRESSED;
		}
	}


	if (fwdButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		a2dp_sink.set_volume(currVolume);
		a2dp_sink.next();
		fwdButton = NOTPRESSED;
	}


	if (touchRead(VDNPIN) < touchThresholdVdn) {
		touchSumVdn += touchRead(VDNPIN);
		countVdn += 1;
		if (currMillis - lastVdnDebounceTime > touchDelay) {
			lastVdnDebounceTime = currMillis;
			touchAvgVdn = touchSumVdn / countVdn;
			touchSumVdn = 0;
			countVdn = 0;
			if (touchAvgVdn < touchThresholdVdn) {
				
				// If it's still below the threshold, set the boolean flag
				vdnButton = PRESSED;
				
				} else {
				vdnButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			vdnButton = NOTPRESSED;
		}
	}

	if (vdnButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		currVolume -= volStep;
		if (currVolume < 0) {
			currVolume = 0;
		}
		a2dp_sink.set_volume(currVolume);
		vdnButton = NOTPRESSED;
	}


	if (touchRead(VUPPIN) < touchThresholdVup) {
		touchSumVup += touchRead(VUPPIN);
		countVup += 1;
		if (currMillis - lastVupDebounceTime > touchDelay) {
			lastVupDebounceTime = currMillis;
			touchAvgVup = touchSumVup / countVup;
			touchSumVup = 0;
			countVup = 0;
			if (touchAvgVup < touchThresholdVup) {
				
				// If it's still below the threshold, set the boolean flag
				vupButton = PRESSED;
				
				} else {
				vupButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			vupButton = NOTPRESSED;
		}
	}

	if (vupButton == PRESSED) {
		currVolume = a2dp_sink.get_volume();
		currVolume += volStep;
		if (currVolume > 255) {
			currVolume = 255;
		}
		a2dp_sink.set_volume(currVolume);
		vupButton = NOTPRESSED;
	}


	if (touchRead(MUTEPIN) < touchThresholdMute) {
		touchSumMute += touchRead(MUTEPIN);
		countMute += 1;
		if (currMillis - lastMuteDebounceTime > touchDelay) {
			lastMuteDebounceTime = currMillis;
			touchAvgMute = touchSumMute / countMute;
			touchSumMute = 0;
			countMute = 0;
			if (touchAvgMute < touchThresholdMute) {
				
				// If it's still below the threshold, set the boolean flag
				muteButton = PRESSED;
				
				} else {
				muteButton = NOTPRESSED;
			}
			} else {
			// If the variable is not below the threshold, reset the boolean flag
			muteButton = NOTPRESSED;
		}
	}

	if (muteState == UNMUTED && muteButton == PRESSED) {
		prevVolume = a2dp_sink.get_volume();
		currVolume = 0;
		a2dp_sink.set_volume(currVolume);
		muteState = MUTED;
		muteButton = NOTPRESSED;
	}

	if (muteState == MUTED && muteButton == PRESSED) {
		currVolume = prevVolume;
		a2dp_sink.set_volume(currVolume);
		muteState = UNMUTED;
		muteButton = NOTPRESSED;
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

	uint32_t cassMoveLEDsMillis = 150;
	uint32_t currMillis = millis();
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


void cassetteSAORewind() { // WIP to implement a rewind animation, is there something to key on in the a2dp_sink library for rewind state - challenge anyone?

	audioState = a2dp_sink.get_audio_state();

	uint32_t cassMoveLEDsMillis = 75;
	uint32_t currMillis = millis();
	if((currMillis - prevMillis > cassMoveLEDsMillis) && (audioState == 2)) {

		digitalWrite(prevCassPin, HIGH);
		digitalWrite(currCassPin, LOW);

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

void debug() {

	// Check the current BLE power level (might be useful for debugging)
	int32_t currentLevel = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_ADV);
	Serial.print("Current BLE Power Level: ");
	Serial.println(currentLevel); // This will likely print 5 (corresponding to +3dBm)
	
	Serial.println("audioState = " + String(audioState));
	
	Serial.println("PlayPin = " + String(touchRead(PLAYPIN)));
	Serial.println("RwdPin = " + String(touchRead(RWDPIN)));
	Serial.println("FwdPin = " + String(touchRead(FWDPIN)));
	Serial.println("VDnPin = " + String(touchRead(VDNPIN)));
	Serial.println("VUpPin = " + String(touchRead(VUPPIN)));
	Serial.println("MutePin = " + String(touchRead(MUTEPIN)));
}