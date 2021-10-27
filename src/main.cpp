#include <Arduino.h>
#include <Servo.h>

#define LEFT_PIN 9
#define RIGHT_PIN 10
#define BAUDRATE 9600


/*
  Globals
*/
Servo leftS;
Servo rightS;


/*
  Helpers
*/

int rangle(int a) {
  return 180 - a;
}

void writeSame(int a) {
  leftS.write(a);
  rightS.write(rangle(a));
}

void resetServos() {
  writeSame(90);
  delay(200);
}

// Flattens ears, waits for t milliseconds, then vibrates them up. 
void flatten(int t = 2500) {
  writeSame(0);
  delay(t);
  for (int pos = 5; pos <= 90; pos++ ) {
    writeSame(pos-5);
    delay(30);
    writeSame(pos+5);
    delay(30);
  }
}

// Wiggles ears n times, with angle a, with dt millis between each motion
void wiggle(int n = 3, int a = 20, int dt = 200) {
  resetServos();
  for (int i = 0; i < n; i++) {
    leftS.write(90-a);
    rightS.write(rangle(90+a));
    delay(dt);
    leftS.write(90+a);
    rightS.write(rangle(90-a));
    delay(dt);
  }
  resetServos();
}

// Vibrates ears
void vibrate() {
  wiggle(20, 20, 40);
}


/*
  Main
*/

void setup() {
  Serial.begin(BAUDRATE);
  leftS.attach(LEFT_PIN);
  rightS.attach(RIGHT_PIN);
  resetServos();
  // Your code here:
}

void loop() {
  
  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'w':
        wiggle();
        break;
      case 'v':
        vibrate();
        break;
      case 'f':
        flatten();
      case 'r':
      default:
        resetServos();
        break;
    }
  }
}

/*
  Serial Protocol:
  ---------------------
  Need to encode:
  - 181 degrees * 2 ears
  - 8 ish animations
  - 256 intensities * 3 colors * 18
  1 byte leftS
  1 byte rightS
  1 byte other
  3 bytes * 18 LEDs


*/