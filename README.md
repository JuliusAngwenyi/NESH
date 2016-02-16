# Introduction

NESH will equip the elderly peoples homes with a set of sensors that will monitor them non-intrusively (for example without using cameras). The sensors will allow their relatives or social carers know whether they have got out of bed, they have taken medication and have taken a hot drink for example. The list could span most areas in the home, including bathroom, watching TV, used the fridge, among others.

The data from these sensors will be collected centrally using Azure IoT services suite. The data can be accessed remotely by social care services or relatives. The data can also be shared with family doctor or hospital trust

NESH will complement and not replace the social care for the elderly by letting our senior citizens enjoy their day to day chores non-intrusively and significantly reduce the cost social workers services.

#About this project
This repository is for my attempt at "World’s Largest Arduino Maker Challenge",  https://www.hackster.io/challenges/arduino-microsoft-maker

##Getting the Arduino connected to host PC
Follow the steps below to get your Arduino device connected to your host PC, ready to code!
The raw StandardFirmata sketch works for USB and Bluetooth, while modified versions are available for WiFi and Ethernet.
Follow the steps below to upload the StandardFirmata sketch to your Arduino.

1. Download and install the Arduino software from [http://arduino.cc](http://arduino.cc).
2. Connect your Arduino device to the computer using USB. 
3. Launch the Arduino application.
4. Verify that you have the correct Arduino board selected under *Tools > Board*
5. Verify that you have the correct COM Port selected under *Tools > Port*
6. In the Arduino IDE, navigate to *File > Examples > Firmata > StandardFirmata*
7. Verify that StandardFirmata will use the correct baud rate for your connection (see Notes on Serial Commuinication below)
8. Press “Upload” to deploy the StandardFirmata sketch to the Arduino device.


# Useful links

Arduino MKR1000 (US only)/Genuino MKR1000 (Outside US), https://www.arduino.cc/en/Main/ArduinoMKR1000

Windows Remote Arduino, http://ms-iot.github.io/content/en-US/win10/WRA.htm

Windows Remote Arduino Experience app from the Microsoft Store, https://www.microsoft.com/store/apps/9nblggh2041m


