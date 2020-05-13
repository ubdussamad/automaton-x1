/*
  EMDSUBSYSTEMS
  AutomatonX1 - Automation Series Product Routine
  Author: ubdussamad <ubdussamad@gmail.com>
  Ref: 27 NOV 2019
  Version: 0.3
  Rev: 2
  Coyright 2019 EMDSUBSYSTEMS
  


  We have GPIO -> 13 , 12 , 14 , 16 , 4 , 5 , 10 , 9
*/


#include <ESP8266WiFi.h>
#include <FS.h> // Used for writing the AP data to the flash memory so as to use it later and also to write the staes of GPOI busses.
#include <typeinfo>


#define TID 1000
#define MAX_RECONNECTION_WAIT_TIME 30//240 // Multiple of 0.5 seconds
#define GPIO_NUM 3

// If the currently stored IP isn't reachable, after the below timeout period
// the device would try to connect to it's default AP just once. This process will
// continue until the device gets a connection. 
#define CUSTOM_IP_TRIAL_TIMEOUT 30000//3000000 // Multiple of milliseconds ~5Mins

#ifndef STASSID
#define STASSID "Galaxy A50"
#define STAPSK  "zebrafamily"
#endif

String APSettingsFile = "/credentials.config";
String RelayStates = "/states.config";
String ssid = STASSID;
String password = STAPSK;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);


int GPIOS[8] = {4 , 5 , 14};// , 12 , 13 };

