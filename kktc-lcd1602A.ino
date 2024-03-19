/*
  Based on LiquidCrystal Library - Hello World
  http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

// include the library code:
#include <LiquidCrystal.h>
#include <EncButton2.h>

String in = "";
bool rdy, gotFirstMessage, backlight = false;
int mode = 0, backlightTO = 0, screen = 0;
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
EncButton2<EB_BTN> enc(INPUT_PULLUP, 7);           // просто кнопка


void setup() {
  Serial.begin(9600);
  enc.setHoldTimeout(5000);
  pinMode(6, OUTPUT);
  digitalWrite(6, 1);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Power up...");
}

void(* resetFunc) (void) = 0;

void loading() {
  Serial.println("Loading...");
  lcd.setCursor(0,0);
  lcd.print("Display     ");
  lcd.setCursor(0,1);
  lcd.print("disconnected");
}

void connEstablished(){
  mode = 0;
  in = "";
  Serial.println(mode);
  lcd.setCursor(0,0);
  lcd.print("Connected");
  lcd.setCursor(0,1);
  lcd.print("Getting data...");
}

void printMainScreen(String a){
  String y = a.substring(0, 4);
  String m = a.substring(4, 6);
  String d = a.substring(6, 8);
  String h = a.substring(8, 10);
  String mm = a.substring(10, 12);
  int temp = a.substring(12, 15).toInt() - 273;
  String weather = a.substring(15, 20);
  String wind = a.substring(20, 22);
  String result1;
  result1.reserve(16);
  result1 += d;
  result1 += "/";
  result1 += m;
  result1 += "/";
  result1 += y;
  result1 += " ";
  result1 += h;
  result1 += ":";
  result1 += mm;
  String result2;
  result2.reserve(16);
  result2 += temp;
  result2 += "C ";
  result2 += weather;
  result2 += " ";
  result2 += wind;
  result2 += "m/s";
  lcd.setCursor(0,0);
  lcd.print(result1);
  lcd.print("     ");
  lcd.setCursor(0,1);
  lcd.print(result2);
  lcd.print("     ");
}

void printDiscScreen(String a){
  int totSpace = a.substring(0, 4).toInt();
  int occSpace = a.substring(4, 8).toInt();
  int freeSpace = totSpace - occSpace;
  double occPercent = ((float)occSpace/totSpace)*100;
  int squares = ceil(occPercent/12.5);
  String result1;
  result1.reserve(16);
  result1 += "Free:";
  result1 += freeSpace;
  result1 += "/";
  result1 += totSpace;
  result1 += "Gb";

  lcd.setCursor(0,0);
  lcd.print(result1);
  lcd.print("     ");
  lcd.setCursor(0,1);
  lcd.print("Used:");
  for (int i = 0; i < squares; i++) {
    lcd.write(255);
  }
  for (int i = 0; i < 8-squares; i++) {
    lcd.write("_");
  }
  lcd.print((int)ceil(occPercent));
  lcd.print("%");
  lcd.print("     ");
}

void printHealthScreen(String a){
  long uptimeSec = a.substring(0, 7).toInt();
  String cpuTemp = a.substring(7, 10);
  cpuTemp.trim();
  String cpuLoad = a.substring(10, 12);

  String result1;
  result1.reserve(16);
  result1 += "Up for ";
  result1 += uptimeSec/60/60;
  result1 += "h ";
  result1 += (uptimeSec/60)%60;
  result1 += "m";

  String result2;
  result2.reserve(16);
  result2 += "CPU: ";
  result2 += cpuTemp;
  result2 += "C ";
  result2 += cpuLoad;
  result2 += "%";
  
  lcd.setCursor(0,0);
  lcd.print(result1);
  lcd.print("     ");
  lcd.setCursor(0,1);
  lcd.print(result2);
  lcd.print("     ");
}

void turnOnBacklight() {
  backlightTO = 0;
  digitalWrite(6, 1);
  backlight = true;
}

void loop() {
  enc.tick();                       // опрос происходит здесь
  if (rdy == false) {
    if (millis()%5000==0) { 
      loading();
    }
  }
  
  if (Serial.available() > 0) {
    in = Serial.readString();
    in.trim();
    if (in == "rdy") {
      rdy = true;
      connEstablished();
    }
    else {
      if (!gotFirstMessage) {
        mode = 0;
        printMainScreen(in);
        turnOnBacklight();
        gotFirstMessage = true;
      }
      if (screen == 0 && in != "") {
        printMainScreen(in);
      }
      else if (screen == 1 && in != "") {
        printDiscScreen(in);
      }
      else if (screen == 2 && in != "") {
        printHealthScreen(in);
      }
    }
  }

  if (millis()%1000==0) {
    backlightTO += 1;
//    Serial.println(backlightTO);
    if (backlightTO > 200) {
      digitalWrite(6, 0);
      backlightTO = 0;
      backlight = false;
    }
  }
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  // print the number of seconds since reset:
  if (enc.click()) {
    if (rdy && backlight) {
      mode += 1;
      Serial.println(mode%3);  
      screen = mode%3;

    }
    turnOnBacklight();
  }
  
  if (enc.held()) resetFunc();
}
