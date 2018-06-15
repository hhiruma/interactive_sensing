#include <Servo.h>
 
Servo myServo; // create a servo object
Servo myServo2; 
 
int const potPin = A0; // analog pin used to connect the potentiometer
int const potPin2 = A1; 
int potVal; // variable to read the value from the analog pin
int potVal2;
int angle; // variable to hold the angle for the servo motor
int angle2;
 
void setup() {
myServo.attach(9); // attaches the servo on pin 9 to the servo object
myServo2.attach(8);
Serial.begin(9600); // open a serial connection to your computer
}
 
void loop() {
potVal = analogRead(potPin); // read the value of the potentiometer
potVal2 = analogRead(potPin2);
// print out the value to the serial monitor
Serial.print("potVal: ");
Serial.print(potVal);
 
// scale the numbers from the pot
angle = map(potVal, 0, 1023, 0, 179);
angle2 = map(potVal2, 0, 1023, 0, 179);
// print out the angle for the servo motor
Serial.print(", angle: ");
Serial.println(angle);

// set the servo position
myServo.write(angle);
myServo2.write(angle2);
// wait for the servo to get there
delay(15);
}
