/*
  Automaton X1 - Main 
  EMD Subsystems
  Version : 1.0
  Rev: 3
  Ref: 24APR20
  Author: ubdussamad <ubdussamad@gmail.com>
*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "tools.h"



/* INITIALIZING SERVER OBJECT */
WiFiServer server(80);

/* AVAILABLE PINS */
String SSID,PSK;
String pinNames[RELAY_COUNT];

/* Setup routine, runs once on boot. */
void setup() {
  /* SYMBOL RATE FOR SERIAL COM */
  Serial.begin(115200);
  
  LOG(F("EMD SUBSYSTEMS"
        "\n\nAutomaton X1"
        "\nVersion 1 Rev 3"
        "\nRef: 25 APR 2020"
        "\nAuthor: ubdussamad <ubdussamad@gmail.com>"
        ));
  
  // Mounting the file system.
  LOGL("FS_INIT: "+ SPIFFS.begin() ? "\nSucess":"\nFailed");

  /* Fetching Relay Sates */
  LOG("Loading last Relay Sates: ");
  File relay_states_f = SPIFFS.open(RELAY_STATE_FILE, "r");
  if (!relay_states_f || RELAY_COUNT > relay_states_f.size() ){ LOG("Failed."); }
  else { int i=0;
  while ( relay_states_f.available()) {
    pinMode( GPIOS[i] , OUTPUT);
    digitalWrite( GPIOS[i], (char)relay_states_f.read() == '1' ? 1 : 0 );i++;}
  relay_states_f.close();
  LOG("Sucess.");
  }

  /* Loading Relay Names. */
  LOG("Reading Relay Names:");
  if (SPIFFS.exists(RELAY_NAME_FILE)) {
    int index = 0;char iter;
    File relayNameFile = SPIFFS.open(RELAY_NAME_FILE,"r");
    while ( relayNameFile.available() ){
      iter = (char) relayNameFile.read();
      if (iter==','){index++;continue;}
      pinNames[index]+=iter;
    }
    relayNameFile.close();
  }
  else {
    /* Write the config file with the constant data. */
    File f = SPIFFS.open( RELAY_NAME_FILE , "w");
    if (!f) {LOGL("ERROR:029");return;}
    String buffer;
    for( int i = 0; i < RELAY_COUNT; i++) {
      buffer+=String(GPIOS[i])+",";
    }
    f.print(buffer);f.close(); // Work on it.
  }
  
  


  /* Checking if Custom AP is Set and Vaild. */
  if (SPIFFS.exists(STA_SETTING_FILE)) {
    LOGL("Custom AP Read/Verify: ");
    File fObj = SPIFFS.open(STA_SETTING_FILE, "r");
    bool switch_ = true;char iter;
    while (fObj.available()){iter = (char)fObj.read();
    if (iter=='\n') {switch_ = false;continue;}
    if (switch_) { SSID += iter;}else { PSK += iter; }
    }fObj.close();
    if (switch_ || !SSID.length() || PSK.length() < 8){LOG("Failed.");}
    else {LOG("Sucess.");
    connect_to_station(SSID, PSK);}}

  /* Start the Server */
  server.begin();
  LOG(F("\nServer Running .. @"));
  LOGL(WiFi.localIP());
}

/* Main loop. */
void loop () {
  
  if (WiFi.status() != WL_CONNECTED) {
    connect_to_station(SSID , PSK);
  }

  /* Server.available() catches a client request and returns a */
  /* client instance, which inturn is captured by the variable client. */ 
  WiFiClient client = server.available();
  if (!client) {return;}
  
  /* Client data transmittion timeout. */
  client.setTimeout(1500);

  /* Disabling Nagle's packet buffering. */
  client.setNoDelay(true);
  
  /* Simply Reading the header. */
  String requestHeader = client.readStringUntil('\r');

  /* Send the request to dedicated handler routine. */
  requestHandler( requestHeader , &client );

  /* Flushing the data to the Client with a timeout of 500ms. */
  client.flush(500);

}
