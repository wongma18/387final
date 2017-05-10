/* Michael Wong
 * ECE 387 Final Code
 * 10 May 2017
 * Final Revision Version 7.08
 * 
 * This code if for the GPS system I made using
 * an Arduino Mega 2560
 * Github: https://github.com/wongma18/387final
 */
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

// initialize GPS
const int RXPin = 51;
const int TXPin = 53;
int GPSBaud = 4800;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// initialize the lcd
LiquidCrystal lcd(33, 31, 29, 27, 25, 23);

// initialize the keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {34, 32, 30, 28};
byte colPins[COLS] = {26, 24, 22};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// control buttons
const int modeUp = 3;
const int modeDown = 2;

// global variables
static double latitude = 0.0;
static double longitude = 0.0;
double skmph = 0.0;
static double dir = 0.0;
double altm = 0.0;
static double targetLat = 0;
static double targetLng = 0;

// for switch case
static int mode = 0;
static int modeChanged = 0;

void setup() {
  // start hardware serial
  Serial.begin(9600);

  // start software serial
  gpsSerial.begin(GPSBaud);

  // start lcd
  lcd.begin(16, 2);

  // mode change interupts
  pinMode(modeUp, INPUT_PULLUP);
  pinMode(modeDown, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(modeUp), upMode_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(modeDown), downMode_ISR, FALLING);
}


void loop() {
  while (gpsSerial.available() > 0)   // only run while SS port is active
  {
    // look for key pressed
    char key = keypad.getKey();
    // call coordinate entry
    if (key != NO_KEY) {  // enter target coordinates
      Serial.println(key);
      if (key == '#') {
        Serial.println("Entering Coordinates");
        lcd.clear();
        delay(100);
        targetLat = enterLat();
        targetLng = enterLng();
        lcd.clear();
        mode = 0;
      }
    }
    if (gps.encode(gpsSerial.read()))   // run GPS if it is sending data
    {
      switch (mode)
      {
        case 0 :
          mode0();
          break;
        case 1 :
          mode1();
          break;
        case 2 :
          mode2();
          break;
        case 3 :
          mode3();
          break;
        case 4 :
          mode4();
          break;
        case 5 :
          mode5();
          break;
        default :
          mode0();
          break;
      }
    }
  }
} // end loop

void requestEvent() {

}

// Default
void mode0()
{
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("GPS READY");
}

// clock
void mode1()
{
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }

  // Date
  lcd.setCursor(0, 0);
  lcd.print(F("Date: "));
  if (gps.date.isValid())
  {
    lcd.print(gps.date.month());
    lcd.print(F("/"));
    lcd.print(gps.date.day());
    lcd.print(F("/"));
    lcd.print(gps.date.year());
  }
  else
  {
    lcd.print(F("INVALID"));
  }

  // Time
  lcd.setCursor(0, 1);
  lcd.print("UT: ");
  lcd.setCursor(4, 1);
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) lcd.print(F("0"));
    lcd.print(gps.time.hour());
    lcd.print(F(":"));
    if (gps.time.minute() < 10) lcd.print(F("0"));
    lcd.print(gps.time.minute());
    lcd.print(F(":"));
    if (gps.time.second() < 10) lcd.print(F("0"));
    lcd.print(gps.time.second());
  }
  else
  {
    lcd.print(F("INVALID"));
  }
} // end clock

// position
void mode2()
{
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }

  lcd.setCursor(0, 0);
  if (gps.location.isValid())
  {
    lcd.print("LAT: ");
    lcd.print(gps.location.lat(), 6);

    lcd.setCursor(0, 1);
    lcd.print("LNG: ");
    lcd.print(gps.location.lng(), 6);
  }
  else
  {
    lcd.print(F("INVALID"));
  }
} // end position

// bearing
void mode3()
{
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }

  lcd.setCursor(0, 0);
  if (gps.speed.isValid())
  {
    lcd.print("SPEED: ");
    lcd.print(gps.speed.mps(), 1);
    lcd.print(" MPS");
  }
  else
  {
    lcd.print("INVALID");
  }

  lcd.setCursor(0, 1);
  if (gps.course.isValid())
  {
    lcd.print("HDG: ");
    lcd.print(gps.course.deg(), 2);
    lcd.print(" DEG");
  }
  else
  {
    lcd.print("INVALID");
  }
} // end bearing

// navigate
void mode4()
{
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }

  Serial.print("Target Lat: ");
  Serial.print(targetLat);
  Serial.print(", ");
  Serial.print("Target Lng: ");
  Serial.println(targetLng);
  
  // angle is unused in this version
  double angle = gps.course.deg() - (TinyGPSPlus::courseTo(gps.location.lat(),
                                     gps.location.lng(), targetLat, targetLng));
  // displayed on lcd
  double distTo = TinyGPSPlus::distanceBetween(gps.location.lat(),
                  gps.location.lng(), targetLat, targetLng);

  // put navigation info on LCD screen
  lcd.setCursor(0, 0);
  lcd.print("CC:");
  lcd.print(gps.course.deg(), 1);

  lcd.setCursor(0, 1);
  lcd.print("TC:");
  lcd.print(TinyGPSPlus::courseTo(gps.location.lat(),
                                  gps.location.lng(), targetLat, targetLng), 1);

  lcd.setCursor(9, 0);
  lcd.print("DIST:");
  lcd.setCursor(9, 1);
  lcd.print(distTo / 1000, 3);
}

