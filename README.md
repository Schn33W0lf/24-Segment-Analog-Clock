# 24-Segment-Analog-Clock

Inspiration: [7-Segment-Digital-Clock](https://github.com/leonvandenbeukel/7-Segment-Digital-Clock)<br>
Original: [3D-7-Segment-Digital-Clock](https://github.com/leonvandenbeukel/3D-7-Segment-Digital-Clock)

This is the source and schema for a homemade analog clock I've made. Here's a link to the [YouTube video](https://www.youtube.com/watch?v=1aNHF5tcTpw) of the digital clock by Leon van den Beukel, __not me__

In daylight it's not very bright.
You can download the Android app from the Play Story here: https://play.google.com/store/apps/details?id=nl.leonvandenbeukel.BTDigitalClockApp (Also __not my app!__)


![alt text](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/blob/master/Schema.png)

| Hardware                              		      | Additional hardware for wireless clock | Software                                                                |
| -------------                          	        | -------------                          | -------------                                                           |
| Arduino Nano                           			    | Li-Ion 18650 Battery                   | Arduino IDE                                                             |
| LED strip WS2812B 1 meter 30 RGB LED's		 	    | 5V 1A Step Up Power Module             | Library: [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) |
| Real time clock: DS3231 and battery          		| A battery holder wouldn't be bad...    | Library: [DHT](https://github.com/adafruit/DHT-sensor-library)          |
| Bluetooth module: HC-05                 			  |                                        | Library: [FastLED](https://github.com/FastLED/FastLED)                  |
| Temperature and Humidity Sensor: DHT11 or DHT22 |                                        | Library: [RTCLib](https://github.com/adafruit/RTClib)                   |
| LED 4 Digits 7 Segment Display: TM1637          |                                        | Library: [Timer](https://github.com/JChristensen/Timer)                 |
| 5V / 2A  Power Supply								            |                                        | ? Library: [TM1637](#)                                                  |
| PCB                                    			    |                                        |                                                                         |
| Wires, Glue and a lot of patience :)       	    |                                        |                                                                         |

