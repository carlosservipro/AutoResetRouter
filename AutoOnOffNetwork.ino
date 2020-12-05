//
// De Carlos Ruiz para todos
// Trabajando con ingenio
// Muchas gracias por visitar mi canal
// Este codigo fue adaptado de https://gist.github.com/ItKindaWorks/24fac4d9b0dcc11670a1ffcc947babf7 de ESP8266 a la nueva version que es la ESP32 

#include "WiFi.h"
#include <ESP32Ping.h>
#include "Metro.h"

//Variables to define ports
#define led 2
#define relay 13

//Wifi connection ssid and password
const char* ssid="INFINITUMlclu";
const char* pass="6055fa06f6";

//URL a monitorear
const char* pingHost = "www.google.com";

// 30 seconds if no connection to URL
Metro connectTimeout = Metro(30000);
// 5 secods every time to do the ping to URL
Metro pingTimer = Metro(5000);
// 10 seconds to power on again
Metro relayTimer = Metro(10000);
//2 minutes to wait later power on
Metro cooldownTimer = Metro(120000);

//3 state machine
enum states {CHECKING, RUNNING, COOLDOWN};
//Current state machine
int8_t currentState = CHECKING;
//Set default ping status
bool pingStatus = false;

void setup() {
  //Set serial por to enable debug mesages
  Serial.begin(115200);
  //Set pin mode
  pinMode(led,OUTPUT);
  pinMode(relay,OUTPUT);
  //Turn on relay
  digitalWrite(relay, HIGH);
  //WiFi start connection
  WiFi.begin(ssid,pass);
  //Conection led in off
  digitalWrite(led, LOW);
  //Checking Wifi conection
  while(WiFi.status()!=WL_CONNECTED){
   delay(1000);
   Serial.println("Connecting to WiFi..."); 
  }
  //If connection was ok, then turn on led status
  Serial.println(WiFi.localIP());
  digitalWrite(led, HIGH);
}

void loop() {
  //Firts machine state url ping process
  if(currentState == CHECKING && pingTimer.check()){
      //Update pingstatus with results of ping or false if no wifi
      if(WiFi.status()==WL_CONNECTED)
      {
        pingStatus = Ping.ping(pingHost,3);
        if(pingStatus){
          //led on, ping url ok
          digitalWrite(led, HIGH);
        }
        else{
          //led off, ping url fail
          digitalWrite(led, LOW);
        }
      }
      else{
        pingStatus = false;
        //led off, ping url fail
        digitalWrite(led, LOW);
        }
      //Print out results of ping attempt
      if(pingStatus){ Serial.println(String("Good ping to " + String(pingHost)));}
      else{           Serial.println(String("Could not ping " + String(pingHost)));}
      pingTimer.reset();
  }
  //Reset the timer if the pingStatus is true
  if(pingStatus){
    connectTimeout.reset();
  }
  //If the ESP cannot connect
  //Then trigger the relay pin and start (reset) the relay timer
  if(currentState == CHECKING && connectTimeout.check() ){
    Serial.println("Cannot connect to wifi or cannot reach WAN, restarting router...");
    digitalWrite(relay, LOW);
    relayTimer.reset();
    currentState = RUNNING;
  }
  //If the relay is on and the timer for turning the relay off has elapsed
  //Then turn the relay off
  else if(currentState == RUNNING && relayTimer.check()){
    Serial.println("Turning router back on...");
    digitalWrite(relay, HIGH);
    digitalWrite(led, LOW);
    cooldownTimer.reset();
    currentState = COOLDOWN;
  }
  //Cooldown state timer checker (cooldown to prevent the router from being rebooted while still starting up)
  if(currentState == COOLDOWN && cooldownTimer.check()){
    Serial.println("Router restart cooldown complete returning to normal state.");
    connectTimeout.reset();
    currentState = CHECKING;
  }
  //Time to repeat again the process
  delay(50);
}
