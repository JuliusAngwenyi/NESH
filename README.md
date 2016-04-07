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
4. Install the SAMD board using the Boards Manager..*Tools > Board > Boards Manager...* That’s SAMD not SAM
4. Verify that you have the correct Arduino board selected under *Tools > Board*
5. Verify that you have the correct COM Port selected under *Tools > Port*
6. In the Arduino IDE, navigate to *File > Examples > Firmata > StandardFirmata*
7. Verify that StandardFirmata will use the correct baud rate for your connection (see Notes on Serial Commuinication below)
8. Press “Upload” to deploy the StandardFirmata sketch to the Arduino device.

##Getting WiFi setup on MKR1000 Arduino 
Follow the following steps to get you Arduino MKR1000 set up with latest Firmata with WiFi Support

The latest Firmata library is version 2.5.2 as of this writing. It has the WiFi transport support created by Jesse Frush.
There are two ways to get this latest version on to your Arduino IDE.

<strong>1. Arduino Library Manager:</strong>

If you have already installed the latest version of the Arduino Environment, you can navigate to Sketch > Include Library > Manage Libraries and search for “Firmata” .. If you see the 2.5.0 version or lower, you will need to upgrade it.

Select the library and see if you get the “Update” button. If you don’t get the update button, it means the library definitions are not yet in sync with the Arduino IDE. So you might have to install it manually by following option 2) below.

<strong>2. Manual update using the “Add Zip Library…”</strong>

The latest version of the Firmata library is available in the following github location. Anything at 2.5.2 or above should get you going.

https://github.com/firmata/arduino/releases

Depending on your Arduino IDE Version choose the appropriate firmata zip folder. If your Arduino IDE version is 1.6.7 for example, then download the Arduino-1.6.x-Firmata-2.5.2.zip file. Always check for latest.

You need to first remove existing Firmata library before you update it with the new version. You should find the Firmata library in the following folder on windows.

C:\Users\yourusername\Documents\Arduino\libraries

If you don’t find it in the above folder, check the following folder also

C:\Program Files (x86)\Arduino\libraries\

Delete  that Firmata folder.

Now you can go to Sketch>Include Library> Add Zip Library  and choose the already downloaded “Arduino-1.6.x-Firmata-2.5.2” zip file.

Now if you go back to the *Sketch > Include Library > Manage Libraries* and search for Firmata you should see the version 2.5.2 installed.

You are now all set with the latest Firmata library that has wifi support.

In addition, you might find an additional Firmata library under C:\Program Files (x86)\Arduino\libraries\

Now we also need to get the following two libraries also updated to their latest versions.  a) WiFi101 and b)RTCZero.  If you cannot do it through Manage Libraries, you can always do it manually. Locations of the latest releases are :

https://github.com/arduino-libraries/WiFi101/releases

https://github.com/arduino-libraries/RTCZero/releases

<strong>Updating the WiFi Config for MKR1000</strong>

1. On the Arduino IDE load the StandardFirmataWiFi sketch from *File>Examples>Firmata*
2. Switch to the wifiConfig.h tab. This contains all the wifi configuration needed for the different shields. By default the config is enabled for Arduino WiFi Shield. To enable it for MKR1000, you need to comment this line with a “//” in front of it
3.      &#35;define ARDUINO_WIFI_SHIELD
4.      and uncomment the following line by removing the “//”
5.      //&#35;define WIFI_101
6. Move down and make sure to update the ssid , comment or uncomment either WPA or WEP section based on your WiFi Network and wpa_passphrase or wep_key accordingly. There is also a WIFI_NO_SECURITY option if your WiFi doesn’t require any password.
7. As far as the IP Address and the Port configuration is  concerned, the default IP is obtained through DHCP and the default port is 3030. You can change these configurations if you want. If you don’t change, then you need to know the IP address that your MKR1000 will be assigned. For that you can run the sketch with the debug flag on so that you can open the Serial monitor and see the WiFi status of your MKR1000.
8. Uncomment the following line on the top section in the StandardFirmataWiFi file:
9.  //&#35;define SERIAL_DEBUG
10.  You should now be able to compile and upload the sketch to MKR1000. 
 
<strong>For further reference </strong>, follow steps also available at: http://mohanp.com/mkr1000-windows-remote-arduino-firmata-wifi/

##Firmware update sample screenshot
![Firmware update](https://raw.githubusercontent.com/JuliusAngwenyi/NESH/master/ProofOfConceptNesh/ProofOfConceptNesh/Assets/1.%20Firmware%20Updater%20for%20WiFi101.PNG)

##Setting up the SSL for Azure IoT Hub on the MKR1000 WiFi

Azure IoT Hub Services uses SSL to secure the communication. On the Arduino MKR1000 board, when the sketch runs, it needs to establish an SSL communication with the Azure IoT hub first before it can proceed further. The MKR1000’s WiFi chip doesn’t have that much memory. It cannot possible store a large number of SSL certificates like ordinary computer does. So we need to first upload the SSL certificate to the WiFi chip as follows:

1. Get the latest version of WiFi101 Firmware updater from here (https://github.com/arduino-libraries/WiFi101-FirmwareUpdater/releases/latest). 
2. Extract the contents of the zip file to a folder. You should see “winc1500-uploader-gui” in that folder. You can also use the console application found there.
3. On the Arduino IDE, choose the FirmwareUpdater sketch from *File > Examples > WiFi101 > FirmwareUpdater*
4. With the MKR1000 connected run this sketch. Your MKR1000 will now be ready to accept the new WiFi Firmware.
5. Run the “winc1500-uploader-gui” application
6. On the Ip or Domain name field provide the name of your IoTHub host name.  Click download to get the SSL certificate.
7. Select the COM port that MKR1000 is connected to.
8. Press upload certificates to send the certificate to the WiFi module. 
9. Once the certificates are uploaded you are all set for MKR1000 Azure IoT Hub interfacing.

<strong>Https certification into Arduino</strong>
![Https certification into Arduino](https://raw.githubusercontent.com/JuliusAngwenyi/NESH/master/ProofOfConceptNesh/ProofOfConceptNesh/Assets/2.%20Upload%20cert%20to%20Arduino%20WiFi101.PNG)


# Useful links

Arduino MKR1000 (US only)/Genuino MKR1000 (Outside US), https://www.arduino.cc/en/Main/ArduinoMKR1000

Windows Remote Arduino, http://ms-iot.github.io/content/en-US/win10/WRA.htm

Windows Remote Arduino Experience app from the Microsoft Store, https://www.microsoft.com/store/apps/9nblggh2041m


