#include <Arduino.h>
#include <string>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <settings.h>
#include <Servo.h>

WiFiClient porte;
PubSubClient MQTTclient(porte);
Servo PorteSocial;
// #define START_DEGREE_VALUE 0 // The degree value written to the servo at time of attach.

void onConnected(const WiFiEventStationModeConnected &event);
void onGotIP(const WiFiEventStationModeGotIP &event);
void callback(char *topic, byte *payload, unsigned int length);

void MQTTreceive();
void MQTTconnect();
void moving();
void initmoving();

bool need_to_send_ack = false;

const int led_connect_broker = 13;  // d7
const int led_connection_wifi = 15; // d8
const int servopin = 12;            // D6
int angle = 0;

void setup()
{

    Serial.begin(9600);
    Serial.println("hello");
    // nom de l'objet OTA
    ArduinoOTA.setHostname(HostnameOTA.c_str());
    ArduinoOTA.setPassword(PasswordOTA.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.softAP("porte social");
    // config ip fixe
    WiFi.config(ip_device, gateway, subnet);
    WiFi.hostname(Hostname_wifi.c_str());
    WiFi.begin(SSID, pass);

    // démarrage de la connection ; création d'un event pour la connection
    static WiFiEventHandler onConnectionHandler = WiFi.onStationModeConnected(onConnected);
    static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);

    pinMode(led_connection_wifi, OUTPUT);
    pinMode(led_connect_broker, OUTPUT);
    MQTTclient.setServer(server, 1883);
    MQTTclient.setCallback(callback);
    MQTTclient.setKeepAlive(20);

    Serial.print(F("Attach servo at pin "));
    Serial.println(servopin);
    PorteSocial.attach(servopin);
    initmoving();
}

void loop()
{
    // connect serveur MQTT
    if (!MQTTclient.connected())
    {
        MQTTconnect();
    }

    if (MQTTclient.loop())
    {
    }
    else
    {
        Serial.println("Déconnecté");
    }

    // televersement à distance
    ArduinoOTA.handle();
}
void initmoving()
{
    Serial.println("move");

    // move from 180 to 0 degrees with a negative angle of 5
    for (angle = 90; angle >= 1; angle -= 1)
    {
        PorteSocial.write(angle);
        delay(100);
    }

    delay(1000);
    Serial.println("end move");
}

void moving()
{
    Serial.println("move");

    for (angle = 5; angle < 140; angle += 1)
    {
        PorteSocial.write(angle);
        delay(50);
    }
    Serial.println("wait 10 sec");

    delay(10000);

    // move from 180 to 0 degrees with a negative angle of 5
    for (angle = 140; angle >= 5; angle -= 1)
    {
        PorteSocial.write(angle);
        delay(50);
    }

    delay(1000);
    Serial.println("end move");
    if (!MQTTclient.connected())
    {
        Serial.println("client not connected");
        need_to_send_ack = true;
    }
    else
    {
        MQTTclient.publish("002/doors/002PS01/", "OK", true);
        need_to_send_ack = false;
        Serial.println("client connected");
    }
}

void onConnected(const WiFiEventStationModeConnected &event)
{
    if (WiFi.isConnected())
    {
        Serial.println("Wifi connecté");
        ArduinoOTA.begin();
        digitalWrite(led_connection_wifi, HIGH);
    }
    else
    {
        Serial.println("Not Connected");
    }
}

void onGotIP(const WiFiEventStationModeGotIP &event)
{
    Serial.println("Connection Ip : " + WiFi.localIP().toString());
    Serial.println("puissance du signal : ");
    Serial.println(WiFi.RSSI());
}

void MQTTreceive()
{
    MQTTclient.subscribe("002/doors/002PS01/set", 1);
    MQTTclient.publish("devices/connected/002PS01", "True", true);
    MQTTclient.publish("devices/list/002PS01", "002/doors/002PS01/", true);
}

void MQTTconnect()
{
    while (!MQTTclient.connected())
    {
        Serial.print("Attente  MQTT connection...");
        Serial.println(clientId.c_str());

        if (MQTTclient.connect(clientId.c_str(), "devices/connected/002PS01", 1, false, "Dirty"))
        {
            Serial.println("connecté au server MQTT");
            Serial.println(server.toString());
            digitalWrite(led_connect_broker, HIGH);

            if (need_to_send_ack)
            {
                Serial.println("SEND DOOR ACK");
                MQTTclient.publish("002/doors/002PS01/", "OK", true);
                need_to_send_ack = false;
            }
            MQTTreceive();
        }

        else
        { // si echec affichage erreur
            Serial.print("ECHEC, rc=");
            Serial.print(MQTTclient.state());
            Serial.println(" nouvelle tentative dans 5 secondes");
            digitalWrite(led_connect_broker, LOW);
            delay(5000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{

    payload[length] = '\0';
    String message((char *)payload);

    Serial.println(message);
    Serial.println(topic);

    if (strcmp(topic, "002/doors/002PS01/set") == 0)
    {
        Serial.println("Dans topic 002/doors/002PS01/set");
        if ((char)payload[0] == '1')
        {
            moving();
        }
    }
}