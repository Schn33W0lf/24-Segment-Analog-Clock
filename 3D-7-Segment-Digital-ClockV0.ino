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
 * TODO: Implement 7s display in some functions
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
RTC_DS3231 rtc;

String btBuffer = "";
bool btReading = false;
#define btStartEnd '|'
#define btSeperate ','

// Setup for Timer
Timer t1;
Timer t2;
Timer t3;

volatile bool pingRunning = false;
volatile int pingValue = 0;
volatile bool timerRunning = false;
volatile int timerValue = 0;
volatile int scoreLeft;
volatile int scoreRight;

// Setup for Clock
volatile short mode = 0;
volatile bool modeToggle = false;
volatile int toggleInterval = 5000;

char temperatureMode = 'C';

CRGB colorBackground = CRGB::DarkGoldenrod;
CRGB colorPointerMins = CRGB::Red;
CRGB colorPointerHours = CRGB::Blue;
CRGB colorScoreLeft = CRGB::Blue;
CRGB colorScoreRight = CRGB::Red;
float brightnessBackground = 2.56f; /// 1%
float brightnessPointerHours = 255.0f; /// 100%
float brightnessPointerMins0 = 255.0f; /// 100%
float brightnessPointerMins1 = 51.2f; /// 10%
float brightnessPointerMins2 = 102.4f; /// 20%
short backgroundLighting = 1; /// 0 = off, 1 = quarters, 2 = all

// Setup
void setup () {

	// Initialize LED strip
	FastLED.delay(3000);
	FastLED.addLeds<settings_led_chip, DATA_PIN, settings_led_type>(LEDs, NUM_LEDS);

	// Initialize Serial Monitor, wait for Serial Monitor
	Serial.begin(9600);
	while (!Serial) { /* Wait until serial is ready */ }
	Serial.println("[D] Serial started at 9600");

	// Initialize Serial Bluetooth
	BTserial.begin(9600);
	Serial.println("[D] BTserial started at 9600");

	// Initialize DHT
	dht.begin();

	// Wait for RTC
	if (!rtc.begin()) {
		Serial.println("[E] Couldn't find RTC.");
		while (1);
	}

	// Set RTC time if lost (set time to the time this sketch was compiled)
	if (rtc.lostPower()) {
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		Serial.println("[I] RTC lost power, Reset the time.");
	}
	
	// Initialitze Timers
	t1.every(1000 * 29, refreshDisplay);
	t2.every(1000, refreshTimer);
	t3.every(toggleInterval, toggleMode);
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
	t3.update();

}

// Utils
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
	btReading = false;
	if (btBuffer == "RESET") {
		Serial.println("[I] Restarting Arduino...");
		delay(1000);
		resetFunc();
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
		BTserial.print("|TIMER,");
		BTserial.print(timerRunning);
		BTserial.print("|");
	} else if (btBuffer == "MODETOGGLE") {
		modeToggle = !modeToggle;
		/* [D]
		String v = modeToggle ? "on" : "off";
		Serial.print("[D] Toggle Temp/ Hum is ");
		Serial.print(v);
		Serial.println(" now");*/
		BTserial.print("|MODETOGGLE,");
		BTserial.print(modeToggle);
		BTserial.print("|");
	} else if (btBuffer.startsWith("RTC")) {
		long y = getValue(btBuffer, ',', 1).toInt();
		long m = getValue(btBuffer, ',', 2).toInt();
		long d = getValue(btBuffer, ',', 3).toInt();
		long h = getValue(btBuffer, ',', 4).toInt();
		long mm = getValue(btBuffer, ',', 5).toInt();
		long s = getValue(btBuffer, ',', 6).toInt();
		rtc.adjust(DateTime(y, m, d, h, mm, s));
		Serial.println("[D] DateTime set");
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
			case 'H':
				colorPointerHours.setRGB(r, g, b);
				break;
			case 'M':
				colorPointerMins.setRGB(r, g, b);
				break;
			case 'L':
				colorScoreLeft.setRGB(r, g, b);
				break;
			case 'R':
				colorScoreRight.setRGB(r, g, b);
				break;
			default:
				Serial.println("[E] Unknown target");
				return;
		}
		// [D] Serial.println("[D] Color set");
	} else if (btBuffer.startsWith("BRIGHTNESS")) {
		char target = getValue(btBuffer, ',', 1)[0];
		int value = getValue(btBuffer, ',', 2).toInt();
		switch (target) {
			case 'B':
				brightnessBackground = (float)value;
				break;
			case 'H':
				brightnessPointerHours = (float)value;
				break;
			case '0':
				brightnessPointerMins0 = (float)value;
				break;
			case '1':
				brightnessPointerMins1 = (float)value;
				break;
			case '2':
				brightnessPointerMins2 = (float)value;
				break;
			case 'L':
				backgroundLighting = value;
				break;
			default:
				Serial.println("[E] Unknown target");
				return;
		}
		// [D] Serial.println("[D] Brightness set");
	} else {
		Serial.println("[E] Unknown command");
	}
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

void toggleMode() {
	if (mode == 1) {
		mode = 2;
		refreshDisplay();
	} else if (mode == 2) {
		mode = 1;
		refreshDisplay();
	}
}

