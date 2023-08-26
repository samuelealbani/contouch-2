/*
  Multiple Serial test

  Receives from the main serial port, sends to the others.
  Receives from serial port 1, sends to the main serial (Serial 0).

  This example works only with boards with more than one serial like Arduino Mega, Due, Zero etc.

  The circuit:
  - any serial device attached to Serial port 1
  - Serial Monitor open on Serial port 0

  created 30 Dec 2008
  modified 20 May 2012
  by Tom Igoe & Jed Roach
  modified 27 Nov 2015
  by Arturo Guadalupi

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/MultiSerialMega
*/
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

#define RXD2 16
#define TXD2 17

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];  // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = { 0 };
int integerFromPC = 0;
float floatFromPC = 0.0;

boolean newData = false;

int value1;
int value2;
int value3;
int value4;

char ssid[] = "G11";        // your network SSID (name)
char pass[] = "Fb)M2}8ht";  // your network password

WiFiUDP Udp;  // A UDP instance to let us send and receive packets over UDP
WiFiUDP UdpOnSerial;
const IPAddress outIp(10, 100, 117, 166);  // remote IP of your computer
const unsigned int outPort = 3333;         // remote port to receive OSC


/// MPR121
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

uint8_t touchSensitivity = 8;
uint8_t releaseSensitivity = 4;

int fsrAnalogPin0 = 36;  // FSR is connected to analog 0
int fsrAnalogPin1 = 39;  // FSR is connected to analog 0
int fsrReading0;         // the analog reading from the FSR resistor divider
int fsrReading1;         // the analog reading from the FSR resistor divider


void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  /// MPR121
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  cap.setThresholds(touchSensitivity, releaseSensitivity);
  Serial.println("MPR121 found!");
}

void loop() {
  readSendMPR121();

  readSendPressure();

  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    showParsedData();
    sendOSCParseData();



    newData = false;
  }
}

//============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial2.available() > 0 && newData == false) {
    rc = Serial2.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//============

void parseData() {  // split the data into its parts

  char* strtokIndx;  // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");  // get the first part - the first integer
  value1 = atoi(strtokIndx);            // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  value2 = atoi(strtokIndx);            // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  value3 = atoi(strtokIndx);            // convert this part to an integer
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  value4 = atoi(strtokIndx);            // convert this part to an integer
}
//============

void showParsedData() {

  Serial.print(value1);
  Serial.print("\t ");
  Serial.print(value1);
  Serial.print("\t ");
  Serial.print(value3);
  Serial.print("\t ");
  Serial.print(value4);
  Serial.println();
}

//============

void sendOSCParseData() {
  // if (value1 > 100000 || value2 > 100000 || value3 > 100000 || value4 > 100000) {
  //   Serial.println("Restarting ESP32 due to large value");
  //   ESP.restart();  // Restart the ESP32
  // }

  OSCMessage msgOnSerial("/jack0/capacitives/analog");
  String text = "";
  msgOnSerial.add(value1);
  msgOnSerial.add(value2);
  msgOnSerial.add(value3);
  msgOnSerial.add(value4);

  UdpOnSerial.beginPacket(outIp, outPort);
  msgOnSerial.send(UdpOnSerial);
  UdpOnSerial.endPacket();
  msgOnSerial.empty();
}

//============

void readSendMPR121() {
  currtouched = cap.touched();
  for (uint8_t i = 0; i < 12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
      OSCMessage msg("/jack0/capacitives/digital");
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
      OSCMessage msg("/jack0/capacitives/digital");
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
}

//============

void readSendPressure() {
  fsrReading0 = analogRead(fsrAnalogPin0);
  fsrReading1 = analogRead(fsrAnalogPin1);

  Serial.print("Analog reading = ");
  Serial.print(fsrReading0);
  Serial.print("\t");
  Serial.println(fsrReading1);

  OSCMessage msgPress("/jack0/pressure");
  msgPress.add(fsrReading0);
  msgPress.add(fsrReading1);
  Udp.beginPacket(outIp, outPort);
  msgPress.send(Udp);
  Udp.endPacket();
  msgPress.empty();
}
