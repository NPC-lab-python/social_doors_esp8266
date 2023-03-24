#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// NDE fixe
IPAddress server(192,168,0,220);

const char * SSID ="maison";
const char * pass = "galileo@9493!";

// NDE fixe
IPAddress ip_device(192,168,0,115);

IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
//IPAddress dns(192,168,0,1);
String Hostname_wifi = "002PS01";

String clientId = "002PS01";
String id_mqtt = "cage1_porte_social1";
String pass_mqtt = "steve";

// OTA
String HostnameOTA = "002PS01";
String PasswordOTA = "steve";


