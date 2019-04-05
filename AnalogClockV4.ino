/*
 * MIT License
 * 
 * Copyright (c) 2018 Leon van den Beukel, 2019 Schn33W0lf
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Original: 
 * https://github.com/leonvandenbeukel/3D-7-Segment-Digital-Clock
 * Source:
 * https://github.com/Schn33W0lf/24-Segment-Analog-Clock/
 * 
 * External libraries you need:
 * Adafruit Sensor Library:	https://github.com/adafruit/Adafruit_Sensor
 * Adafruit DHT:		https://github.com/adafruit/DHT-sensor-library
 * Adafruit RTCLib:		https://github.com/adafruit/RTClib
 * FastLED:			https://github.com/FastLED/FastLED
 * Timer Library:		https://github.com/JChristensen/Timer
 */

/*
 * TODO: Timer: show milliseconds with cricles?
 *
 * INFO: Because of performance problems, I commented most [D]ebug messages out.
 * INFO: For Debugging:
 *       [T] = Test debug   (for short tests/ finding bugs (development versions))
 *       [D] = Debug        (to tell what the arduino is doing (less important, development versions))
 *       [I] = Info         (to tell what the arduino is doing (important stuff))
 *       [W] = Warning      (something bad happened but it should be ok)
 *       [E] = Error        (something bad happened (eg wrong user input))
 *       [F] = Fatal Error  (something really bad happened)
 */

// Includes
#include <DHT.h>
#include <DHT_U.h>
#include <FastLED.h>
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
#include "Timer.h"
#include <string.h>
#include <TM1637Display.h>

// Setup for TM1637
/// Arduino(D2)----TM1637(CLK)
/// Arduino(D3)----TM1637(DIO)
TM1637Display display(2, 3);
const byte numbersWithDot[] = {
    0b10111111, // 0
    0b10000110, // 1
    0b11011011, // 2
    0b11001111, // 3
    0b11100110, // 4
    0b11101101, // 5
    0b11111101, // 6
    0b10000111, // 7
    0b11111111, // 8
    0b11101111, // 9
};
const byte numbersWithoutDot[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
};
const byte extras[] = {
    0b01100011, // º
    0b00111001, // C(elcius)
    0b01011100, // º lower
    0b01110001, // F(ahrenheit)
};

// Setup DHT (Temp & Hum)
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Setup FastLED
#define NUM_LEDS 24
/// Arduino(D6)----LED(DI)
#define DATA_PIN 6
#define settings_led_chip WS2812B
#define settings_led_type GRB
CRGB LEDs[NUM_LEDS];

// Setup Bluetooth
/// BTserial(RXD, TXD);
/// Arduino(D9)-----BT Module(RXD)
/// Arduino(D8)-----BT Module(TXD)
SoftwareSerial BTserial(8, 9);

String btBuffer = "";
bool btReading = false;
#define btStartEnd '|'
#define btSeperate ','

// Setup RTC
RTC_DS3231 rtc;

// Setup for Timer
Timer t1;
Timer t2;

short refresh = 0;
volatile bool timerRunning = false;
volatile int timerValue = 0;
volatile int scoreLeft;
volatile int scoreRight;

// Setup for Clock
short muxReturn[2];

volatile short mode = 1;
volatile bool modeToggle = false;
const int toggleInterval = 5;

char temperatureMode = 'C';

CRGB colorBackground = CRGB::DarkGoldenrod;
CRGB colorPrimary = CRGB::Blue;
CRGB colorSecondary = CRGB::Red;
float brightnessBackground = 2.56f; /// 1%
/**
 * Info for offset:
 * offset 0 means that the minute mod 5 is 0 (minutes: 0, 5, 10, 15, 20, ... 55)
 *        1 means that the minute mod 5 is 1 (minutes: 1, 4, 6,  9,  11, ... 59)
 *        2 means that the minute mod 5 is 2 (minutes: 2, 3, 7,  8,  12, ... 58)
 * the idea is, to tell with o0, that it is 5 past[before]
 *                           o1, that it is 5 past[before] BUT it is more near to 10 past[before] by 1 minute
 *                           o2, that it is 5 past[before] BUT it is more near to 10 past[before] by 2 minutes
 */
float brightnessOffset0 = 255.0f; /// 100%
float brightnessOffset1 = 51.2f; /// 10%
float brightnessOffset2 = 102.4f; /// 20%
bool backgroundLighting = false; /// false = quarters, true = all

