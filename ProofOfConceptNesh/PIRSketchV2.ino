/*
This code connects to Azure IoT Hub.
Some logic has been adapted from Mohan Palanisamy (http://mohanp.com) 
*/

#include <SPI.h>
#include <WiFi101.h>
#include <RTCZero.h>

///*** PINs Config ***///
const int MKR1000_LED = 6 ;
const int  MKR1000_PINPIR1 = 14;               // choose the input pin (for PIR sensor)



int calibrationTime = 30;       //the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int val = 0;                    // variable for reading the pin status
long unsigned int lowIn;         //the time when the sensor outputs a low impulse
long unsigned int pause = 5000;  //the amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;  
String duration ="0";
boolean webServiceSucceeded;

///*** WiFi Network Config ***///
char ssid[] = "YourWifiSSID"; //  your network SSID (name)
char pass[] = "YourPassword";    // your network password (use for WPA, or use as key for WEP)

///*** Azure IoT Hub Config ***///
//see: http://mohanp.com/  for details on getting this right if you are not sure.

char hostname[] = "nesh.azure-devices.net";    // host name address for your Azure IoT Hub
char authSAS[] = "YourSharedAccessSignature";

String azureReceive = "/devices/YourDeviceId/messages/devicebound?api-version=2016-02-03"; //feed URI
char senduri[] = "/devices/YourDeviceId/messages/events?api-version=2016-02-03";//send URI

// message Complete/Reject/Abandon URIs.  "etag" will be replaced with the message id E-Tag recieved from recieve call.
String azureComplete = "/devices/YourDeviceId/messages/devicebound/etag?api-version=2016-02-03";         
String azureReject   = "/devices/YourDeviceId/messages/devicebound/etag?reject&api-version=2016-02-03";  
String azureAbandon  = "/devices/YourDeviceId/messages/devicebound/etag/abandon?&api-version=2016-02-03"; 

///*** Azure IoT Hub Config ***///

unsigned long lastConnectionTime = 0;            
const unsigned long pollingInterval = 5L * 1000L; // 5 sec polling delay, in milliseconds

int status = WL_IDLE_STATUS;

WiFiSSLClient client;

///*** RTCZero Config ***///
/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 00;
byte minutes = 18;
byte hours = 20;

/* Change these values to set the current initial date */
byte day = 10;
byte month = 4;
byte year = 16;

///*** RTCZero Config ***///

void setup() {

   pinMode(MKR1000_LED, OUTPUT);

   pinMode(MKR1000_PINPIR1, INPUT);     // declare sensor as input

    
  //give the sensor some time to calibrate
  delay(50);
  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);

    //Uncomment to set RTC manually
    //Serial.println("Syncing RTC:");
    //syncRTC(year, month, day, hours, minutes, seconds);    
    //Serial.print("Successfully synced RTC, date time set to:");
    Serial.println(getTimeStamp()); // Print date...
    
    delay(50);
     
    Serial.println("check for the presence of the shield");
  //check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }
   Serial.println("Good. this board supports Wifi!");

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.println("wait 10 seconds for wifi connection:");
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("wifi connection succeeded:");
}

// this method makes an HTTPS connection to the Azure IOT Hub Server:
boolean azureHttpPost(String content) {

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
    //lastConnectionTime = millis();
    return true;
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection to azure webservice failed");
    reconnetWifi();
    return false;
  }
}

