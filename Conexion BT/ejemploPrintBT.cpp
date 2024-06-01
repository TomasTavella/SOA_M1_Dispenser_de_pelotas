#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // RX, TX
 
char w;
 
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(1000); //wait to start the device properly
}
 
void loop() {
  if (mySerial.available()) {
    w = mySerial.read();
    Serial.println(w);  //PC
    delay(10);
    //commands with Serial.println(); show on pc serial monitor
  }
 
  if (Serial.available()) {
    w = Serial.read();
    mySerial.println(w); //Phone
    delay(10);
    //commands with mySerial.println(); show on the device app
  }
 
//shown on pc
Serial.println("This is a test run");
Serial.println("Congratulations! Device Connected!");
Serial.println();
 
//shown on device app
mySerial.println("This is a test run");
mySerial.println("Congratulations! Device Connected!");
mySerial.println();
 
delay(2000);
}