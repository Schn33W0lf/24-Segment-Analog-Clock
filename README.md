# 24-Segment-Analog-Clock
___7-Segment-Digital-Clock\_Analog-Patch___

This is the source and schema for a homemade analog clock I've made. The code is based on [this](https://github.com/leonvandenbeukel/3D-7-Segment-Digital-Clock/blob/master/3D-7-Segment-Digital-Clock.ino).

In daylight it's not very bright.
To control the clock you can use a bluetooth terminal of your choice or my [app](app-release/analog-clock-v1-build2-release.apk).<br>

## App

Feel free to modify my app like you want to. [Source code](app-data/analog-clock-v1-build2.zip)

If the app asks you to enable bluetooth, the devices wont display after you allowed it, first you need to reload the current site.<br>
Press on the Devices button to reload the site (works on other sites too).

### App permissions:

The App needs following permissions:

 - &lt;uses-permission android:name="android.permission.BLUETOOTH" /&gt;
 - &lt;uses-permission android:name="android.permission.BLUETOOTH_ADMIN" /&gt;
 - &lt;uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" /&gt;
 - &lt;uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" /&gt;

Bluetooth should be logical. The app will ask befor enabling Bluetooth or access the storage. The storage is used to save some values (see site advanced). You can save and load color values.<br>
The values will be stored in most cases to `/storage/emulated/0/AnalogClockV1/config.json`, just open a file explorer and navigate to internal storage. anywhere there you should find the folder AnalogClockV1.

## Hardware

___The resistor R1.1 need to be a pull UP resistor!___ <br>
I'll fix the plans (wiring, pcb, perfboard (AnalogClockV1.fzz)) later (with the ext. EEPROM update I'm working on)<br>
_(Btw, the cirquit worked with even no resistors...)_

[![schema](wiring/AnalogClockV1_wiring_diagram.png)](wiring)

See wiring folder for sketches

| Hardware                              		      | Additional hardware for wireless clock | Software                                                                |
| -------------                          	      | -------------                          | -------------                                                           |
| Arduino Nano (ATmega328!)              			    | Li-Ion 18650 Battery                   | Arduino IDE                                                             |
| LED strip WS2812B 1m 30 RGB LEDs (24 needed)  | 5V 1A Step Up Power Module             | Library: [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) |
| Real time clock: DS3231                       | A battery holder wouldn't be bad...    | Library: [DHT](https://github.com/adafruit/DHT-sensor-library)          |
| (and a battery if not included)               | 2x Generic female header - 15 pins     | Library: [FastLED](https://github.com/FastLED/FastLED)                  |
| Bluetooth module: HC-05                 			   | 4x Generic female header - 4 pins      | Library: [RTCLib](https://github.com/adafruit/RTClib)                   |
| Temperature & Humidity Sensor: DHT11 or DHT22 | 1x Generic female header - 3 pins    | Library: [Timer](https://github.com/JChristensen/Timer)                 |
| LED 4 Digits 7 Segment Display: TM1637        |                                        | Library: [TM1637](https://github.com/avishorp/TM1637)                   |
| 5V / 2A  Power Supply                  			    |                                        |                                                                         |
| PCB, Breadboard, ...                          |                                        |                                                                         |
| Wires, Glue and a lot of patience :)       	  |                                        |                                                                         |

## Commands

To send a command to the clock, you need to wrap it into this characters: ||

Synthax:

`| Command,arg0,argN|` 

| Command     | Arguments        | Description |
| ---         | ---              | ---         |
| PING        | none             | Returns `\|STATUS,(some parameters\*)\|`. Its for the app (to check if right device, for different versions, ...) |
| RESET       | none             | Restarts the Arduino (like pushing the reset button or put the reset pin high) |
| CLOCK       | none             | Set mode to 0 (Display time) |
| TEMPERATURE | none             | Set mode to 1 (Display time and temperature)\*\* |
| HUMIDITY    | none             | Set mode to 2 (Display time and humidity)\*\* |
| SCOREBOARD  | L, R             | Set mode to 3 and Set score of L and R (both type integer, 0 ≤ n ≤ 99) |
| TIMER       | none             | Set mode to 4 and start/ stop timer (Returns `\|TIMER,(timerRunning as int)\|`) |
| MODETOGGLE  | none             | Define, if the clock toggles every 5 seconds between mode 1 and 2 (Returns `\|MODETOGGLE,(modeToggle as int)\|`) |
| RTC         | Y, M, D, h, m, s | Set time. No leading zeros required (Year, Month, Day, Hour, Minute, Second) |
| COLOR       | target, r, g, b  | Set rgb color of target (B, P, S)\*\*\* |
| BRIGHTNESS  | target, value    | Set brightness of target(B, H, 0, 1, 2, L)*\*\*\*  |

\*For parameters see [AnalogClockV4 » processBtBuffer() » if PING \[#L218\]](AnalogClockV4.ino#L218)
 
\*\*If modeToggle is true, the clock toggles between temperature and humidity.<br>
\*\*\*List of color targets:
 - B: Background
 - P: Hour dot (pointer) &  Score left
 - S: Minute dot(s) (pointer) & Score right

\*\*\*\*List of brightness targets:
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

[Clock displaying humidity](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/dev/AnalogClock_dev_humidity.PNG)<br>
[Clock displaying scoreboard](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/dev/AnalogClock_dev_scoreboard.PNG)<br>
[Clock displaying temperature](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/dev/AnalogClock_dev_temperature.PNG)<br>
[Clock displaying 18:55](https://github.com/Schn33W0lf/24-Segment-Analog-Clock/raw/master/dev/AnalogClock_dev_time.PNG)

