/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "ESP8266WiFi.h"
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>

SoftwareSerial mySerial(13,15); //RX TX

WiFiClient espClient;
PubSubClient client(espClient);

// Put in here the IP Adress of your MQTT Broker
IPAddress mqtt_broker(192, 168, 88, 207);
long lastReconnectAttempt = 0;

// Put in here the credentials for the MQTT Broker
const char* mqtt_user = "user";
const char* mqtt_pass = "pass";

// Put in here your WIFI SSID/Password
const char* SSID = "YOUR_WIFI";
const char* PSK = "WIFI_PASS";


const int BUFFER_SIZE = 282;
byte buf[BUFFER_SIZE];

unsigned long exec_time = 0;


// Put in here the key you received from NETZ NÖ in HEX-Notation
byte evn_schluessel []= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
byte frame [254];
byte frameCounter [4];
byte systemTitel [8];
byte init_vector [12];
byte apdu [254];

/** Variables for extraction of data **/

float wirkenergie_plus = 0.0;
float wirkenergie_minus = 0.0;
float momentanleistung_plus = 0.0;
float momentanleistung_minus = 0.0;
float spannung_l1 = 0.0;
float spannung_l2 = 0.0;
float spannung_l3 = 0.0;
float strom_l1 = 0.0;
float strom_l2 = 0.0;
float strom_l3 = 0.0;
float leistungsfaktor = 0.0;

// Object for decryption
GCM<AES128> gcmaes128;


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}


boolean reconnect() {
  if (client.connect("arduinoClient",mqtt_user,mqtt_pass)) {/**Serial.println("Connected to MQTT-Broker!");**/}
  return client.connected();
}

void slice_array(byte array_from[], byte array_to[],int start, int stop)
{
  int new_length = stop-start;
  for (int i = 0; i <= new_length; i++){
    array_to[i] = array_from[start+i];
  }
}

void combine_array(byte array_1[],byte array_2[], size_t length_array_1, size_t length_array_2, byte array_to[])
{
  for (size_t i = 0; i < length_array_1; i++){
    array_to[i] = array_1[i];
  }
  for (size_t i = 0; i < length_array_2; i++){
    array_to[length_array_1+i] = array_2[i];
  }
}

float return_scaled_value_32(byte apdu [], int pointer_start)
{
  return ((float)((uint32_t) apdu[pointer_start+3] | (apdu[pointer_start+2] << 8) | (apdu[pointer_start+1] << 16) | (apdu[pointer_start] << 24)))*pow(10,((int8_t) apdu[pointer_start+7]));
}
 
float return_scaled_value_16(byte apdu [], int pointer_start)
{
  return ((float)((uint16_t) apdu[pointer_start+1] | (apdu[pointer_start] << 8) ))*pow(10,((int8_t) apdu[pointer_start+5]));
}

void setup()
{

  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  mySerial.begin(2400);

  Serial.println("Connect! - Conexion ");
  // set the data rate for the SoftwareSerial port
  //mySerial.begin(9600);


  WiFi.begin(SSID, PSK);

  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
  }

  Serial.println(WiFi.localIP());

  // Für MQTT
  client.setServer(mqtt_broker, 1883);
  client.setCallback(mqtt_callback);
  client.setBufferSize(4096);

}
 
void loop() // run over and over
{
if (mySerial.available())
{
  String eingabe;

  int rlen = 0;
  if (mySerial.available() > 0) {
    // read the incoming bytes:
    rlen = mySerial.readBytes(buf, BUFFER_SIZE);

    // prints the received data
    Serial.print("I received: ");
    for(int i = 0; i < rlen; i++) {
      Serial.print(buf[i],HEX);
      eingabe += printf("%02x",buf[i]);
      }
  }

  exec_time = millis();
  slice_array(buf, systemTitel,11,18);
  slice_array(buf, frameCounter,22,25);
  slice_array(buf, frame,26,279);

  combine_array(systemTitel,frameCounter,sizeof(systemTitel), sizeof(frameCounter), init_vector);

  gcmaes128.setKey(evn_schluessel, sizeof(evn_schluessel));
  gcmaes128.setIV(init_vector, sizeof(init_vector));
  gcmaes128.decrypt(apdu, frame, sizeof(frame));

  wirkenergie_plus = return_scaled_value_32(apdu, 43);
  wirkenergie_minus = return_scaled_value_32(apdu, 62);
  momentanleistung_plus = return_scaled_value_32(apdu, 81);
  momentanleistung_minus = return_scaled_value_32(apdu, 100);
  spannung_l1 = return_scaled_value_16(apdu, 119);
  spannung_l2 = return_scaled_value_16(apdu, 136);
  spannung_l3 = return_scaled_value_16(apdu, 153);
  strom_l1 = return_scaled_value_16(apdu, 170);
  strom_l2 = return_scaled_value_16(apdu, 187);
  strom_l3 = return_scaled_value_16(apdu, 204);
  leistungsfaktor = return_scaled_value_16(apdu, 221);

  Serial.print("\n--------------\n");
  Serial.print("Wirkenergie+\t");Serial.print(wirkenergie_plus/1000.);Serial.println("\tkWh");
  Serial.print("Wirkenergie-\t");Serial.print(wirkenergie_minus/1000.);Serial.println("\tkWh");
  Serial.print("momentanleistung+\t");Serial.print(momentanleistung_plus);Serial.println("\tW");
  Serial.print("momentanleistung-\t");Serial.print(momentanleistung_minus);Serial.println("\tW");
  Serial.print("spannung_l1\t");Serial.print(spannung_l1);Serial.println("\tV");
  Serial.print("spannung_l2\t");Serial.print(spannung_l2);Serial.println("\tV");
  Serial.print("spannung_l3\t");Serial.print(spannung_l3);Serial.println("\tV");
  Serial.print("strom_l1\t");Serial.print(strom_l1);Serial.println("\tA");
  Serial.print("strom_l2\t");Serial.print(strom_l2);Serial.println("\tA");
  Serial.print("strom_l3\t");Serial.print(strom_l3);Serial.println("\tA");
  Serial.print("leistungsfaktor\t");Serial.print(leistungsfaktor);Serial.println("\t");


  //MQTT Reconnection
  if (!client.connected()) {
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      // Client connected
      client.loop();
    }

  //Serial.print("Status MQTT-Verbindung: ");
  //Serial.println(client.connected());   

  bool publish_sucess;
  if(client.connected()){
    publish_sucess = client.publish("/haus/heizung/test",buf,rlen);
    publish_sucess = client.publish("/haus/stromzaehler_oben/wirkenergie_plus",String(wirkenergie_plus).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/wirkenergie_minus",String(wirkenergie_minus).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/momentanleistung_plus",String(momentanleistung_plus).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/momentanleistung_minus",String(momentanleistung_minus).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/spannung_l1",String(spannung_l1).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/spannung_l2",String(spannung_l2).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/spannung_l3",String(spannung_l3).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/strom_l1",String(strom_l1).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/strom_l2",String(strom_l2).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/strom_l3",String(strom_l3).c_str());
    publish_sucess = client.publish("/haus/stromzaehler_oben/leistungsfaktor",String(leistungsfaktor).c_str());
    
  }
  /**Serial.print("----------------------");
  Serial.print("Publish sucess:    ");
  Serial.print(publish_sucess);
    Serial.println("----------------------");**/

  Serial.print("Exec time: ");
  Serial.println(millis()-exec_time);
  delay(500);
}
}