// send coordinates over serial
void mode5() {
  if (modeChanged == 1) {
    lcd.clear();
    modeChanged = 0;
  }
  lcd.print("SENDING");
  lcd.setCursor(0, 1);
  lcd.print("COORDINATES");

  // creates string that receiver is supposed to parse out into coordinates
  Serial.print('a');
  Serial.print(gps.location.lat(), 6);
  Serial.print('A');
  Serial.println("");
  delay(500);
  Serial.print('b');
  Serial.print(gps.location.lng(), 6);
  Serial.print('B');
}

double enterLat() // called by coordinate entry for latitude
{
  int sign, digit1, digit2, digit3, digit4, digit5, digit6;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Lat: ");
  lcd.setCursor(8, 1);
  sign = getDigit();
  Serial.println("Sign Enertered");
  Serial.println(sign);
  lcd.setCursor(8, 1);
  if (sign == 0)
    lcd.print("-");
  if (sign == 1)
    lcd.print("+");
  lcd.setCursor(9, 1);
  digit1 = getDigit();
  Serial.println("Digit 1 Enertered ");
  Serial.println(digit1);
  lcd.setCursor(10, 1);
  digit2 = getDigit();
  Serial.println("Digit 2 Enertered ");
  Serial.println(digit2);
  lcd.setCursor(11, 1);
  lcd.print(".");
  lcd.setCursor(12, 1);
  digit3 = getDigit();
  Serial.println("Digit 3 Enertered ");
  Serial.println(digit3); 
  lcd.setCursor(13, 1);
  digit4 = getDigit();
  Serial.println("Digit 4 Enertered ");
  Serial.println(digit4);
  lcd.setCursor(14, 1);
  digit5 = getDigit();
  Serial.println("Digit 5 Enertered ");
  Serial.println(digit5);
  lcd.setCursor(15, 1);
  digit6 = getDigit();
  Serial.println("Digit 6 Enertered ");
  Serial.println(digit6);

  double coord;

  coord = (10 * digit1) + digit2 + (0.1 * digit3)
          + (0.01 * digit4) + (0.001 * digit5) + (0.0001 * digit6);
  if (sign == 0)
  {
    coord = coord * -1;
  }
  Serial.println(coord);
  return coord;
}

double enterLng() // called by coordinate entry for longitude
{
  int sign, digit1, digit2, digit3, digit4, digit5, digit6, digit7;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Lng: ");
  lcd.setCursor(7, 1);
  sign = getDigit();
  Serial.println("Sign Enertered");
  Serial.println(sign);
  lcd.setCursor(7, 1);
  if (sign == 0)
    lcd.print("-");
  if (sign == 1)
    lcd.print("+");
  lcd.setCursor(8, 1);
  digit1 = getDigit();
  Serial.println("Digit 1 Enertered ");
  Serial.println(digit1);
  lcd.setCursor(9, 1);
  digit2 = getDigit();
  Serial.println("Digit 2 Enertered ");
  Serial.println(digit2);
  lcd.setCursor(10, 1);
  digit3 = getDigit();
  Serial.println("Digit 3 Enertered ");
  Serial.println(digit3);
  lcd.setCursor(11, 1);
  lcd.print(".");
  lcd.setCursor(12, 1);
  digit4 = getDigit();
  Serial.println("Digit 4 Enertered ");
  Serial.println(digit4);
  lcd.setCursor(13, 1);
  digit5 = getDigit();
  Serial.println("Digit 5 Enertered ");
  Serial.println(digit5);
  lcd.setCursor(14, 1);
  digit6 = getDigit();
  Serial.println("Digit 6 Enertered ");
  Serial.println(digit6);
  lcd.setCursor(15, 1);
  digit7 = getDigit();
  Serial.println("Digit 7 Enertered ");
  Serial.println(digit7);

  double coord;

  coord = (100 * digit1) + (10 * digit2) + digit3 + (0.1 * digit4)
          + (0.01 * digit5) + (0.001 * digit6) + (0.0001 * digit7);
  if (sign == 0)
  {
    coord = coord * -1;
  }

  return coord;
}

// enter digit
int getDigit()
{
  char keyPressed = keypad.getKey();
  Serial.print(keyPressed);
  int digit;
  while (keyPressed == NO_KEY) {
    keyPressed = keypad.getKey();
  }
  if (keyPressed != NO_KEY && keyPressed != '*') {
    switch (keyPressed) // which value this digit should be
    {
      case '0':
        digit = 0;
        lcd.print(digit);
        return digit;
        break;
      case '1':
        digit = 1;
        lcd.print(digit);
        return digit;
        break;
      case '2':
        digit = 2;
        lcd.print(digit);
        return digit;
        break;
      case '3':
        digit = 3;
        lcd.print(digit);
        return digit;
        break;
      case '4':
        digit = 4;
        lcd.print(digit);
        return digit;
        break;
      case '5':
        digit = 5;
        lcd.print(digit);
        return digit;
        break;
      case '6':
        digit = 6;
        lcd.print(digit);
        return digit;
        break;
      case '7':
        digit = 7;
        lcd.print(digit);
        return digit;
        break;
      case '8':
        digit = 8;
        lcd.print(digit);
        return digit;
        break;
      case '9':
        digit = 9;
        lcd.print(digit);
        return digit;
        break;
    }
  }

}

// ISRs
void upMode_ISR() {
  mUp();
}

void mUp() {
  mode = mode + 1;
  if (mode >= 6) {
    mode = 0;
  }
  delay(200);
  modeChanged = 1;
}

void downMode_ISR() {
  mDown();
}

void mDown() {
  mode = mode - 1;
  if (mode <= 0) {
    mode = 5;
  }
  delay(200);
  modeChanged = 1;
}
