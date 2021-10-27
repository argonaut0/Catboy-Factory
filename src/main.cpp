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
void flatten(int t = 1000) {
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
void wiggle(int n = 1, int a = 20, int dt = 200) {
  for (int i = 0; i < n; i++) {
    signed int neg = 0;
    while (!neg) {
      neg = random(-1,2);
    }
    leftS.write(90-(a*neg));
    rightS.write(rangle(90+(a*neg)));
    delay(dt);
    leftS.write(90);
    rightS.write(rangle(90));
  }
}

// Vibrates ears
void vibrate() {
  for (int i = 0; i < 10; i++) {
    leftS.write(90-20);
    rightS.write(rangle(90+20));
    delay(20);
    leftS.write(90+20);
    rightS.write(rangle(90-20));
    delay(20);
    writeSame(90);
    delay(20);
  }
}


/*
  Main
*/

void setup() {
  Serial.begin(BAUDRATE);
  leftS.attach(LEFT_PIN);
  rightS.attach(RIGHT_PIN);
  char cmd = ' ';
  while (cmd != 'i') {
    if (Serial.available()) {
      cmd = Serial.read();
    }
  }
  Serial.println("Initialized");
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