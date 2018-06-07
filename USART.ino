#include <EEPROM.h>

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2

#define EMPTY 0
#define PLACED_BLOCK_PIXEL 1
#define PIXEL 2
#define SCORE_PIXEL 3
#define CORNER_PIXEL_ABSENT 4
#define CORNER_PIXEL_PRESENT 5
#define CURRENT_BLOCK_PIXEL 6
#define CORNER_PIXEL_ABSENT_PLACED 7
#define END_MSG 8
// Last parameter = 'true' enables double-buffering, for flicker-free,
// buttery smooth animation.  Note that NOTHING WILL SHOW ON THE DISPLAY
// until the first call to swapBuffers().  This is normal.
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);
// Double-buffered mode consumes nearly all the RAM available on the
// Arduino Uno -- only a handful of free bytes remain.  Even the
// following string needs to go in PROGMEM:

int receivedData = 0;
void myFunction ();
void myClear();
void clearScreen();

const int WIDTH = 32;
const int HEIGHT = 16;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  matrix.begin();
}

void loop() {
  static int prefix = 0;
  static int count = 0;
  static int x = 0;
  static int y = 0;
  static int screen[32][16];
  // put your main code here, to run repeatedly:

  //if there's no prefix currently set, then I am waiting for one
  if (prefix == 0) {
    if (Serial.available()) {
      receivedData = Serial.read();
      prefix = receivedData;
      Serial.print("FIRST PREFIX:");
      Serial.println(prefix);
    }
  } else if (prefix == 1) {    
      if (Serial.available()) {
        receivedData = Serial.read();
        //Serial.println(receivedData);
//         if (receivedData == EMPTY) {
//          matrix.drawPixel(x, y, matrix.Color333(0, 0, 0)); //off
//         } else if (receivedData == PLACED_BLOCK_PIXEL) {
//          matrix.drawPixel(x, y, matrix.Color333(7, 7, 7)); //red
//         } else if (receivedData == PIXEL) {
//          matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));//blue
//         } else if (receivedData == SCORE_PIXEL) {
//          matrix.drawPixel(x, y, matrix.Color333(7, 0, 7)); 
//         } else if (receivedData == CORNER_PIXEL_ABSENT) {
//          matrix.drawPixel(x, y, matrix.Color333(7, 7, 0));
//         } else if (receivedData == CORNER_PIXEL_PRESENT) {
//          matrix.drawPixel(x, y, matrix.Color333(7, 0, 4));
//         } else if (receivedData == CURRENT_BLOCK_PIXEL) {
//          matrix.drawPixel(x, y, matrix.Color333(4, 0, 7));
//         } else if (receivedData == CORNER_PIXEL_ABSENT_PLACED) {
//          matrix.drawPixel(x, y, matrix.Color333(7, 0, 4));
//         } else if (receivedData == END_MSG) {
              x = 0;
              y = 0;
           }

         if (receivedData!=EMPTY) {
          Serial.println(receivedData);
         }
         x++;
         if (x >= WIDTH) {
          y++;
          x = 0;
         }
         if (y >= HEIGHT) {
            x = 0;
            y = 0;
         }
//      }
//      if (Serial.available()) {
//        receivedData = Serial.read();
//        Serial.print("should be drawing!!");
//        Serial.println(receivedData);
//        Serial.print("x is");
//        Serial.println(x);
//        Serial.print("Y IS ");
//        Serial.println(y);
//        for (unsigned char n = 0; n < 8; n++) {
//          if (bitRead(receivedData, n) == 1) {
//            matrix.drawPixel(x, y, matrix.Color333(0, 7, 0));
//          }
//        }
//        if (x + 8 >= WIDTH) {
//          x = 0;
//          y++;
//        } else {
//          x += 8;
//        }
//        if (y >= HEIGHT) {
//          y = 0;
//        }
//      }

  } else if (prefix == 2) {
    prefix = 0;
  } else if (prefix == 3) {
    prefix = 0;
  } else if (prefix == 4) {
    prefix = 0;
  } else if (prefix == 5) {
    prefix = 0;
  } else if (prefix == 6) {
    prefix = 0;
    clearScreen();
  }
  //Serial.println(prefix);
}

void myClear() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void clearScreen() {
  matrix.fillScreen(0);
}