// Setup
void setup () {

	// Initialize LED strip
	//FastLED.delay(3000);
	FastLED.addLeds<settings_led_chip, DATA_PIN, settings_led_type>(LEDs, NUM_LEDS);

	// Initialize Serial Monitor, wait for Serial Monitor
	/*Serial.begin(9600);
	while (!Serial) { /* Wait until serial is ready * / }
	Serial.println("[D] Serial started at 9600");*/

	// Initialize Serial Bluetooth
	BTserial.begin(9600);
	//Serial.println("[D] BTserial started at 9600");

	// Initialize DHT
	dht.begin();

	// Wait for RTC
	if (!rtc.begin()) {
		//Serial.println("[E] Couldn't find RTC.");
		while (1);
	}

	// Set RTC time if lost (set time to the time this sketch was compiled)
	if (rtc.lostPower()) {
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		//Serial.println("[I] RTC lost power, Reset the time.");
	}
	
	// Initialize TM1637 (clear)
	display.clear();
	display.setBrightness(0x0f);
	
	// Initialize Timers
	t1.every(1000 * 29, refreshDisplay);
	t2.every(1000, refreshTimer);
	refreshDisplay();
}

// Main
void loop () {
	// If there is BT available, read. (Format |ARG[,MORE ARGS, ...]|) 
	if (BTserial.available()) {
		char received = BTserial.read();
		// Ignore Bluetooth until btStartEnd character
		if (btReading) {
			// Read Bluetooth until btStartEnd character
			if (received == btStartEnd) {
				// Process & Reset btBuffer
				processBtBuffer();
			} else {
				btBuffer += (String)received;
			}
		} else if (received == btStartEnd) {
			btReading = true;
		}
		
	}
	
	t1.update();
	t2.update();

}

// Utils
/**
 * multiplexer:
 * xx   -> x x no dp
 * xx.x -> x x dp
 * 0x.x -> x x dp
 **/
void mux(float value, bool score) {
	if (score) {
		short tmp = (int)value;
		muxReturn[0] = numbersWithoutDot[tmp];
		muxReturn[1] = numbersWithDot[(int)((value - tmp) * 10)];
	} else {
		if (value < 10) {
			//x.x°C
			short tmp = (int)value;
			muxReturn[0] = numbersWithDot[tmp];
			muxReturn[1] = numbersWithoutDot[(int)((value - tmp) * 10)];
		} else {
			//xx°C
			muxReturn[0] = numbersWithoutDot[(int)value / 10];
			muxReturn[1] = numbersWithoutDot[(int)value % 10];
		}
	}
}
String getValue(String data, char separator, int index) {
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i+1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : (String)separator;
}
void(* resetFunc) (void) = 0; ///declare reset function @ address 0

