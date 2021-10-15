/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  modified by Cody Snow to automate a halloween fogger 
  October 2021
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       ""
#define WLAN_PASS       ""

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

// Subscribe to onoff state changes
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

// Subscribe to the timer interval length from slider. If greater than 0, activate timer loop
Adafruit_MQTT_Subscribe interval = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/interval");

// Publish when fogger is ready 
Adafruit_MQTT_Publish isready = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ready");

// Publish when fogger is running 
Adafruit_MQTT_Publish fogger_running = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/fogger_running");

/******************** Variable Declarations *********************************/

unsigned long previousMillis = millis();
unsigned long currentMillis = millis(); 
int timer_interval = 0;
boolean fogger_ready = false;
boolean fogging = false;



/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
void warmup();
void interval_fog();
void fog();
void recover();

void setup() {
  Serial.begin(115200);
  delay(10);

  // Launch warmup loop
  warmup();

  // Set up pin 14 for relay control
  pinMode(14, OUTPUT);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed, activate_loop, and interval
  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&activate_loop);
  mqtt.subscribe(&interval);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (strcmp((char *)onoffbutton.lastread, "ON") == 0 && fogger_ready && !fogging) {
        fog();
      }
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\nFogger is ready "));
  Serial.print(true);
  Serial.print("...");
  if (! isready.publish(true)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
  
}

// function to allow the fogger time to do its initial warmup
void warmup(){

  // publish that fogger is in a "not ready" state

  // wait 300 seconds for fogger to warm up
  
  // set fogger_ready to true and publish that fogger is in a 
  // "ready" state once 300 seconds has passed
  if (elapsedTime >= 300000) {
    fogger_ready = true;

    // publish ready state
    
  }
}

// function that triggers fog at a regular interval 
void interval_fog(){
  // check amount of time that has passed since last fog activation

  // add time to counter

  // call fog if interval has elapsed 
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    fog();
  }
    
}

// function that runs the fogger for 30 seconds
void fog(){
  // set variables 
  fogging = true; 
  
  // start fogging
  digitalWrite(14, HIGH); 

  // publish that the fogger is running

  // allow 30 seconds to pass
  

  // after 30 seconds, publish that the fogger is not running
  
  // turn off relay switch
  digitalWrite(14, LOW); 

  // set variables 
  fogger_ready = false; 

  // call recover function here? or in main loop?
  recover();
}


// function to allow the fogger to recover from fogging and reach its ready state 
void recover(){
  // publish that fogger is in a "not ready" state

  // wait 30 seconds 

  // publish that fogger is in a "ready" state
  
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