void setup() {
  
  // We begin at 115200 Bits/Sec Data/Baud rate.
  Serial.begin(115200);

  
  Serial.print(F("EMD SUBSYSTEMS"));
  Serial.print(F("\n-----------------------------------------"));
  Serial.print(F("\nHome Automaton Utility Prototype"));
  Serial.print(F("\nVersion 0.3"));
  Serial.print(F("\nRef: 27 Nov 18"));
  Serial.print(F("\nAuthor: Ubdussamad <ubdussamad@gmail.com>"));
  
  // Mounting the file system.
  if(SPIFFS.begin()){
  Serial.println("\nSPIFFS Initialize: Ok");}
  else{Serial.println("\nSPIFFS Initialization: Failed");}

  String data = "";

  // Checking if there is a file storing credentials data
  if ( !SPIFFS.exists(APSettingsFile) ) { 
    // Meaning this is the first run for this product then use the default value stored in the variables

    //Format File System
    //if(!SPIFFS.format()) { Serial.println("Filesystem format Failed.");}

    //Write the config file with the constant data
    File f = SPIFFS.open( APSettingsFile, "w");
    
    if (!f) {Serial.println("\nSettings Config File Creation Failed.");} // TODO: Write a Suite to recover from this. Maybe Restart??
    
    else {
        f.print( ssid + "%20" + password + "%21");
        f.close();  //Close file
    }

    
  }
  
  
  else {
    // Incase there is a file then read its content.
    Serial.println("\nReading AP-data stored in flash.");
    File f = SPIFFS.open(APSettingsFile, "r");
    if (!f) {
      Serial.println("\nError opening existing config file, Rverting to base SSID and Password.");
    }
    else {
      for(int i=0;i<f.size();i++) {
        data += (char)f.read();
      }
      
      f.close();
    }
  }

  if (data) {// Meaning file data was read
    
    String Credentials[2][2];
    int x = 0, y = 0; // X is credentials index and y is subcredentials index which separtes ssid and paswords.
    for ( int i=0; i < data.length(); i++ ) {
      if ( data[i] == '%' && data[i+1] == '2'){
        if ( data[i+2] == '0' ) {
          //Subcredential Break
          i+=3;
          //Serial.println("Subcredential Break");
          ++y;
        }
        else if ( data[i+2] == '1') {
          //Credentials Break Point
          //Serial.println("Credential Break");
          i+=3;
          ++x;
        }
      }
      Credentials[x][y] += data[i];
    }
  ssid = Credentials[0][0];
  password = Credentials[0][1];
  Serial.println("Sucessfully read data from internaly sotred file.");
  
  }

  File relay_states_f = SPIFFS.open( RelayStates, "r");
  String buff;
      if (!relay_states_f) {
        Serial.println("Failed to open Relay States File.");
      }

      else {
        Serial.println("Rely Sate Files present.");
       
        while ( relay_states_f.available()) {
       
           buff += (char) relay_states_f.read();
        }

        relay_states_f.close();
      }
  
  // Loop over GPIO pin array and keep setting pinmodes
  for (int i=0;i<GPIO_NUM;i++) {
    pinMode( GPIOS[i] , OUTPUT);
    digitalWrite( GPIOS[i] , (buff.length() > 0) ? ((buff[i] == '1') ? 1 : 0) : 0 );
  }

  // Connect to WiFi network
  Serial.print(F("\nConnecting to Host AP named: "));
  Serial.println(ssid);
  Serial.print(F("\nPlease Wait"));

  unsigned long initial_epoch = millis(); // Time of starting the initial connection trial
  // if there is no connection till CUSTOM_IP_TRIAL_TIMEOUT ,  then the module
  // will try to connect to the default AP once!. 
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if ( millis() - initial_epoch > (CUSTOM_IP_TRIAL_TIMEOUT) ){ // This could create some problems, due to overflow and stuff
      // Try connecting to the default AP once for 20 seconds then reboot.
      WiFi.begin((String)STASSID, (String)STAPSK);
      int i = 20;
      Serial.println("Timeout, Trying connecting to defalut AP..");
      while ( i && WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.println(".");
          i--;
      }
      if (WiFi.status() != WL_CONNECTED) { 
        // Reboot the chip
        ESP.restart();
      }
    
    }
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("\nStation Successfully connected to AP."));

  // Start the server
  server.begin();
  Serial.print(F("\nServer Running .. @"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

char pageRequest = 'h'; // By default we would like to show the main home page to the user.

void loop() {
  // Check if a client has connected
  
  if (WiFi.status() != WL_CONNECTED) {
      delay(2000);
      int time_delta = 0;
      Serial.print(F("\nRe-Connecting to Host AP named: "));
      Serial.println(ssid);
      Serial.print(F("\nPlease Wait"));
    
      //WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        if (time_delta++ > MAX_RECONNECTION_WAIT_TIME ) { // About 10 secs
          Serial.print(F("\n\n||||||||||||||RESTARTING MODULE|||||||||||||||||\n\n"));
          ESP.restart();
          break;
        }
        delay(500);
        Serial.print(F("."));
      }
      Serial.println();
      Serial.println(F("\nStation Successfully connected to AP."));

  }
  
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  //Serial.println(F("NEW Client PING"));
  
  client.setTimeout(1500); // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("_____________________"));
  //Serial.println(F("Request Type: "));
  //Serial.println(req);

  // Match the request
  if (req.indexOf(F("/stat")) != -1) {
    Serial.println("\nEMD Subsystems - AUTOMATON-X1\nPID: AX1\nDevice UID:");
    Serial.println(TID);
    pageRequest = 'j';
  }
  
  else if (req.indexOf(F("/gpio")) != -1) {
    // This is now supposed to toggle the state of the current switch
      Serial.println(F("\nGPIO Command"));
      for (int i=0; i<GPIO_NUM; i++) {
        if (req.indexOf( "/"+String(GPIOS[i])) !=-1) {
          // Toggle that line
          Serial.println( " Setting GPIO: " + String(GPIOS[i]) + "LOW" );
          digitalWrite( GPIOS[i] , !digitalRead(GPIOS[i]) );
          
        }
      }
      register_switch_states(); // Write the new switch states to flash
      pageRequest = 'j';
  }

  else if (req.indexOf(F("/restart ")) != -1) {
    Serial.println("Restarting the Switches....");
    delay(2000);
    ESP.restart();
  }
  
  else if (req.indexOf(F("/config ")) != -1) {
    // Display a new page for updating the wifi credentials
    Serial.println("Genric Settings Command");
    pageRequest = 'c'; // Change page to config 
    
  }
  else if (req.indexOf(F(".ico")) != -1) {
    // Blank IP Request
    //pageRequest = 'h';
  }

  else if (req.indexOf(F("/config/set_credentials")) != -1) {
    // Format of the request would be "/config/set_credentials/username/password"
    // If the password/username contains white spaces? it'll be here as %20
    // GET /config/set_credentials/Galaxy%20A50/something%20cool HTTP/1.1
    // Update Current Wifi Settings
    pageRequest = 'x'; // Change page to config;
    
    int psk_begin_index = req.lastIndexOf('/', req.lastIndexOf('/')-1 );
    int psk_end_index = req.lastIndexOf('H')-1;
    int ssid_begin_index = req.lastIndexOf('/' , psk_begin_index  - 1 );
    String n_ssid = req.substring(ssid_begin_index+1, psk_begin_index);
    String n_psk = req.substring(psk_begin_index+1 , psk_end_index);
    n_ssid.replace("%20", " ");
    n_psk.replace("%20", " ");

    if ( n_ssid.indexOf("set_credentials") != -1){
      pageRequest='h';
    }
    else {
      Serial.print("New AP Credentials!\n SSID: ");
      Serial.println(n_ssid);
      Serial.print("PSK: ");
      Serial.println(n_psk);
      File f = SPIFFS.open( APSettingsFile, "w");
      
      if (!f) {Serial.println("\nSettings Config File Creation Failed.");} // TODO: Write a Suite to recover from this. Maybe Restart??
      
      else {
          f.print( n_ssid + "%20" + n_psk + "%21");
          f.close();  //Close file
          Serial.println("Restarting Switch with new credentials, please wait....");
      }
      
    }
    
    
    
  }
  
  else {
    //Serial.println(F("Invalid Request: "));
    //Serial.println(F(req));
//    for (int i=0;i<GPIO_NUM;i++){
//      int parity = digitalRead(GPIOS[i]);
//      digitalWrite(GPIOS[i], parity);
//    }
      pageRequest = 'h';
  }

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  if ( pageRequest == 'h') { 
      client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
      ""
      "<!DOCTYPE HTML>\r\n<html>\r"
      "<head> <title> Automatonx1 - EMD Subsystems </title>"
      "<meta charset=\"utf-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
      "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/css/bootstrap.min.css\">"
      "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>"
      "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/js/bootstrap.min.js\"></script>"
      "</head>"
      "<body style=\"background: #F2F6DF;\">"
      "<br/>"
      "<h3 style=\"color:rgb(100,100,100);\" align=\"center\">EMD Subsystems</h3>"
      "<h4 style=\"color: #0D5EA5; \" align=\"center\">AutomatonX1</h4>"
      "<h5 style=\"color: #0D5EA5; \" align=\"center\">Automation Series Product Line</h5>"
      "<h5 align=\"center\">Author: ubdussamad <ubdussamad@gmail.com> </h5>"
      "<h5 align=\"center\">Ref: 27 NOV 2019 </h5>"
      "<div align=\"center\">"
      "<br/> <br/> <h5 align=\"center\"> Control Relay # </h4>"
      ));
    
      
      String IP = IpAddress2String(WiFi.localIP());
      for (int i=0;i<GPIO_NUM;i++) {
      char buff[10000];
      sprintf(buff,
      "<br/><a href='http://%s/gpio/%d1' class=\"btn btn-primary\" align=\"center\" role=\"button\" > ON-%d </a><a href='http://%s/gpio/%d0' class=\"btn btn-danger\" align=\"center\" role=\"button\" > OFF-%d </a>"
      , IP.c_str() , GPIOS[i] , GPIOS[i] , IP.c_str() , GPIOS[i] , GPIOS[i]);
      client.print(buff);
      
      }
      
      client.print(F(
      "</div>"
      "<br/><br/>"));
      char buff[1000];
      sprintf(buff,
      "<a href='http://%s/config' class=\"btn btn-warning\" align=\"center\" role=\"button\" > Config </a>"
      , IP.c_str());
      client.print(buff);
      client.print(F(
      "<footer style=\"background: rgb(100,100,100);color:white;\">"
      "<h10 align=\"center\"> © Copyright 2019 EMD Subsystems </h10>"
      "</footer>"
      "</body>"
      "</html>"));
  }

  else if ( pageRequest == 'c' ) { // Config Page
    // Print the config page stuf to the thing, ok??
      File config_page = SPIFFS.open("/settings.html", "r");
     
      if (!config_page) {
        Serial.println("Failed to open settings.html Page");
      }

      else {
        Serial.println("Success in reading data from file.");
        while ( config_page.available()) {
       
          client.print( (char) config_page.read());
        }

        config_page.close();
      }
     
  }

  else if ( pageRequest == 'j' ) { // Config Page
  String ids, logic;
  for (int i=0;i<GPIO_NUM;i++) {
      ids += String(GPIOS[i])+ ',';
      logic += digitalRead(GPIOS[i]) ? "1," : "0," ;
  }
    client.print(ids[:-1]+";"+logic[:-1]+";");
  }

  else if ( pageRequest == 'x') { //Post config page
        // Print the config page stuf to the thing, ok??
      File config_page = SPIFFS.open("/ack.html", "r");
     
      if (!config_page) {
        Serial.println("Failed to open settings.html Page");
      }

      else {
        Serial.println("Success in readnig data from file.");
       
        while ( config_page.available()) {
       
          client.print( (char) config_page.read());
        }

        config_page.close();
      }
     delay(10000);
     ESP.restart();
  }

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  // Serial.println(F("Disconnecting from client"));
}

String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void register_switch_states() {
  String buff = "000";
  
  for (  int i=0; i < GPIO_NUM ; i++ ) {
    buff[i] = digitalRead(GPIOS[i]) ? '1' : '0';
  }

  //Write the config file with the constant data
  File f = SPIFFS.open( RelayStates , "w");
    
  if (!f) {Serial.println("\nWriting Sate chages.");} // TODO: Write a Suite to recover from this. Maybe Restart??
    
  else {
        f.print(buff);
        f.close();  //Close file
    }
  
}