// Methods
void processBtBuffer() {
	//Serial.println(btBuffer);
	btReading = false;
	if (btBuffer == "RESET") {
		//Serial.println("[I] Restarting Arduino...");
		//delay(1000);
		resetFunc();
	} else if (btBuffer == "PING") {
		BTserial.print("|STATUS,4,");
		BTserial.print((timerRunning ? 1 : 0));
		BTserial.print(',');
		BTserial.print(scoreLeft);
		BTserial.print(',');
		BTserial.print(scoreRight);
		BTserial.print(',');
		BTserial.print((modeToggle ? 1 : 0));
		BTserial.print(',');
		BTserial.print(mode);
		BTserial.print(',');
		BTserial.print(colorBackground.r);
		BTserial.print(',');
		BTserial.print(colorBackground.g);
		BTserial.print(',');
		BTserial.print(colorBackground.b);
		BTserial.print(',');
		BTserial.print(colorPrimary.r);
		BTserial.print(',');
		BTserial.print(colorPrimary.g);
		BTserial.print(',');
		BTserial.print(colorPrimary.b);
		BTserial.print(',');
		BTserial.print(colorSecondary.r);
		BTserial.print(',');
		BTserial.print(colorSecondary.g);
		BTserial.print(',');
		BTserial.print(colorSecondary.b);
		BTserial.print(',');
		BTserial.print((int)brightnessBackground);
		BTserial.print(',');
		BTserial.print((int)brightnessOffset0);
		BTserial.print(',');
		BTserial.print((int)brightnessOffset1);
		BTserial.print(',');
		BTserial.print((int)brightnessOffset2);
		BTserial.print(',');
		BTserial.print((backgroundLighting ? 1 : 0));
		BTserial.print('|');
		// [D] Serial.println("[D] Received PING, Sending PONG with VERSION 3");
	} else if (btBuffer == "CLOCK") {
		mode = 0;
		// [D] Serial.println("[D] Switched mode to 0");
	} else if (btBuffer == "TEMPERATURE") {
		mode = 1;
		// [D] Serial.println("[D] Switched mode to 1");
	} else if (btBuffer == "HUMIDITY") {
		mode = 2;
		// [D] Serial.println("[D] Switched mode to 2");
	} else if (btBuffer == "TIMER") {
		if (!timerRunning) {
			// Reset timer value before starting timer
			timerValue = 0;
		}
		// Set mode
		mode = 4;
		// Start/ stop timer
		timerRunning = !timerRunning;
		// Feedback
		/* [D] 
		String v = (timerRunning ? "Started" : "Stopped");
		Serial.print("[D] Switched mode to 4, ");
		Serial.print(v);
		Serial.println(" timer");*/
	} else if (btBuffer == "MODETOGGLE") {
		modeToggle = !modeToggle;
		/* [D]
		String v = modeToggle ? "on" : "off";
		Serial.print("[D] Toggle Temp/ Hum is ");
		Serial.print(v);
		Serial.println(" now");*/
	} else if (btBuffer.startsWith("RTC")) {
		long y = getValue(btBuffer, ',', 1).toInt();
		long m = getValue(btBuffer, ',', 2).toInt();
		long d = getValue(btBuffer, ',', 3).toInt();
		long h = getValue(btBuffer, ',', 4).toInt();
		long mm = getValue(btBuffer, ',', 5).toInt();
		long s = getValue(btBuffer, ',', 6).toInt();
		rtc.adjust(DateTime(y, m, d, h, mm, s));
		//Serial.println("[D] DateTime set");
	} else if (btBuffer.startsWith("SCOREBOARD")) {
		scoreLeft = getValue(btBuffer, ',', 1).toInt();
		scoreRight = getValue(btBuffer, ',', 2).toInt();
		mode = 3;
		// [D] Serial.println("[D] Switched mode to 3");
	} else if (btBuffer.startsWith("COLOR")) {
		char target = getValue(btBuffer, ',', 1)[0];
		int r = getValue(btBuffer, ',', 2).toInt();
		int g = getValue(btBuffer, ',', 3).toInt();
		int b = getValue(btBuffer, ',', 4).toInt();
		switch (target) {
			case 'B':
				colorBackground.setRGB(r, g, b);
				break;
			case 'P':
				colorPrimary.setRGB(r, g, b);
				break;
			case 'S':
				colorSecondary.setRGB(r, g, b);
				break;
			/*default:
				Serial.println("[E] Unknown target");
				return;*/
		}
		// [D] Serial.println("[D] Color set");
	} else if (btBuffer.startsWith("BRIGHTNESS")) {
		char target = getValue(btBuffer, ',', 1)[0];
		int value = getValue(btBuffer, ',', 2).toInt();
		switch (target) {
			case 'B':
				brightnessBackground = (float)value;
				break;
			case '0':
				brightnessOffset0 = (float)value;
				break;
			case '1':
				brightnessOffset1 = (float)value;
				break;
			case '2':
				brightnessOffset2 = (float)value;
				break;
			case 'L':
				backgroundLighting = (value == 1);
				break;
			/*default:
				Serial.println("[E] Unknown target");
				return;*/
		}
		// [D] Serial.println("[D] Brightness set");
	}/* else {
		Serial.println("[E] Unknown command");
	}*/
	btBuffer = "";
	refreshDisplay();
}

void refreshDisplay() {
	switch (mode) {
		case 0:
			displayAnalogClock();
			break;
		case 1:
			displayAnalogClock();
			displayTemperature();
			break;
		case 2:
			displayAnalogClock();
			displayHumidity();
			break;
		case 3:
			displayScoreboard();
			break;
		///case 4: Time counter has it's own timer
	}
}

void refreshTimer() {
	if (modeToggle) {
		refresh++;
		if(refresh >= toggleInterval) {
			if (mode == 1) {
				mode = 2;
				refreshDisplay();
			} else if (mode == 2) {
				mode = 1;
				refreshDisplay();
			}
			refresh = 0;
		}
	}
	if(mode == 4 && timerRunning && timerValue < 6000) {
		timerValue++;
		int m = timerValue / 60;
		int s = timerValue % 60;
		uint8_t data[4];
		if (m < 10) {
			data[0] = numbersWithoutDot[0];
			data[1] = numbersWithDot[m];
		} else {
			data[0] = numbersWithoutDot[(int)(m / 10)];
			data[1] = numbersWithDot[m % 10];
		}
		if (s < 10) {
			data[2] = numbersWithoutDot[0];
			data[3] = numbersWithoutDot[s];
		} else {
			data[2] = numbersWithoutDot[(int)(s / 10)];
			data[3] = numbersWithoutDot[s % 10];
		}
		display.clear();
		display.setSegments(data);
	}
}

