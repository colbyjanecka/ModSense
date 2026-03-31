# ModSense: Motion-Based CV for Eurorack

**Have you ever wanted to control your Eurorack patch with just your presence, or maybe add an invisible whammy bar to your ModCaster?**  Well, this 6 HP module allows you to use your bodily movement within your patch, enabling quick and effective live-performance control options.

## Schematic
![schematic](Plot/MotionMod.png)



## Features

- 4 x Programmable CV Out (0-12v)
- Generates Trigger Output when object presence is detected/changed
- Ultrasonic ranging transducer provides multi-functional outputs:
  - Detection area: 2cm~500cm;  precision: down to 0.3 cm
  - Example: distance and speed of detected object

## Parts

- Raspberry Pi Pico (RP2040)
- MCP4728 12 Bit I2C DAC
- TL074 Op-Amp
- HC-SR04 Ultrasonic Module
- Infrared Photoelectric Module (3-pin)
