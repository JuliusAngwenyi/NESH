/*
This code connects to Azure IoT Hub.
Some logic has been adapted from Mohan Palanisamy (http://mohanp.com) 
*/

#include <SPI.h>
#include <WiFi101.h>

///*** PINs Config ***///
const int MKR1000_LED = 6 ;
const int  MKR1000_PINPIR1 = 14;               // choose the input pin (for PIR sensor)


int val = 0;                    // variable for reading the pin status
long unsigned int lowIn;         //the time when the sensor outputs a low impulse
long unsigned int pause = 5000;  //the amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;  
String duration ="0";

///*** WiFi Network Config ***///
char ssid[] = "YourWifiSSID"; //  your network SSID (name)
char pass[] = "YourPassword";    // your network password (use for WPA, or use as key for WEP)

///*** Azure IoT Hub Config ***///
//see: http://mohanp.com/  for details on getting this right if you are not sure.

char hostname[] = "nesh.azure-devices.net";    // host name address for your Azure IoT Hub
char feeduri[] = "/devices/YourDeviceId/messages/devicebound?api-version=2016-02-03"; //feed URI
char senduri[] = "/devices/YourDeviceId/messages/events?api-version=2016-02-03";//send URI
char authSAS[] = "YourSharedAccessSignature";

///*** Azure IoT Hub Config ***///

unsigned long lastConnectionTime = 0;            
const unsigned long pollingInterval = 5L * 1000L; // 5 sec polling delay, in milliseconds

int status = WL_IDLE_STATUS;

WiFiSSLClient client;

void setup() {

   pinMode(MKR1000_LED, OUTPUT);

   pinMode(MKR1000_PINPIR1, INPUT);     // declare sensor as input
    
  //check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.println("wait 10 seconds for wifi connection:");
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("wifi connection succeeded:");
}

void azureMessageloop() 
{
  String response = "";
  char c;
  ///read response if WiFi Client is available
  while (client.available()) {
    c = client.read();
    response.concat(c);
  }

  if (!response.equals(""))
  {
    //if there are no messages in the IoT Hub Device queue, Azure will return 204 status code. 
    if (response.startsWith("HTTP/1.1 204"))
    {
      //turn off onboard LED
      digitalWrite(MKR1000_LED, LOW);
    }
    else
    {
      //turn on onboard LED
      digitalWrite(MKR1000_LED, HIGH);
    }
  }

  // polling..if pollingInterval has passed
  if (millis() - lastConnectionTime > pollingInterval) {
    digitalWrite(MKR1000_LED, LOW);   
  }
}

// this method makes an HTTPS connection to the Azure IOT Hub Server:
void azureHttpRequest() {

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(hostname, 443)) {
    //make the GET request to the Azure IOT device feed uri
    client.print("GET ");  //Do a GET
    client.print(feeduri);  // On the feedURI
    client.println(" HTTP/1.1"); 
    client.print("Host: "); 
    client.println(hostname);  //with hostname header
    client.print("Authorization: ");
    client.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection to azure webservice failed");
  }
}

// this method makes an HTTPS connection to the Azure IOT Hub Server:
void azureHttpPost(String content) {

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(hostname, 443)) {
    //make the GET request to the Azure IOT device feed uri
    client.print("POST ");  //Do a POST
    client.print(senduri);  // On the sendURI
    client.println(" HTTP/1.1"); 
    client.print("Host: "); 
    client.println(hostname);  //with hostname header
    client.print("Authorization: ");
    client.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
    client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(content.length());
    client.println();
    client.println(content);
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection to azure webservice failed");
  }
}

void loop(){
  val = digitalRead(MKR1000_PINPIR1);   // read input value
  if (val == HIGH) {                    // check if the input is HIGH
    digitalWrite(MKR1000_LED, HIGH);    // turn LED ON
    //delay(150);

    if (lockLow) {           
         lockLow = false;               //makes sure we wait for a transition to LOW before any further output is made:
         duration = millis()/1000;
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(duration);
         Serial.println(" sec");                 
         
         azureHttpPost("{deviceId:NeshMKR100Dev1, motion:1,elapsed:"+ duration + "}");
    }
    
    takeLowTime = true;
  } 
  else {
      digitalWrite(MKR1000_LED, LOW);   // turn LED OFF
      //delay(150);    

       if(takeLowTime){
          lowIn = millis();             //save the time of the transition from high to LOW
          takeLowTime = false;          //make sure this is only done at the start of a LOW phase
        }    

       //if the sensor is low for more than the given pause, we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){  
           //makes sure this block of code is only executed again after a new motion sequence has been detected
           lockLow = true;        
           duration = (millis() - pause)/1000;              
           Serial.print("motion ended at ");      //output
           Serial.print(duration);
           Serial.println(" sec");    
           
          azureHttpPost("{deviceId:NeshMKR100Dev1, motion:0,elapsed:"+ duration + "}");  
    }
  }
}