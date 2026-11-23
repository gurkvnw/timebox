# Goals, plan, and risks

## Topic

TimeBox: a timed lockbox controlled by an Arduino Uno.

## Goals

- Let the user choose a waiting period with physical buttons.
- Show the selected and remaining time on a 16×2 LCD.
- Move a servo to lock and unlock positions.
- Give an audible signal at the end.
- Build a box and explain the program and connections.

## Work completed

- Wokwi concept with an LCD, buttons, a servo, and a buzzer.
- Physical bench assembly with a yellow START button and an analog keypad.
- Removal of the lid sensor from the physical design.
- A single-file Arduino sketch with keypad calibration.
- Student-reported successful upload, LCD operation, countdown, button input, and servo movement.
- Buzzer operation confirmed by the student in the latest bench update.

## Next work

2. Record exact part models and confirm the servo supply with the instructor.
2. Fit the servo and a latch to a box; adjust the two angles.
3. Test repeated cycles, movement under load, and longer presets.
4. Add a short demo of the completed enclosure and update the report with the remaining test results.

## What is needed

The existing bench components, Arduino IDE, a USB data cable, a box, suitable mounting materials, and help confirming the servo supply.

## Risks and limits

Loose jumper connections may interrupt signals. A weak supply may affect the motor or reset the board. An incorrect angle can prevent the latch from moving freely. Reset and power loss erase the countdown. The system has no lid-position feedback, and the intended smoking-reduction benefit has not been measured.