void loop(){
  val = digitalRead(MKR1000_PINPIR1);   // read input value
  if (val == HIGH) {                    // check if the input is HIGH
    digitalWrite(MKR1000_LED, HIGH);    // turn LED ON    

    if (lockLow) {           
         lockLow = false;               //makes sure we wait for a transition to LOW before any further output is made:
         duration = millis()/1000;
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(duration);
         Serial.println(" sec");                 
         delay(50);
         webServiceSucceeded = azureHttpPost("{deviceId:NeshMKR100Dev1, motion:1,timestamp:" + getTimeStamp() + "}");
         //webServiceSucceeded = httpRequest("POST", senduri,"text/plain","{deviceId:NeshMKR100Dev1, motion:1,timestamp:" + getTimeStamp() + "}");
         if(webServiceSucceeded == false)
         {
            Serial.println("retrying webservice call ");
            azureHttpPost("{deviceId:NeshMKR100Dev1, motion:1,timestamp:" + getTimeStamp() + "}");
            //httpRequest("POST", senduri,"text/plain","{deviceId:NeshMKR100Dev1, motion:1,timestamp:" + getTimeStamp() + "}");
         }
    }
    
    takeLowTime = true;
  } 
  else {
      digitalWrite(MKR1000_LED, LOW);   // turn LED OFF     

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
           delay(50);
          webServiceSucceeded = azureHttpPost("{deviceId:NeshMKR100Dev1, motion:0,timestamp:" + getTimeStamp() + "}");
          //webServiceSucceeded = httpRequest("POST", senduri,"text/plain","{deviceId:NeshMKR100Dev1, motion:0,timestamp:" + getTimeStamp() + "}");
          if(webServiceSucceeded == false)
         {
            Serial.println("retrying webservice call ");
            azureHttpPost("{deviceId:NeshMKR100Dev1, motion:0,timestamp:" + getTimeStamp() + "}");
            //webServiceSucceeded = httpRequest("POST", senduri,"text/plain","{deviceId:NeshMKR100Dev1, motion:0,timestamp:" + getTimeStamp() + "}");
         }
    }
  }

  /* ------Poling for Cloud-to-Device messages------*/
  
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
      digitalWrite(MKR1000_LED, LOW); //turn off onboard LED 
    }
    else
    {          
      digitalWrite(MKR1000_LED, HIGH); //turn on onboard LED 
      //Now that the message is processed.. do either Complete, Reject or Abandon HTTP Call.

      //first get the ETag from the received message response 
      String eTag=getHeaderValue(response,"ETag");
      Serial.print("Azure response received eTag: ");
      Serial.println(eTag);
      String syncRTC = getResponsePayload(response); //E.g. SyncRTC: "2016-4-10T21:00"
      if ( syncRTC.startsWith("SyncRTC:"))
      {
        Serial.println(syncRTC);
        
        syncRTCReceived(syncRTC);

        Serial.print("Successfully synced RTC, date time set to:");
        Serial.println(getTimeStamp()); // Print date...
            
        //Uncomment the following line and comment out Reject and Abandon calls to verify Complete
        azureIoTCompleteMessage(eTag);
      }
      else
      {
        //Uncomment the following line and comment out Complete and Abandon calls to verify Reject
        azureIoTRejectMessage(eTag);        
      }

      //Uncomment the following line and comment out Complete and Reject calls to verify Abandon
      //azureIoTAbandonMessage(eTag); 

    }
  }
  
  // polling..if pollingInterval has passed
  if (millis() - lastConnectionTime > pollingInterval) 
  {
    digitalWrite(MKR1000_LED, LOW);
    azureIoTReceiveMessage();    
    lastConnectionTime = millis();// note the time that the connection was made:
  }  

  /* ------- Polling for C2D messages ------*/
}

void reconnetWifi(){
    // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.println("wait 10 seconds for wifi connection:");
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("wifi connection succeeded:");
}

void syncRTC(byte year, byte month, byte day, byte hours, byte minutes, byte seconds){
    rtc.begin(); // initialize RTC  
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year);
}

String getTimeStamp(){
  //ISO8601 GMT yyyy-mm-ddTHH:MM:SS
  String dateTime = "20" + String(rtc.getYear()) + "-" + String(rtc.getMonth())  + "-" + String(rtc.getDay())+ "T" + String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds());
  return dateTime;
}

void syncRTCReceived(String resp)
{
  //String resp = "SyncRTC: "2016-4-10T21:00";
  int idxStart=12;  
  int idxEnd = resp.indexOf("-", idxStart);
  year = resp.substring(idxStart,idxEnd).toInt();  

  idxStart=  idxEnd +1;
  idxEnd = resp.indexOf("-", idxStart);
  month = resp.substring(idxStart,idxEnd).toInt();    
  
  idxStart=  idxEnd +1;
  idxEnd = resp.indexOf("T", idxStart);  
  day = resp.substring(idxStart,idxEnd).toInt();   

  idxStart=  idxEnd +1;
  idxEnd = resp.indexOf(":", idxStart);
  hours = resp.substring(idxStart,idxEnd).toInt();   

  idxStart=  idxEnd +1;
  idxEnd = idxStart +2;
  minutes = resp.substring(idxStart,idxEnd).toInt();   

  rtc.begin(); // initialize RTC  
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
}


