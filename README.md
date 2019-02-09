# 24-Segment-Analog-Clock

This is the source and schema for a homemade analog clock I've made. Here's a link to the [YouTube video](https://www.youtube.com/watch?v=1aNHF5tcTpw) of the digital clock by Leon van den Beukel, __not me__

In daylight it's not very bright.
You can download the Android app from the Play Story here: https://play.google.com/store/apps/details?id=nl.leonvandenbeukel.BTDigitalClockApp (Also __not my app!__)


![alt text](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/blob/master/Schema.png)

| Hardware                              			      |
| -------------                          			      |
| Arduino Nano                           			      |
| LED strip WS2812B 1 meter 30 RGB LED's		 	      |
| Real time clock: DS3231 and battery          		  |
| Bluetooth module: HC-05                 			    |
| Temperature and Humidity Sensor: DHT11 or DHT22  	|
| LED 4 Digits 7 Segment Display: TM1637            |
| 5V / 2A  Power Supply								              |
| PCB                                    			      |
| Wires, Glue and a lot of patience :)    			    |

| Additional Hardware                               |
| -------------                          			      |
| Li-Ion 18650 Battery                         			|
| 5V 1A Step Up Power Module                        |
| A battery holder wouldn't be bad...               |

| Software                                          |
| -------------                          			      |
| Arduino IDE                                       |
| Library: [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) |
| Library: [DHT](https://github.com/adafruit/DHT-sensor-library)          |
| Library: [FastLED](https://github.com/FastLED/FastLED)                  |
| Library: [RTCLib](https://github.com/adafruit/RTClib)                   |
| Library: [Timer](https://github.com/JChristensen/Timer)                 |
| ? Library: [TM1637](#)

