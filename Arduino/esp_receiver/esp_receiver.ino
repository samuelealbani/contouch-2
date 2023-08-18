// ESP32 - Receiver

// ESP32 - Receiver
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <OSCMessage.h>

#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#define RXD2 16
#define TXD2 17

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

uint8_t touchSensitivity = 6;
uint8_t releaseSensitivity = 6;


const int magPin = 15;
int magState = 0;      // 0 close - 1 open switch
int prevMagState = 0;  // 0 close - 1 open switch

int value1;
int value2;
int value3;
int value4;

char ssid[] = "G11";        // your network SSID (name)
char pass[] = "Fb)M2}8ht";  // your network password

WiFiUDP Udp;                               // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(10, 100, 117, 166);  // remote IP of your computer
const unsigned int outPort = 3333;         // remote port to receive OSC


int fsrAnalogPin0 = 36;  // FSR is connected to analog 0
int fsrAnalogPin1 = 39;  // FSR is connected to analog 0
int fsrReading0;         // the analog reading from the FSR resistor divider
int fsrReading1;         // the analog reading from the FSR resistor divider



void setup() {
  // pinMode(magPin, INPUT_PULLUP);

  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  /// MPR121
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");
  cap.setThresholds(touchSensitivity, releaseSensitivity);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting WIFI to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // // magnetic
  // magState = digitalRead(magPin);
  // if (magState != prevMagState) {
  //   OSCMessage msgMag("/magnetic");
  //   msgMag.add(magState);
  //   Udp.beginPacket(outIp, outPort);
  //   msgMag.send(Udp);
  //   Udp.endPacket();
  //   msgMag.empty();

  //   prevMagState = magState;
  // }


  // pressure
  fsrReading0 = analogRead(fsrAnalogPin0);
  Serial.print("Analog reading = ");
  Serial.print(fsrReading0);

  fsrReading1 = analogRead(fsrAnalogPin1);
  Serial.print("\t");
  Serial.println(fsrReading1);
  OSCMessage msgPress("/pressure");
  msgPress.add(fsrReading0);
  msgPress.add(fsrReading1);
  Udp.beginPacket(outIp, outPort);
  msgPress.send(Udp);
  Udp.endPacket();
  msgPress.empty();

  // MPR121
  currtouched = cap.touched();
  for (uint8_t i = 0; i < 12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
      OSCMessage msg("/capacitives/digital");
      switch (i) {
        case 0:
          msg.add("/0");
          break;
        case 1:
          msg.add("/1");
          break;
        case 2:
          msg.add("/2");
          break;
        case 3:
          msg.add("/3");
          break;
        case 4:
          msg.add("/4");
          break;
        case 5:
          msg.add("/5");
          break;
        case 6:
          msg.add("/6");
          break;
        case 7:
          msg.add("/7");
          break;
        case 8:
          msg.add("/8");
          break;
        case 9:
          msg.add("/9");
          break;
        case 10:
          msg.add("/10");
          break;
        case 11:
          msg.add("/11");
          break;
      }
      msg.add(1);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();


      Serial.print(i);
      Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
      OSCMessage msg("/capacitives/digital");
      switch (i) {
        case 0:
          msg.add("/0");
          break;
        case 1:
          msg.add("/1");
          break;
        case 2:
          msg.add("/2");
          break;
        case 3:
          msg.add("/3");
          break;
        case 4:
          msg.add("/4");
          break;
        case 5:
          msg.add("/5");
          break;
        case 6:
          msg.add("/6");
          break;
        case 7:
          msg.add("/7");
          break;
        case 8:
          msg.add("/8");
          break;
        case 9:
          msg.add("/9");
          break;
        case 10:
          msg.add("/10");
          break;
        case 11:
          msg.add("/11");
          break;
      }
      msg.add(0);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();

      Serial.print(i);
      Serial.println(" released");
    }
  }

  // reset our state
  lasttouched = currtouched;


  // comment out this line for detailed data from the sensor!
  // return;

  // // debugging info, what
  // Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  // Serial.print("Filt: ");
  // for (uint8_t i=0; i<12; i++) {
  //   Serial.print(cap.filteredData(i)); Serial.print("\t");
  // }
  // Serial.println();
  // Serial.print("Base: ");
  // for (uint8_t i=0; i<12; i++) {
  //   Serial.print(cap.baselineData(i)); Serial.print("\t");
  // }
  // Serial.println();

  // nano sensors
  if (Serial2.available() >= 16) {  // Wait for all 20 bytes to be available
    byte data[16];
    Serial2.readBytes(data, sizeof(data));
    // Decode integer values from the byte array
    value1 = decodeInt(data, 0);
    value2 = decodeInt(data, 4);
    value3 = decodeInt(data, 8);
    value4 = decodeInt(data, 12);

    OSCMessage msg("/capacitives/analog");
    String text = "";
    msg.add(value1);
    msg.add(value2);
    msg.add(value3);
    msg.add(value4);

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();

    // Do something with the received values
    // For example, print them to the serial monitor
    Serial.print("Received values: ");
    Serial.print(value1);
    Serial.print(", ");
    Serial.print(value2);
    Serial.print(", ");
    Serial.print(value3);
    Serial.print(", ");
    Serial.println(value4);
  }

  delay(50);
}

int decodeInt(byte *buffer, int offset) {
  int value = 0;
  value |= ((int)buffer[offset]) & 0xFF;
  value |= (((int)buffer[offset + 1]) << 8) & 0xFF00;
  value |= (((int)buffer[offset + 2]) << 16) & 0xFF0000;
  value |= (((int)buffer[offset + 3]) << 24) & 0xFF000000;
  return value;
}