void refreshTimer() {// TODO implement 7s display
	if(mode == 4 && timerRunning && timerValue < 6000) {
		timerValue++;
		int m = timerValue / 60;
		int s = timerValue % 60;
		// DEBUG
		Serial.print("[D] Timer: ");
		Serial.print(m);
		Serial.print(":");
		Serial.println(s);
		// TODO: display timer on 7s led module
	}
	if (pingRunning) {
		pingValue++;
	}
}

void displayAnalogClock() {
	DateTime now = rtc.now();
	int hours = now.hour();
	int mins = now.minute();
	// [D]
	Serial.print("[D] Time: ");
	Serial.print(hours);
	Serial.print(":");
	Serial.println(mins);
	/// Convert 24h to 12h to led id
	hours %= 12;
	hours += 12; // use 2nd circle
	int offset = mins % 5; //offset value
	mins /= 5; //led id
	/// Reset leds
	fill_solid(&(LEDs[0]), NUM_LEDS, CRGB::Black);
	switch(backgroundLighting) {
		case 1:
			LEDs[0] = colorBackground;
			LEDs[3] = colorBackground;
			LEDs[6] = colorBackground;
			LEDs[9] = colorBackground;
			LEDs[0].nscale8_video(brightnessBackground);
			LEDs[3].nscale8_video(brightnessBackground);
			LEDs[6].nscale8_video(brightnessBackground);
			LEDs[9].nscale8_video(brightnessBackground);
			break;
		case 2:
			fill_solid(&(LEDs[0]), NUM_LEDS, colorBackground);
			for(int i = 0; i < NUM_LEDS; i++) {
				LEDs[i].nscale8_video(brightnessBackground);
			}
			break;
		}
	/// Set leds - hours
	LEDs[hours] = colorPointerHours;
	LEDs[hours].nscale8_video(brightnessPointerHours);
	/// Set leds - mins
	if(offset == 0) {
		/// If minutes are 5; 10; 15; ... 55: only this led on
		LEDs[mins] = colorPointerMins;
		LEDs[mins].nscale8_video(brightnessPointerMins0);
	} else {
		/// If minutes are 1; 2; 3; 4; 6; ... 59: 2 leds on
		switch(offset) {
			case 1:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorPointerMins);
					LEDs[mins].nscale8_video(brightnessPointerMins0);
					LEDs[mins + 1].nscale8_video(brightnessPointerMins1);
				} else {
					LEDs[11] = colorPointerMins;
					LEDs[0] = colorPointerMins;
					LEDs[11].nscale8_video(brightnessPointerMins0);
					LEDs[0].nscale8_video(brightnessPointerMins1);
				}
				break;
			case 2:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorPointerMins);
					LEDs[mins].nscale8_video(brightnessPointerMins0);
					LEDs[mins + 1].nscale8_video(brightnessPointerMins2);
				}
				else {
					LEDs[11] = colorPointerMins;
					LEDs[0] = colorPointerMins;
					LEDs[11].nscale8_video(brightnessPointerMins0);
					LEDs[0].nscale8_video(brightnessPointerMins2);
				}
				break;
			case 3:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorPointerMins);
					LEDs[mins + 1].nscale8_video(brightnessPointerMins0);
					LEDs[mins].nscale8_video(brightnessPointerMins2);
				} else {
					LEDs[11] = colorPointerMins;
					LEDs[0] = colorPointerMins;
					LEDs[0].nscale8_video(brightnessPointerMins0);
					LEDs[11].nscale8_video(brightnessPointerMins2);
				}
				break;
			case 4:
				if(mins < 11) {
					fill_solid(&(LEDs[mins]), 2, colorPointerMins);
					LEDs[mins + 1].nscale8_video(brightnessPointerMins0);
					LEDs[mins].nscale8_video(brightnessPointerMins1);
				} else {
					LEDs[11] = colorPointerMins;
					LEDs[0] = colorPointerMins;
					LEDs[0].nscale8_video(brightnessPointerMins0);
					LEDs[11].nscale8_video(brightnessPointerMins1);
				}
				break;
		}
	}
	FastLED.show();
}

void displayTemperature() {// TODO implement 7s display
	float temperature = dht.readTemperature(temperatureMode == 'F' ? true : false);
	if (isnan(temperature)) {
		Serial.println("[E] Failed to read from DHT sensor");
	} else {
		// DEBUG
		Serial.print("[D] Temperature: ");
		Serial.print(temperature);
		Serial.print("°");
		Serial.println(temperatureMode);
		// TODO: display temp on 7s led module
	}
}

void displayHumidity() {// TODO implement 7s display
	float humidity = dht.readHumidity();
	if (isnan(humidity)) {
		Serial.println("Failed to read from DHT sensor!");
	} else {
		// DEBUG
		Serial.print("[D] Humidity: ");
		Serial.print(humidity);
		Serial.println("%");
		// TODO: display hum on 7s led module
	}
}

void displayScoreboard() {// TODO implement 7s display
	fill_solid(&(LEDs[0]), NUM_LEDS, CRGB::Black);
	fill_solid(&(LEDs[1]), 5, colorScoreRight);
	fill_solid(&(LEDs[7]), 5, colorScoreLeft);
	FastLED.show();
	// DEBUG
	Serial.print("[D] Score Left: ");
	Serial.println(scoreLeft);
	Serial.print("[D] Score Right: ");
	Serial.println(scoreRight);
	// TODO: display score on 7s led module
}
