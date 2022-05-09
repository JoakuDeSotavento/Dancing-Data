/*
  Este código es el encargado de enviar todo lo necesario desde el Arduino hasta el programa que distribuye la información en Processing

  Aquí configuraras los elementos como la dirección IP y el puerto desde el cual arduino envia y recibe los mensajes OSC

  Tambien podras configurar las entradas analógicas de Arduino dependiendo los sensores que conectes 
  
  
  NINA Firmware 1.2.1
  Requires the following libraries:
  WiFi by Arduino
  WiFiNINA by Arduino
  OSC by Adrian Freed, Yotam Mann
  Circular Buffer
  
*/
/*

  This example connects to an unencrypted Wifi network.
  Then it prints the  MAC address of the Wifi module,
  the IP address obtained, and other network details.

  created 13 July 2010
  by dlf (Metodo2 srl)
  modified 31 May 2012
  by Tom Igoe
*/

///////////////// Libraries
#include <SPI.h>
#include <OSCMessage.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <CircularBuffer.h>
#include <OSCBundle.h>


////////////////////////////// Configuración del WIFI al que se conecta
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h and don't publish it in any place
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


//////////////////// CONEXION, RECEPCIÓN Y TRANSMISIÓN DE MENSAJES UDP
WiFiUDP Udp;

/***************************************(｡◕‿◕｡)************************************************/
unsigned int localPort = 8888;      // local port to listen on
//the Arduino's IP
IPAddress ip(192, 168, 152, 61);
//destination IP
IPAddress outIp(192, 168, 152, 62);
const unsigned int outPort = 12000;
//const unsigned int outPort2 = 13000;
const unsigned int inPort = 888;

////////////////////////// CIRCULAR BUFFER Para el suavizado de la señal

// setup circular buffer (rolling window)

const int numberSamples = 10;

CircularBuffer<int, numberSamples> buffer; //la capacidad del buffer es numbersamples, cuyo valor previamente hemos fijado
CircularBuffer<int, numberSamples> buffer1;
CircularBuffer<int, numberSamples> buffer2;
CircularBuffer<int, numberSamples> buffer3;
CircularBuffer<int, numberSamples> buffer4;
CircularBuffer<int, numberSamples> buffer5;
CircularBuffer<int, numberSamples> buffer6;
CircularBuffer<int, numberSamples> buffer7;

/////////////*CALIBRATION ROUTINE VARIABLES (EN PROCESSO) */////////////////

// variables:

//const int sensorPin = A0;    // pin that the sensor is attached to
int senVal[] = {0, 0, 0, 0, 0, 0, 0, 0};
int senMin[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023};
int senMax[] = {0, 0, 0, 0, 0, 0, 0};
const int senPin[] = {A0, A1, A2, A3, A4, A5, A6};

///////////////////////// SETUP
void setup() {
  Serial.begin(9600);

  // Revisando el módulo WIFI
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // Aquí se le asigna la WIFI especifica que uno desea, se configura arriba
  WiFi.config(ip);

  // Intentos para conectarse a la WIFI
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

// si se conecta a la wifi se enciende el led del Arduino
  if (status == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  // Ya que te has conectado imprime estos datos
  
  Serial.println("********(｡◕‿◕｡)*********");
  Serial.println("Has logrado conectarte a la MATRIX");
  printCurrentNet();
  printWifiData();
  Udp.begin(localPort);
  // Si quieres puedes enviar paquetes a diferentes IP's
  // Udp2.begin(localPort);
  // La rutina de calibración esta en proceso de implementacion
  //calibRoutine();
}

///////////////////////////////////////////////////////////////////Loop function
void loop() {

  for (int j = 0; j < 6; j++) {
    senVal[j] = analogRead(senPin[j]);
    senVal[j] = map(senVal[j], senMin[j], senMax[j], 0, 1023);
  }

  int sensorValue = analogRead(A0);
  int sensorValue1 = analogRead(A1);
  int sensorValue2 = analogRead(A2);
  int sensorValue3 = analogRead(A3);
  int sensorValue4 = analogRead(A4);
  int sensorValue5 = analogRead(A5);
  int sensorValue6 = analogRead(A6);

  ///// building a bundle
  buffer.unshift(sensorValue);
  buffer1.unshift(sensorValue1);
  buffer2.unshift(sensorValue2);
  buffer3.unshift(sensorValue3);
  buffer4.unshift(sensorValue4);
  buffer5.unshift(sensorValue5);
  buffer6.unshift(sensorValue6);

  int s = 0;
  int s1 = 0;
  int s2 = 0;
  int s3 = 0;
  int s4 = 0;
  int s5 = 0;
  int s6 = 0;

  for (int i = 0; i <= buffer.size(); i++) {

    s += buffer[i];
    s1 += buffer1[i];
    s2 += buffer2[i];
    s3 += buffer3[i];
    s4 += buffer4[i];
    s5 += buffer5[i];
    s6 += buffer6[i];
  }

  s = s / buffer.size();
  s1 = s1 / buffer1.size();
  s2 = s2 / buffer2.size();
  s3 = s3 / buffer3.size();
  s4 = s4 / buffer4.size();
  s5 = s5 / buffer5.size();
  s6 = s6 / buffer6.size();

  OSCBundle bndl;
  bndl.add("/analog/0").add((int32_t)s);
  bndl.add("/analog/1").add((int32_t)s1);
  bndl.add("/analog/2").add((int32_t)s2);
  bndl.add("/analog/3").add((int32_t)s3);
  bndl.add("/analog/4").add((int32_t)s4);
  bndl.add("/analog/5").add((int32_t)s5);
  bndl.add("/analog/6").add((int32_t)s6);

  ///////////////////////////// Need to put the IP directly VERY IMPORTANT  ////////////////////////////////////////
  Udp.beginPacket(outIp, outPort);
  bndl.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  bndl.empty(); // free space occupied by message

  //  OSCMessage msg("/analog/0");
  //  msg.add(String((int32_t)analogRead(A0), DEC));
  //the message wants an OSC address as first argument
  //msg.add((int32_t)analogRead(A0));
  //Serial.println((int32_t)analogRead(0));

  //  Udp.beginPacket("192.168.3.69", outPort2);
  //  msg.send(Udp); // send the bytes to the SLIP stream
  //  Udp.endPacket(); // mark the end of the OSC Packet
  //  msg.empty(); // free space occupied by message

  //* For the arduino to initiate the calibration process*//
  //aquí estoy tratabdo de meter la calibracion via processing
  OSCMessage msg("/calib/");
  int size;

  if ( (size = Udp.parsePacket()) > 0)
  {
    Serial.println("llego un mesaje de processing");
    while (size--) {
      msg.fill(Udp.read());
      Serial.println("llenando la variable");
      if (msg.isInt(1)) {
        //get that integer
        int data = msg.getInt(1);
        Serial.println(data);
      }
    }
    //if (!bundleIN.hasError())
    //  bundleIN.route("/calib", 0);
  }
  delay(20);
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void calibRoutine() {

  digitalWrite(LED_BUILTIN, LOW);
  // calibrate during the first five seconds
  while (millis() < 5000) {

    for (int i = 0; i < 6; i++) {
      senVal[i] = analogRead(senPin[i]);
      if (senVal[i] > senMax[i]) {
        senMax[i] = senVal[i];
      }
      if (senVal[i] > senMin[i]) {
        senMin[i] = senVal[i];
      }
    }
  }
  // signal the end of the calibration period
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Calibration Donde, lets make some noise");
}
