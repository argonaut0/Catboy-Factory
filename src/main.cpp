#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define DATA_PIN            8
#define NUM_LEDS            18
#define MAX_POWER_MILLIAMPS 300
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60


#define LEFT_PIN 9
#define RIGHT_PIN 10
#define BAUDRATE 9600
#define BUTTON1 4
#define BUTTON2 2
#define BUTTON3 6
#define BUTTONLED 3


/*
  Globals
*/
Servo leftS;
Servo rightS;

CRGB leds[NUM_LEDS];
bool gReverseDirection = false;

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
  LED
*/

CRGBPalette16 pacifica_palette_1 = 
    { 0x005007, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };




// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}


// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

#define COOLING  60

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,200) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


/*
  Main
*/

void setup() {
  Serial.begin(BAUDRATE);
  leftS.attach(LEFT_PIN);
  rightS.attach(RIGHT_PIN);
  pinMode(BUTTON1, INPUT);
  digitalWrite(BUTTON1, HIGH);
  pinMode(BUTTON2, INPUT);
  digitalWrite(BUTTON2, HIGH);
  pinMode(BUTTON3, INPUT);
  digitalWrite(BUTTON3, HIGH);
  /*
  char cmd = ' ';
  while (cmd != 'i') {
    if (Serial.available()) {
      cmd = Serial.read();
    }
  }
  */
  Serial.println("Initialized");
  resetServos();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);
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

  if (digitalRead(BUTTON1) == LOW) {
    flatten();
  }
  if (digitalRead(BUTTON2) == LOW) {
    wiggle();
  }
  if (digitalRead(BUTTON3) == LOW) {
    vibrate();
  }

  EVERY_N_MILLISECONDS(70) {
    //pacifica_loop();
    Fire2012();
    leftS.detach();
    rightS.detach();
    FastLED.show();
    leftS.attach(LEFT_PIN);
    rightS.attach(RIGHT_PIN);
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