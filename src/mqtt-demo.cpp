/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "MQTT.h"

/*
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);
*/

// Pin definitions
#define LED0 D7                   //  Onboard LED
#define LED1 D0                   //  External LED
#define BTN1 A0                   //  BTN1 toggles flash LED0
#define ADCpin A5                 //  Potentiometer Pin

// Global function headers
void flashLED0();
void toggleFlashEnabled();
void callback(char* topic, byte* payload, unsigned int length);

// Global variables
int adcVal;                       //  value read from ADCpin
bool volatile flashEnabled;       //  if true --> flash
Timer timer(200, flashLED0);


// MQTT client setup
const uint8_t MQTTServer[] = {153, 104, 61, 218};
const char* MQTTClientName = "photon60";
const char* MQTTBrokerUsername = "ece2431";
const char* MQTTBrokerPassword = "villaNova";
MQTT client(MQTTServer, 1883, callback);


void setup() {
  Serial.begin(9600);
  Serial.println("Particle on.");

  // Initializing pins
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(BTN1, INPUT_PULLDOWN);
  flashEnabled = 0;
  timer.start();

  //  Attatch interrupt for BTN1 to activate flashing
  attachInterrupt(BTN1, toggleFlashEnabled, RISING);

  //  Initialize Particle Variables
  Particle.variable("adcValue", adcVal);
  Particle.variable("flashEnabled", flashEnabled);

  //  Connect to MQTT Broker
  client.connect(MQTTClientName, MQTTBrokerUsername, MQTTBrokerPassword);

  if (client.isConnected()) {
    Serial.println("MQTT broker connected.");
    client.publish("photon60/outTopic/message","hello world");
    client.subscribe("photon60/inTopic/message");
  }
  else{
    Serial.println("Failed connecting to MQTT broker.");
  }
}

void loop() {
  adcVal = analogRead(ADCpin) / 16;
  analogWrite(LED1, adcVal);

  if (client.isConnected()){
    client.loop();
    client.publish("photon60/adcVal", String(adcVal));
    client.publish("photon60/LED0status", String(flashEnabled));
  }

  delay(100);
}



//      GLOBAL FUNCTIONS

// This function handles MQTT payloads
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    Serial.println(p);
}

void toggleFlashEnabled () {
    flashEnabled = !flashEnabled;
}

void flashLED0(){
    bool currentLEDStatus = digitalRead(LED0);
    if (flashEnabled){                                      //When flashEnabled, LED changes state
        digitalWrite(LED0, !currentLEDStatus);              //This happens every 200ms --> blinking
    }
    else {  
        digitalWrite(LED0, HIGH);                           //Make sure when not flashEnabled, LED must be on
    }
}