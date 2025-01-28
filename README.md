# Walkie-Tappie

Walkie Tappie is an original project by Alex Chang (2020) that allows you to create a walkie talkie that works with morse code using ESP32 LoRa boards and the RH (Radio Head) interface.

Limitations
===========

According to the documentation of the RH_RF95.h interface (https://github.com/PaulStoffregen/RadioHead/blob/master/RH_RF95.h) the maximum message length is 251 uint8_t bytes, but the program uses int 16 to store the arrays (this way you can get more than 255 ms of time), which means that you get 124 numbers to send the messages. Of the 124, half are used to communicate the idle time and the other half is the time to hold down the Morse key, i.e. 2 numbers are needed per beep, therefore a maximum of 62 beeps can be sent per message (The standard word in Morse is "PARIS" which would be about 28 numbers, giving us about two words). In practice, it is about 15s of transmission and less than 10 letters or characters.
Transmission is carried out after 5s of waiting without manipulation, emitting a beep to indicate sending.

Manipulation is monitored by the speaker

The development board screen is not used, nor is any LED type visual indicator used.

The program
===========

The original project uses the Adafruit-Feather-32u4-RFM-LoRa-PCB development board that has the following pin assignment and the RFM69 chip for LoRa

They are used in the project: VBAT/BAT (output + 3.3V), GND (power ground) and pins 11 of the keyer (D11, GPIO PB7, PCINT7, OC0A/OC1C) and 12 of the speaker (D12, GPIO PD6, ADC9, A11, !OC4D)

For its part, the development board to be used is the Lilygo T3_V.1.6.1 868MHz, which mounts the SX1276 chip for LoRa, so the pinout of the board, LoRa and the way of invoking the RH drive vary, and modifications must be made to the original program.

Wiring
======

Wiring diagram for ADAFRUIT, for Lilygo the pins are GND=GND; BAT=> 3.3V; 12 => 22 and 11 => 21

Speaker: 22 and GND
Key: 21 and 3.3V
220Ω resistor: 21 and GND

Additional material
==================

For testing:
- Development board
- Dupont Line cables
  
For testing and final:
- LiLyGo ESP32 LoRa development board (Tested with T3_V1.6.1)
- 220 ohm resistor
- Headphones o Speaker (p.e P2.0 Termina Ultra Slim Mini Speaker with Cable, 8 Ohm, 8R, 0.5W speaker, diameter 20mm, 23mm, 28mm, 30mm, 36mm, 40mm, 50mm, 57mm)
- Key or key mores such as Portable Magnetic Amateur Radio, Shortwave Device, CW, Morse Code, QU-45255
- 3.5mm Male and Female Jack Connectors (PJ-341).

  
Testing
========

The Tappie walkie talkie pair works correctly. Its range is 1 km

Checking
========

- That the LiLygo board works correctly (Screen, SD, WiFi and LoRa) for this you can use , which allows us to check the LoRa PINout (https://github.com/Xinyuan-LilyGO/TTGO-LoRa-- Series/tree/master/LoRa)
- That LoRa emission by tuning the frequency with a SDR (e.g. RTL-SDR & SDR++)
- That the audio part work fine (https://sonotrigger-software.blogspot.com/2025/01/esp32-basic-starter-kit-for-dummys.html)
- Checking the program execution (trace): to do this, just uncomment
  Serial.begin(9600);
  while (!Serial) { delay(1); }
  Serial.println("LoRa radio init failed");
and see the output in the Arduino IDE => Tools => Serial Monitor

Acknowledgements
================

Alex Chang, author of the original project
Doug Skogman and Mike McCauley VK4AMM, without whose help I could not have solved the technical problems

Reference
=========

Walkie Tappie (Wireless Morse Code). Ditch your phones for a wireless Morse code transmitter!: https://projecthub.arduino.cc/alex_chang/walkie-tappie-wireless-morse-code-3725d6 
EA5JTT post in blog: https://sonotrigger-software.blogspot.com/2024/12/esp32-lora-for-dummys-walkie-tappie.html

