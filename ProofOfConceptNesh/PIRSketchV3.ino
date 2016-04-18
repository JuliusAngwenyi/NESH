/*
This code connects to Azure IoT Hub.
Some logic has been adapted from Mohan Palanisamy (http://mohanp.com) 
*/

#include <SPI.h>
#include <WiFi101.h>

///*** PINs Config ***///
const int MKR1000_LED = 6 ;
const int  MKR1000_PINPIR1 = 14;               // choose the input pin (for PIR sensor)



int calibrationTime = 30;       //the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int val = 0;                    // variable for reading the pin status
long unsigned int lowIn;         //the time when the sensor outputs a low impulse
long unsigned int pause = 5000;  //the amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;  
boolean webServiceSucceeded;

///*** WiFi Network Config ***///
char ssid[] = "YourWifiSSID"; //  your network SSID (name)
char pass[] = "YourPassword";    // your network password (use for WPA, or use as key for WEP)

///*** Azure IoT Hub Config ***///
//see: http://mohanp.com/  for details on getting this right if you are not sure.

char hostname[] = "nesh.azure-devices.net";    // host name address for your Azure IoT Hub
char authSAS[] = "YourSharedAccessSignature";

String azureReceive = "/devices/YourDeviceId/messages/devicebound?api-version=2016-02-03"; //feed URI
String senduri = "/devices/YourDeviceId/messages/events?api-version=2016-02-03";//send URI

// message Complete/Reject/Abandon URIs.  "etag" will be replaced with the message id E-Tag recieved from recieve call.
String azureComplete = "/devices/YourDeviceId/messages/devicebound/etag?api-version=2016-02-03";         
String azureReject   = "/devices/YourDeviceId/messages/devicebound/etag?reject&api-version=2016-02-03";  
String azureAbandon  = "/devices/YourDeviceId/messages/devicebound/etag/abandon?&api-version=2016-02-03"; 

///*** Azure IoT Hub Config ***///

unsigned long lastConnectionTime = 0;            
const unsigned long pollingInterval = 60L * 1000L; // 60 sec polling delay, in milliseconds

int status = WL_IDLE_STATUS;

WiFiSSLClient client;
WiFiSSLClient postingClient;

String payload;
String eTag;

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
  postingClient.stop();

  // if there's a successful connection:
  if (postingClient.connect(hostname, 443)) {
    //make the GET request to the Azure IOT device feed uri
    postingClient.print("POST ");  //Do a POST
    postingClient.print(senduri);  // On the sendURI
    postingClient.println(" HTTP/1.1"); 
    postingClient.print("Host: "); 
    postingClient.println(hostname);  //with hostname header
    postingClient.print("Authorization: ");
    postingClient.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
    postingClient.println("Connection: close");
    postingClient.println("Content-Type: text/plain");
    postingClient.print("Content-Length: ");
    postingClient.println(content.length());
    postingClient.println();
    postingClient.println(content);
    postingClient.println();

    Serial.println("---"); 
    Serial.println("service call to azure successful!");
    Serial.println("azureHttpPost");
    Serial.println(senduri); 
    Serial.println(content);            
    Serial.println("---");     
    return true;
  }
  else {
    // if you couldn't make a connection:
    Serial.println("---"); 
    Serial.println("connection to azure webservice failed");
    Serial.println("azureHttpPost");
    Serial.println(senduri);
    Serial.println(content); 
    Serial.println("---"); 
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
         Serial.println("---");
         Serial.println("motion detected at ");                
         delay(150);  
         webServiceSucceeded = azureHttpPost("{""deviceId"":""NeshMKR100Dev1"", ""motion"":""1""}");
         if(webServiceSucceeded == false)
         {
            delay(150);
              Serial.println("retrying webservice call ");
              azureHttpPost("{""deviceId"":""NeshMKR100Dev1"", ""motion"":""1""}");
         }
         else
         {
             delay(1000);//delay at least 1 sec to allow web service to finish
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
           Serial.println("---");           
           Serial.println("motion ended at ");      //output
           delay(150);
         
           webServiceSucceeded = azureHttpPost("{""deviceId"":""NeshMKR100Dev1"", ""motion"":""0""}");
           if(webServiceSucceeded == false)
           {
              delay(150);
              Serial.println("retrying webservice call ");
              azureHttpPost("{""deviceId"":""NeshMKR100Dev1"", ""motion"":""0""}");
           }
           else
           {
             delay(1000);//delay at least 1 sec to allow web service to finish
           }                     
      }
    
  }

  /* ------Poling for Cloud-to-Device messages------*/
  delay(150);
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
      eTag=getHeaderValue(response,"ETag");
      Serial.print("Azure response received eTag: ");
      Serial.println(eTag);
      payload = getResponsePayload(response); //E.g. SyncRTC:"2016-4-10T21:00"
      if ( payload.startsWith("SyncRTC:"))
      {
        Serial.println(payload);
            
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
    bool reconnected = false;
    while (status != WL_CONNECTED) {
      status = WiFi.begin(ssid, pass);
      Serial.println("wait 10 seconds for wifi re-connection:");
      // wait 10 seconds for connection:
      delay(10000);
      reconnected = true;
    }
    if(reconnected ==true){
      Serial.println("wifi re-connection succeeded:");
    }
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
      Serial.println("---"); 
      Serial.println("service call to azure successful!");
      Serial.println(verb);
      Serial.println(uri); 
      Serial.println(content);            
      Serial.println("---"); 
      return true;
    }
  else {
    // if you couldn't make a connection:
    Serial.println("---"); 
    Serial.println("connection to azure webservice failed");
    Serial.println(verb);
    Serial.println(uri);
    Serial.println(content); 
    Serial.println("---");    
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
