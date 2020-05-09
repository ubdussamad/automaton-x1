/*
  Automaton X1 - Configrations 
  EMD Subsystems
  Version : 1.0
  Rev: 3
  Ref: 24APR20
  Author: ubdussamad <ubdussamad@gmail.com>
*/
#pragma once
/* CycleVersion */
#if !defined(CYCLE_VERSION)
#define CYCLE_VERSION "5R6"
#endif // CYCLE_VERSION

/* PRODUCT CONFIG BLOCK */
#define PRODUCT_NAME "AutomatonX1\n"
#define VERSION "V01REV03\n"
#define REF "24APR20\n"
#define UID "002\n"


#define RELAY_COUNT 3
int GPIOS[RELAY_COUNT] = {4 , 5 , 14};// , 12 , 13 };


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
