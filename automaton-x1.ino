/*
  Automaton X1
  EMD Subsystems
  Version : 1.0
  Rev: 3
  Ref: 24APR20
  Author: ubdussamad <ubdussamad@gmail.com>
*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include "tools.h"
#include "prototypes.h"

/* PRODUCT CONFIG BLOCK */
#define PRODUCT_NAME "AutomatonX1\n"
#define VERSION "V01REV03\n"
#define REF "24APR20\n"
#define UID "002\n"


/* TIMEOUTS AND CONFIG [SUBJECT TO CHANGE DURING PRODUCTION] */
#define USER_AP_TRIAL_TIMEOUT 15000 // milliseconds
#define DEFAULT_AP_TRIAL_TIMEOUT 7000 // milliseconds

/* DEFAULTs [SUBJECT TO CHANGE DURING PRODUCTION] */
#define DEFAULT_STA_SSID "Galaxy A50"
#define DEFAULT_STA_PSK "zebrafamily"
#define STA_SETTING_FILE "/credentials.config"
#define RELAY_STATE_FILE "/states.config"
#define RELAY_NAME_FILE "/relayNames.config"

/* DEBUG CONFIGS */
#define DEBUG true
#define LOG Serial.print
#define LOGL Serial.println
#define STR String


/* INITIALIZING SERVER OBJECT */
WiFiServer server(80);

/* AVAILABLE PINS */
#define RELAY_COUNT 3
int GPIOS[8] = {4 , 5 , 14};// , 12 , 13 };
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

/* Routine for Connecting to a AP Station. */
void connect_to_station (const String& SSID ,const String&  PSK ){
  LOG(F("Connecting to custom AP: "));
  Serial.println(SSID);
  Serial.println("Please Wait..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID,PSK);
  unsigned long initial_epoch = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if ( (millis() - initial_epoch) > USER_AP_TRIAL_TIMEOUT ){ 
      WiFi.begin( DEFAULT_STA_SSID , DEFAULT_STA_PSK );
      Serial.println("Custom AP timeout, trying default..");
      while ((millis() - initial_epoch) > \
      (USER_AP_TRIAL_TIMEOUT + DEFAULT_AP_TRIAL_TIMEOUT)\
      && WiFi.status() != WL_CONNECTED) { 
      Serial.println(".");
      }
      if (WiFi.status() != WL_CONNECTED) {ESP.restart();}
    }
    delay(500);Serial.print(F("."));
  }
  Serial.println(F("\nStation Successfully connected to AP."));
  return;
}

/* Routine for handling HTTP requests */
/* Client pointer is taken so print the data directly to prevent copying. */
void requestHandler ( const String& requestHeader , WiFiClient * client ) {
  /* Basic parsing of the request. */
  
  /* Most frequent request. */
  int index = requestHeader.indexOf(F("/gpio"));

  if (index>0) {
    int relayId = requestHeader.substring(index+5).toInt();
    digitalWrite( relayId ,
     !digitalRead(relayId) );
    (*client).print(register_switch_states());
    return;
  }

  if (requestHeader.indexOf(F("/stat"))>0) {
    ((*client).print( STR(PRODUCT_NAME)+
                      STR(VERSION)+
                      STR(REF)+
                      STR(UID)+
                      register_switch_states()
                    ));
    return;
  }

  if (requestHeader.indexOf(F("/restart"))>0) {
    ESP.restart();
    return;
  }

  if (requestHeader.indexOf(F("/config"))>0) {
    serveFile("/config.html" , client );
    return;
  }

  if (requestHeader.indexOf(F("/burncred"))>0) {
    int headIndex = requestHeader.indexOf("afstass%32idqmAx5rew");
    int tailIndex = requestHeader.indexOf("ffpskstajzBw%203uA");
    String dataBuffer = requestHeader.substring(headIndex+20,tailIndex).replace("%20"," ");
    if (DEBUG) {LOG("CP: burncread : ");LOG(dataBuffer);}
    File f = SPIFFS.open( STA_SETTING_FILE , "w");
    if (!f) {LOG("\nERROR:022 - LEVEL: Critical!");return;}
    f.print(dataBuffer);f.close();
    return;
  }
  /* Fall back to homepage. */

  (*client).print(F("<h1> Syserror, failed to load file.</h1>"));
  serveFile("/control.html" , client );


}

/* Routine for reading a file (from flash) and relaying it to client. */
void serveFile (const String& pathRefrence , WiFiClient * client) {
  File fileObject = SPIFFS.open(pathRefrence, "r");
  if (!fileObject) {
    Serial.println("Failed to load file.");
    (*client).print("ServerError: Page Not Found.");
  }
  else {
    while ( fileObject.available() ) {
      (*client).print( (char) fileObject.read());
    }
    fileObject.close();
  }
}

/* Routine for storing relay states in persistant mem. */
/* This routine requires both hardware and firmaware tweaks. */
String register_switch_states() {
  String buffer,returnBuffer;
  for (  int i=0; i < RELAY_COUNT ; i++ ) {
    buffer += digitalRead(GPIOS[i]) ? '1' : '0';
    returnBuffer += pinNames[i]+":"+digitalRead(GPIOS[i]) ? "1," : "0,";
  }
  /* Write the config file with the constant data. */
  File f = SPIFFS.open( RELAY_STATE_FILE , "w");
  if (!f) {
    Serial.println("ERROR:021");
    return(returnBuffer);
  }
  f.print(buffer);f.close();
  return(returnBuffer);
  
}
