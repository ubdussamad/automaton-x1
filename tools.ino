/*
  Automaton X1 - Tooling
  EMD Subsystems
  Version : 1.0
  Rev: 3
  Ref: 24APR20
  Author: ubdussamad <ubdussamad@gmail.com>
*/

/* Routine for Connecting to a AP Station. */
void connect_to_station(const String &SSID, const String &PSK) {
  LOG(F("Connecting to custom AP: "));
  Serial.println(SSID);
  Serial.println("Please Wait..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSK);
  unsigned long initial_epoch = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - initial_epoch) > USER_AP_TRIAL_TIMEOUT) {
      WiFi.begin(DEFAULT_STA_SSID, DEFAULT_STA_PSK);
      Serial.println("Custom AP timeout, trying default..");
      while ((millis() - initial_epoch) >
                 (USER_AP_TRIAL_TIMEOUT + DEFAULT_AP_TRIAL_TIMEOUT) &&
             WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
      }
      if (WiFi.status() != WL_CONNECTED) {
        ESP.restart();
      }
    }
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("\nStation Successfully connected to AP."));
  return;
}

/* Routine for handling HTTP requests */
/* Client pointer is taken so print the data directly to prevent copying. */
void requestHandler(const String &requestHeader, WiFiClient *client) {
  /* Basic parsing of the request. */

  /* Most frequent request. */
  int index = requestHeader.indexOf(F("/gpio"));

  if (index > 0) {
    int relayId = requestHeader.substring(index + 5).toInt();
    digitalWrite(relayId, !digitalRead(relayId));
    (*client).print(register_switch_states());
    return;
  }

  if (requestHeader.indexOf(F("/stat")) > 0) {
    ((*client).print(STR(PRODUCT_NAME) + STR(VERSION) + STR(REF) + STR(UID) +
                     register_switch_states()));
    return;
  }

  if (requestHeader.indexOf(F("/restart")) > 0) {
    ESP.restart();
    return;
  }

  if (requestHeader.indexOf(F("/config")) > 0) {
    serveFile("/settings.html", client);
    return;
  }

  if (requestHeader.indexOf(F("/burncred")) > 0) {
    int headIndex = requestHeader.indexOf("afstass%32idqmAx5rew");
    int tailIndex = requestHeader.indexOf("ffpskstajzBw%903uA");
    String dataBuffer = requestHeader.substring(headIndex + 20, tailIndex);
    dataBuffer.replace("%20", " ");
    dataBuffer.replace("%60e", "\n");
    if (DEBUG) {
      LOG("CP: burncread : ");
      LOG(dataBuffer);
    }
    File f = SPIFFS.open(STA_SETTING_FILE, "w");
    if (!f) {
      LOG("\nERROR:022 - LEVEL: Critical!");
      return;
    }
    f.print(dataBuffer);
    f.close();
    return;
  }


  if (requestHeader.indexOf(F("/burnnames")) > 0){
    int epch = requestHeader.indexOf(F("/burnnames/"));
    String dataBuffer = requestHeader.substring(epch); // Maybe it might need the end index
    File f = SPIFFS.open(RELAY_NAME_FILE, "w");
    if (!f) {
      LOG("\nERROR:024 - LEVEL: Amber!");
      return;
    }
    f.print(dataBuffer);
    f.close();


  }

  /* Fall back to homepage. */

  (*client).print(F("<h1> Syserror, failed to load file.</h1>"));
  serveFile("/control.html", client);
}



/* Routine for reading a file (from flash) and relaying it to client. */
void serveFile(const String &pathRefrence, WiFiClient *client) {
  File fileObject = SPIFFS.open(pathRefrence, "r");
  if (!fileObject) {
    Serial.println("Failed to load file.");
    (*client).print("ServerError: Page Not Found.");
  } else {
    while (fileObject.available()) {
      (*client).print((char)fileObject.read());
    }
    fileObject.close();
  }
}

/* Routine for storing relay states in persistant mem. */
/* This routine requires both hardware and firmaware tweaks. */
String register_switch_states(void) {
  String buffer, returnBuffer;
  for (int i = 0; i < RELAY_COUNT; i++) {
    buffer += digitalRead(GPIOS[i]) ? '1' : '0';
    returnBuffer += String(GPIOS[i]) +"%xef"+ pinNames[i] + ":" + (digitalRead(GPIOS[i]) ? "1," : "0,");
  }
  /* Write the config file with the constant data. */
  File f = SPIFFS.open(RELAY_STATE_FILE, "w");
  if (!f) {
    Serial.println("ERROR:021");
    return (returnBuffer);
  }
  f.print(buffer);
  f.close();
  return (returnBuffer);
}
