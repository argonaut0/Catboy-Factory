#include <Arduino.h>
#include <Servo.h>

#define LEFT_PIN 9
#define RIGHT_PIN 10

Servo leftS;
Servo rightS;

int rangle(int a) {
  return 180 - a;
}

void writeSame(int a) {
  leftS.write(a);
  rightS.write(rangle(a));
}

void resetServos() {
  writeSame(90);
  delay(30);
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

void setup() {
  leftS.attach(LEFT_PIN);
  rightS.attach(RIGHT_PIN);
  // Your code here:
  resetServos();
  flatten();
  wiggle();
  vibrate();
}

void loop() {
}