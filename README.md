# Automaton X1

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9d0d84b2bdd04276a86919ae32953484)](https://app.codacy.com/manual/ubdussamad/automaton-x1?utm_source=github.com&utm_medium=referral&utm_content=ubdussamad/automaton-x1&utm_campaign=Badge_Grade_Dashboard)

Codebase and Hardware layouts for the Automaton X1 Project

Copyright: 2019 Mohammmed S. <ubdussamad@gmail.com>

This repository is published under Apache License.


## Development History:

Last In use System:
Firmware: Version 0.3 / Rev 2 (Ref 27NOV19)
Hardware: Version 2 / Rev 0 (Ref 27NOV19)

Current In Development Version:
Firmware: Version 1 / Rev 03 (*Target*) (*Partial REF 24APR20*)
Hardware: Version 3 / Rev 0 (*Target*)

## NOTES [Checklist before Flashing]:

* The uC must have a relay states file initially set to 00000 (V1REV3)
* The uC also must have a Credentials File present set to default AP.(V1REV3)
* The uC must have a relay names file, set to relay numbers.

## Schemes:

* Control Page State data format: (Response data and not a file.) 

  (It'd be set to reduce uC's work.) (V1REV3)

  pin_id[ALPHA-NUMERIC/str]:current_state[NUMERIC/str],....

  [What kind of data/Which type it is.]
  
  The String at last terminates with a comma.
  The string starts with the pin_id num in str format

  Example:
  12:0,13:1,14:1,15:0,

* Custom Access Point Storage File format (File: credentials.config)

  STA_SSID[Newline]STA_PSK

  The PSK and SSID are seprated by a Simple Newline Character

  Note: This Format may cause problems if the SSID or PSK 
        would Contain a newline but it's highly unlikely.

  Example:
  hotspot_name
  password


* Relay State File Format ( File: states.config )

  [state][state][state]......
  state-> 1/0
  The order is set by the wrting application.

  Example: 1011011

* Format for Requests and Respective Responses:

  Query:
      ----> Response

  1: */stat :

    [Product Name] [NL]
    [Version&Revision] [NL]
    [Date Code][NL]
    [UID (Device Serial/ Unique ID)] [NL]
    [Control Page State Data]

  2: */gpio/[PIN_ID] :
    [Control Page State Data]

  3: */burncred....[cread_begin_identifier][(ssid)(\n)(psk)][cread_end_identifier]
    [ACK]
    NOTE: In SSID&PSK , White Spaces are replaced with %20 .


* PinNames/Relay Names File Format:

  [Pin Name],[Pin Name],.......

  Note: The file must have a comma at the end.


  Example:
  FAN,LIGHT,SOCKET_1,REACTOR_CORE,......


## Future Goals

* Impliment true relay control, by reading final hardware state data.
* Impliment Some sort of authentication for all actions.
* Impliment WAN based relay control intead of relying on just LAN.
