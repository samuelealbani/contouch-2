#include <CapacitiveSensor.h>

// int value1 = 123;
// int value2 = -456;
// int value3 = 7890;
// int value4 = -1234;

bool isDebugging = false;

CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2);    // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor cs_4_5 = CapacitiveSensor(4, 5);    // 10 megohm resistor between pins 7 & 5, pin 5 is sensor pin, add wire, foil
CapacitiveSensor cs_4_8 = CapacitiveSensor(4, 8);    // 10 megohm resistor between pins 9 & 8, pin 8 is sensor pin, add wire, foil
CapacitiveSensor cs_4_10 = CapacitiveSensor(4, 10);  // 10 megohm resistor between pins 9 & 8, pin 8 is sensor pin, add wire, foil

void setup() {
  // put your setup code here, to run once:
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);  // turn off autocalibrate on channel 1 - just as an example

  // wait the esp32 for waking up
  for (int i = 0; i < 2; i++) {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  long start = millis();
  long value1 = cs_4_2.capacitiveSensor(30);
  long value2 = cs_4_5.capacitiveSensor(30);
  // long value3 = cs_4_8.capacitiveSensor(30);
  long value3 = 0;
  long value4 = cs_4_10.capacitiveSensor(30);

  if (!isDebugging) {
    String msg = "<";
    msg.concat(String(value1));
    msg.concat("," + String(value2));
    msg.concat("," + String(value3));
    msg.concat("," + String(value4));
    msg.concat(">");

    writeString(msg);
  } else {
    // Serial.print(millis() - start);  // check on performance in milliseconds

    Serial.print("\t");    // tab character for debug window spacing
    Serial.print(value1);  // print sensor output 1

    Serial.print("\t");    // tab character for debug window spacing
    Serial.print(value2);  // print sensor output 2
    //                        //
    Serial.print("\t");    // tab character for debug window spacing
    Serial.print(value3);  // print sensor output 3

    Serial.print("\t");    // tab character for debug window spacing
    Serial.print(value4);  // print sensor output 4

    Serial.print("\n");
  }

  delay(100);  // arbitrary delay to limit data to serial port
}


void writeString(String stringData) {  // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++) {
    Serial.write(stringData[i]);  // Push each char 1 by 1 on each loop pass
  }

}  // end writeString
