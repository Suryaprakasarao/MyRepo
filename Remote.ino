#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>              //Needed Header files
#include <NTPClient.h>               
#include <WiFiUdp.h>



const char* ssid = "SriHome";
const char* password = "SrVaddadi";     //My Router Credentials

int button  = 0;
int previousState  = 0;
int publishstate = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* AWS_endpoint = "azs4o0fo5e3wd-ats.iot.ap-south-1.amazonaws.com";  //AWS MQTT broker endpoint ID

void callback(char* topic, byte* payload, unsigned int length) {             // Message is obtain one character at a time
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); 
  }
    char Rec = (char)payload[0]; 
    Serial.println(Rec);
    Serial.println();
    if(Rec==49)                                                //When a message is sent to the Smart Device, it's response from MQTT topic "RinTopic" is 
    {                                                           // indicated by making internal pin connected HIGH
      digitalWrite(D4,LOW);
      }
      else
      {
        digitalWrite(D4,HIGH);
        }
    
    }


WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient);           //Defining a Pubsubclient object "client" with given parameter
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {                                                  // Wifi Setup
                                                                            
  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing")) {
      Serial.println("connected");
      
      client.publish("$aws/things/Remote/shadow/update", "Connected");
      client.subscribe("RinTopic");                                         // Smart device will respond to this topic
    
    button = digitalRead(D2);                                               // Read input pin status
    if (previousState == 0 && button == '1') {                             // Condition to check button input and Toggle state, this state is published to MQTT
    previousState = 1;
    publishstate=!publishstate;
  }
  if (previousState == 1 && button == '0') {   
    previousState = 0;
  }
    if(publishstate == '1'){
    client.publish("$aws/things/Remote/shadow/update", "1");     //State is published here
    }
    if(publishstate == '0'){
    client.publish("$aws/things/Remote/shadow/update", "0");    
    }
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(9600);
  Serial.setDebugOutput(true);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(D4, OUTPUT);
  pinMode(D2, INPUT);                                            //Input button is connected to this pin of Esp8266
  setup_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");



    // Load CA file
    File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
    if (!ca) {
      Serial.println("Failed to open ca ");
    }
    else
    Serial.println("Success to open ca");

    delay(1000);

    if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
    else
    Serial.println("ca failed");

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