void displayAnalogClock() {
	display.clear();
	DateTime now = rtc.now();
	int hours = now.hour();
	int mins = now.minute();
	/// Convert 24h to 12h to led id
	hours %= 12;
	hours += 12; // use 2nd circle
	int offset = mins % 5; //offset value
	mins /= 5; //led id
	/// Reset leds
	fill_solid(&(LEDs[0]), NUM_LEDS, CRGB::Black);
	if (brightnessBackground != 0) {
		if (backgroundLighting) {
			fill_solid(&(LEDs[0]), NUM_LEDS, colorBackground);
			for(int i = 0; i < NUM_LEDS; i++) {
				LEDs[i].nscale8_video(brightnessBackground);
			}
		} else {
			LEDs[0] = colorBackground;
			LEDs[3] = colorBackground;
			LEDs[6] = colorBackground;
			LEDs[9] = colorBackground;
			LEDs[0].nscale8_video(brightnessBackground);
			LEDs[3].nscale8_video(brightnessBackground);
			LEDs[6].nscale8_video(brightnessBackground);
			LEDs[9].nscale8_video(brightnessBackground);
		}
	}
	/// Set leds - hours
	LEDs[hours] = colorPrimary;
	LEDs[hours].nscale8_video(brightnessOffset0);
	/// Set leds - mins
	if(offset == 0) {
		/// If minutes are 5; 10; 15; ... 55: only this led on
		LEDs[mins] = colorSecondary;
		LEDs[mins].nscale8_video(brightnessOffset0);
	} else {
		/// If minutes are 1; 2; 3; 4; 6; ... 59: 2 leds on
		switch(offset) {
			case 1:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorSecondary);
					LEDs[mins].nscale8_video(brightnessOffset0);
					LEDs[mins + 1].nscale8_video(brightnessOffset1);
				} else {
					LEDs[11] = colorSecondary;
					LEDs[0] = colorSecondary;
					LEDs[11].nscale8_video(brightnessOffset0);
					LEDs[0].nscale8_video(brightnessOffset1);
				}
				break;
			case 2:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorSecondary);
					LEDs[mins].nscale8_video(brightnessOffset0);
					LEDs[mins + 1].nscale8_video(brightnessOffset2);
				}
				else {
					LEDs[11] = colorSecondary;
					LEDs[0] = colorSecondary;
					LEDs[11].nscale8_video(brightnessOffset0);
					LEDs[0].nscale8_video(brightnessOffset2);
				}
				break;
			case 3:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorSecondary);
					LEDs[mins + 1].nscale8_video(brightnessOffset0);
					LEDs[mins].nscale8_video(brightnessOffset2);
				} else {
					LEDs[11] = colorSecondary;
					LEDs[0] = colorSecondary;
					LEDs[0].nscale8_video(brightnessOffset0);
					LEDs[11].nscale8_video(brightnessOffset2);
				}
				break;
			case 4:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorSecondary);
					LEDs[mins + 1].nscale8_video(brightnessOffset0);
					LEDs[mins].nscale8_video(brightnessOffset1);
				} else {
					LEDs[11] = colorSecondary;
					LEDs[0] = colorSecondary;
					LEDs[0].nscale8_video(brightnessOffset0);
					LEDs[11].nscale8_video(brightnessOffset1);
				}
				break;
		}
	}
	FastLED.show();
}

void displayTemperature() {
	float temperature = dht.readTemperature(temperatureMode == 'F' ? true : false);
	if (!isnan(temperature)) {
		mux(temperature, false);
		uint8_t data[] = {muxReturn[0], muxReturn[1], extras[0], extras[(temperatureMode == 'F' ? 3 : 1)]};
		display.clear();
		display.setSegments(data);
	}
}

void displayHumidity() {
	float humidity = dht.readHumidity();
	if (!isnan(humidity)) {
		mux(humidity, false);
		uint8_t data[] = {muxReturn[0], muxReturn[1], extras[0], extras[2]};
		display.clear();
		display.setSegments(data);
	}
}

void displayScoreboard() {
	fill_solid(&(LEDs[0]), NUM_LEDS, CRGB::Black);
	fill_solid(&(LEDs[1]), 5, colorPrimary);
	fill_solid(&(LEDs[7]), 5, colorSecondary);
	for(int i = 0; i < NUM_LEDS; i++) {
		LEDs[i].nscale8_video(brightnessOffset0);
	}
	FastLED.show();
	uint8_t data[4];
	if (scoreLeft < 10) {
		data[0] = numbersWithoutDot[0];
		data[1] = numbersWithDot[scoreLeft];
	} else {
		data[0] = numbersWithoutDot[(int)(scoreLeft / 10)];
		data[1] = numbersWithDot[scoreLeft % 10];
	}
	if (scoreRight < 10) {
		data[2] = numbersWithoutDot[0];
		data[3] = numbersWithoutDot[scoreRight];
	} else {
		data[2] = numbersWithoutDot[(int)(scoreRight / 10)];
		data[3] = numbersWithoutDot[scoreRight % 10];
	}
	display.clear();
	display.setSegments(data);
}
