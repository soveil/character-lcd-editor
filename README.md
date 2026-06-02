# Character LCD Editor

<img width="450" height="200" alt="16x2 LCD with 'Character LCD Display' on it" src="https://github.com/user-attachments/assets/ceefa804-5dec-4cda-91c8-e3641f8a6b93" />

## Overview

This is a text editor with a 16x2 character LCD display.
The editor code runs on an Arduino, and the text is sent from a computer running the included python script, but any program/device which sends serial data could be used.
The text is stored in a 1KB [split buffer](https://www.cs.unm.edu/~crowley/papers/sds/node9.html#SECTION00052000000000000000), and the editor supports ascii characters, backspace, and arrow keys.


Since the display is small, the editor prioritizes keeping as many characters relevant to what you're editing on the screen as possible.
Due to this, only one line is shown on the display at once, and it is spread across both rows.
The cursor then attempts to show an equal number of characters before and after itself.

## Usage

1. Set up the [Hardware](#hardware) (Arduino and character LCD)
1. Install the required [Dependencies](#dependencies) (LiquidCrystal library for Arduino, uv/pip install for python)
1. Compile and upload [lcd-editor](lcd-editor) either using the Arduino IDE or CLI.
1. Run the [python script](stream.py): `uv run stream.py` or `pip install -r requirements.txt; python stream.py`

If you have the Arduino CLI and uv installed, and the hardware set up,
configure the following with your Arduino board and port, and these commands will start running the editor:

```bash
env ARDUINO_PORT=/dev/ttyACM0 \
    ARDUINO_BOARD=arduino:avr:uno \
bash -c '
git clone https://github.com/soveil/character-lcd-editor.git
cd character-lcd-editor
arduino-cli lib install LiquidCrystal
arduino-cli compile -b $ARDUINO_BOARD lcd-editor
arduino-cli upload -p $ARDUINO_PORT -b $ARDUINO_BOARD lcd-editor
uv run stream.py $ARDUINO_PORT
'
```

## Hardware

The editor requires an Arduino and a LCD display compatible with the Hitachi HD44780 driver.
The code is currently set up for 16x2 displays, but that can be changed by editing two constants at the top of the file.

### Schematic

<img width="450" height="400" alt="Arduino LCD circuit schematic" src="https://github.com/user-attachments/assets/60d955d2-6b7d-4de5-9c1a-4b0021597b7a" />

This schematic comes from the [Arduino LCD Tutorial](https://docs.arduino.cc/learn/electronics/lcd-displays/), which further explains how the display was set up.

## Dependencies

### For the editor

- Arduino [LiquidCrystal](https://docs.arduino.cc/libraries/liquidcrystal/) library
    - To install: `arduino-cli lib install LiquidCrystal`

### For the communication script

- [PySerial](https://www.pyserial.com/)
  - To install: `uv sync` or `pip install -r requirements.txt`
- termios (POSIX only)
