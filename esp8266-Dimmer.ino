#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "BROKER_IP";
const char* topicEncoder = "dial/encoder";
const char* topicButton = "dial/button";
char charPos [5];

#define pinSW D3  // Connected to SW  on KY-040
#define pinA  D1  // Connected to CLK on KY-040
#define pinB  D2  // Connected to DT  on KY-040
int encoderPosCount = 0; 
int pinALast;  
int aVal;
int Button;
int aButton;
boolean bCW;
String strTopic;
String strPayload;


WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("Dimmer");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if (strTopic == "dial/SetValue")
  {
    encoderPosCount = atoi((char*)payload);
    Serial.println (encoderPosCount);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("dial/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


 void setup() { 
   pinMode (pinA,INPUT);
   pinMode (pinB,INPUT);
   pinMode (pinSW, INPUT);
   /* Read Pin A
   Whatever state it's in will reflect the last position   
   */
   pinALast = digitalRead(pinA);   
   Serial.begin (115200);
   setup_wifi();
   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
 } 

 void loop() { 
   if (!client.connected()) {
     reconnect();
   }
   

   if (!(digitalRead(pinSW))) {
     // check if pushbutton is pressed
     client.publish(topicButton, "ON");
     while (!digitalRead(pinSW)) {}  // wait til switch is released
     delay(10);                      // debounce
     client.publish(topicButton, "OFF");        
   }
 
   aVal = digitalRead(pinA);
   if (aVal != pinALast){ // Means the knob is rotating
     // if the knob is rotating, we need to determine direction
     // We do that by reading pin B.
     if (digitalRead(pinB) != aVal) {
       // Means pin A Changed first - We're Rotating Clockwise
       encoderPosCount = encoderPosCount + 5;
       if (encoderPosCount >254) { encoderPosCount=254;}
     } else {
       // Otherwise B changed first and we're moving CCW
       encoderPosCount = encoderPosCount - 5;
       if (encoderPosCount <0) { encoderPosCount=0; }
     }
     //Serial.println(encoderPosCount);
     dtostrf(encoderPosCount,3,1,charPos);
     client.publish(topicEncoder, charPos);
     
   } 
   pinALast = aVal;
   client.loop();

 } 