//Receive Azure IoT Hub "cloud-to-device" message
void azureIoTReceiveMessage()
{
  webServiceSucceeded = httpRequest("GET", azureReceive, "","");
  if(webServiceSucceeded == false)
  {
    Serial.println("retrying webservice call ");
    httpRequest("GET", azureReceive, "","");
  }
}

//Tells Azure IoT Hub that the message with the msgLockId is handled and it can be removed from the queue.
void azureIoTCompleteMessage(String eTag)
{
  String uri=azureComplete;
  uri.replace("etag",trimETag(eTag));

  webServiceSucceeded = httpRequest("DELETE", uri,"","");
  if(webServiceSucceeded == false)
  {
    Serial.println("retrying webservice call ");
    httpRequest("DELETE", uri,"","");
  }  
}


//Tells Azure IoT Hub that the message with the msgLockId is rejected and can be moved to the deadletter queue
void azureIoTRejectMessage(String eTag)
{
  String uri=azureReject;
  uri.replace("etag",trimETag(eTag));

  webServiceSucceeded = httpRequest("DELETE", uri,"","");
  if(webServiceSucceeded == false)
  {
    Serial.println("retrying webservice call ");
    httpRequest("DELETE", uri,"","");
  }    
}

//Tells Azure IoT Hub that the message with the msgLockId is abondoned and can be requeued.
void azureIoTAbandonMessage(String eTag)
{
  String uri=azureAbandon;
  uri.replace("etag",trimETag(eTag));

  webServiceSucceeded = httpRequest("POST", uri,"text/plain","");
  if(webServiceSucceeded == false)
  {
    Serial.println("retrying webservice call ");
     httpRequest("POST", uri,"text/plain","");
  }    
}


bool httpRequest(String verb, String uri,String contentType, String content)
{
    if(verb.equals("")) return false;
    if(uri.equals("")) return false;

    // close any connection before send a new request.
    // This will free the socket on the WiFi shield
    client.stop();
  
    // if there's a successful connection:
    if (client.connect(hostname, 443)) {
      client.print(verb); //send POST, GET or DELETE
      client.print(" ");  
      client.print(uri);  // any of the URI
      client.println(" HTTP/1.1"); 
      client.print("Host: "); 
      client.println(hostname);  //with hostname header
      client.print("Authorization: ");
      client.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
      client.println("Connection: close");

      if(verb.equals("POST"))
      {
          client.print("Content-Type: ");
          client.println(contentType);
          client.print("Content-Length: ");
          client.println(content.length());
          client.println();
          client.println(content);
      }else
      {
          client.println();
      }

      return true;
    }
  else {
    // if you couldn't make a connection:
    Serial.println("connection to azure webservice failed");
    reconnetWifi();
    return false;
  }    
}

String getHeaderValue(String response, String headerName)
{
  String headerSection=getHeaderSection(response);
  String headerValue="";
  
  int idx=headerSection.indexOf(headerName);
  
  if(idx >=0)
  { 
  int skip=0;
  if(headerName.endsWith(":"))
    skip=headerName.length() + 1;
  else
    skip=headerName.length() + 2;

  int idxStart=idx+skip;  
  int idxEnd = headerSection.indexOf("\r\n", idxStart);
  headerValue=response.substring(idxStart,idxEnd);  
  }
  
  return headerValue;
}

//For some reason Azure IoT sets ETag string enclosed in double quotes 
//and that's not in sync with its other endpoints. So need to remove the double quotes
String trimETag(String value)
{
    String retVal=value;

    if(value.startsWith("\""))
      retVal=value.substring(1);

    if(value.endsWith("\""))
      retVal=retVal.substring(0,retVal.length()-1);

    return retVal;     
}

//To get all the headers from the HTTP Response
String getHeaderSection(String response)
{
  int idxHdrEnd=response.indexOf("\r\n\r\n");

  return response.substring(0, idxHdrEnd);
}

//To get only the message payload from http response.
String getResponsePayload(String response)
{
  int idxHdrEnd=response.indexOf("\r\n\r\n");

  return response.substring(idxHdrEnd + 4);
}

