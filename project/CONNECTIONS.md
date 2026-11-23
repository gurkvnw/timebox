# Connections

## Signals

| Module connection | Arduino Uno connection |
| --- | --- |
| Yellow START button S | D4 |
| Analog keypad S | A0 |
| Buzzer S | D8 |
| Servo orange signal | D9 |
| LCD SDA | A4 |
| LCD SCL | A5 |

Follow printed module pin labels. Jumper extensions may change wire colours, so trace each wire from end to end.

## Reported classroom power wiring

| Connection | Reported destination |
| --- | --- |
| LCD VCC, red | Uno 5V |
| LCD GND, black | Common ground rail |
| START, keypad, and buzzer G | Common ground rail |
| START, keypad, and buzzer V | Positive rail connected to Uno 3.3V |
| Servo brown | Common ground rail |
| Servo red | Shared positive rail, as reported by the student |
| Ground rail jumper | Uno GND |

The instructor requested the 3.3V rail for parts other than the display. This record does not establish that the servo is rated for this voltage. Record the actual servo model and confirm the supply current before using a loaded latch. Do not use a breadboard's coloured marking alone to identify its voltage; the supply jumper determines the voltage. A separate servo supply, if used later, must share ground with the Uno and must not be joined to the 3.3V positive rail.

USB powers the board during the bench demo. Disconnect power before changing connections.
