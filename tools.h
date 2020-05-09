
/* Prototypes for the tools which may or may not be included in production build. */

void connect_to_station (const String& SSID ,const String&  PSK );

void requestHandler ( const String& requestHeader , WiFiClient * client );

void serveFile (const String& pathRefrence , WiFiClient * client);

String register_switch_states(void);
