# 24-Segment-Analog-Clock
___7-Segment-Digital-Clock\_Analog-Patch___

__Note:__ Still work in progress, v0: display time, scoreboard (only red/ blue sides). TODO: implement 7s display

This is the source and schema for a homemade analog clock I've made. The code is based on [this](https://github.com/leonvandenbeukel/3D-7-Segment-Digital-Clock/blob/master/3D-7-Segment-Digital-Clock.ino).

In daylight it's not very bright.
To control the clock you can use a bluetooth terminal of your choice (I'm working on an app).

## Hardware

![schema](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/blob/master/Schema.png)

| Hardware                              		      | Additional hardware for wireless clock | Software                                                                |
| -------------                          	        | -------------                          | -------------                                                           |
| Arduino Nano                           			    | Li-Ion 18650 Battery                   | Arduino IDE                                                             |
| LED strip WS2812B 1 meter 30 RGB LED's		 	    | 5V 1A Step Up Power Module             | Library: [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) |
| Real time clock: DS3231 and battery (and a battery for it if not included)          		| A battery holder wouldn't be bad...    | Library: [DHT](https://github.com/adafruit/DHT-sensor-library)          |
| Bluetooth module: HC-05                 			  |                                        | Library: [FastLED](https://github.com/FastLED/FastLED)                  |
| Temperature and Humidity Sensor: DHT11 or DHT22 |                                        | Library: [RTCLib](https://github.com/adafruit/RTClib)                   |
| LED 4 Digits 7 Segment Display: TM1637          |                                        | Library: [Timer](https://github.com/JChristensen/Timer)                 |
| 5V / 2A  Power Supply								            |                                        | Library: [TM1637](https://github.com/avishorp/TM1637)                   |
| PCB                                    			    |                                        |                                                                         |
| Wires, Glue and a lot of patience :)       	    |                                        |                                                                         |

## Commands

To send a command to the clock, you need to wrap it into this characters: ||

Synthax:

`| Command,arg0,argN|` 

| Command     | Arguments        | Description |
| ---         | ---              | ---         |
| PING        | none             | Returns `\|PONG,(version)\|`. Its for the app (to check if right device, for different versions, ...) |
| RESET       | none             | Restarts the Arduino (like pushing the reset button or put the reset pin high) |
| CLOCK       | none             | Set mode to 0 (Display time) |
| TEMPERATURE | none             | Set mode to 1 (Display time and temperature)* |
| HUMIDITY    | none             | Set mode to 2 (Display time and humidity)* |
| SCOREBOARD  | L, R             | Set mode to 3 and Set score of L and R (both type integer, 0 ≤ n ≤ 99) |
| TIMER       | none             | Set mode to 4 and start/ stop timer (Returns `\|TIMER,(timerRunning as int)\|`) |
| MODETOGGLE  | none             | Define, if the clock toggles every 5 seconds between mode 1 and 2 (Returns `\|MODETOGGLE,(modeToggle as int)\|`) |
| RTC         | Y, M, D, h, m, s | Set time. No leading zeros required (Year, Month, Day, Hour, Minute, Second) |
| COLOR       | target, r, g, b  | Set rgb color of target (B, H, M, L, R)** |
| BRIGHTNESS  | target, value    | Set brightness of target(B, H, 0, 1, 2, L)***  |

\*If modeToggle is true, the clock toggles between temperature and humidity.<br>
\*\*List of color targets:
 - B: Background
 - H: Hour dot (pointer)
 - M: Minute dot(s) (pointer)
 - L: Score left
 - R: Score right

\*\*\*List of brightness targets:
 - B: Background
 - H: Hour dot (pointer)
 - 0: Minutes dot (offset 0)
 - 1: Minutes dot (offset 1)
 - 2: Minutes dot (offset 2)
 - L: Set background lighting:
   - 0: Off
   - 1: Quarters
   - 2: All

## Pictures of building process

[Clock displaying scoreboard](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/clock_dev_scoreboard.png)<br>
[Clock displaying 18:55](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/clock_dev_time.png)
