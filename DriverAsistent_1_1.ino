// POJECT DriverAssistant
// Baina S
// ver. 1.1 2017

// LIBRARY
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <TFT.h>
#include <TimerOne.h>

// SETUP PINS RC522
#define RFID_SS 4  // SPI slave select
#define RFID_RST 2 // reset

// SETUP PINS TFT LCD
#define TFT_SS 10 // SPI slave select
#define TFT_DC 9  // data-command
#define TFT_RST 8 // reset

// SETUP PINS SD CARD READER
#define SD_SS 7 // SPI slave select

// SETUP PINS BUZZER
#define BUZZER_PIN 3 // PWM out
#define maxString 13 // максимальное количество символов с строке на экране

// CREATE INSTANS
MFRC522 rfidReader = MFRC522(RFID_SS, RFID_RST);
TFT tftScreen = TFT(TFT_SS, TFT_DC, TFT_RST);
PImage iZMain;    // знак главная дорога
PImage iZ50;      // знак ограничение скорости 50
PImage iZWalker;  // знак пешеходный переход

// GLOBAL VAR
bool SDReady = false;
bool iZReady = false;
char target[maxString + 1] = ""; // переменная для utf8rus

///////////////////////////////////////////////////////

void setup() {

  // prepare device
  prepareRFID();
  prepareScreen();
  prepareSDcard();

  // подготовка картинок
  if (SDReady) {
    iZMain    = tftScreen.loadImage("Z1.bmp");
    iZ50      = tftScreen.loadImage("Z2.bmp");
    iZWalker  = tftScreen.loadImage("Z3.bmp");
    if (iZMain.isValid() && iZ50.isValid() && iZWalker.isValid()) {
      iZReady = true;
    } else {
      iZReady = false;
    }
  }

  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned int tagID=0U;
  
  if (readRFIDtag(&tagID)==1){
    printBlanckToTFT();
    switch(tagID){
      case 1071U: // 04 2F главная дорога
        if(iZReady){
          printInfo1("вы на главной");
          beeBoo();
          tftScreen.image(iZMain, 0, 44);
        } 
        break;
      case 1079U: // 04 37 ограничение скорости 50
        if(iZReady){
          printInfo1("ограничение:");
          beeBoo();
          tftScreen.image(iZ50, 0, 44);
        }
        break;
      case 1035U: // 04 0B пешеходный переход
        if(iZReady){
          printInfo1("Внимание!");
          beeBoo();
          tftScreen.image(iZWalker, 0, 44);
        }
        break;  
      default:
      printBlanckToTFT();
    }
  }

}

byte readRFIDtag(unsigned int* tagID) {

  if (!rfidReader.PICC_IsNewCardPresent()) {
    return 0;
  }

  if (!rfidReader.PICC_ReadCardSerial()) {
    return 0;
  }
  
  *tagID = 0;
  // ID - младшие два байта. Для полного ID использовать "i < rfidReader.uid.size"
  for (byte i = 0; i < 2; i++) {
    *tagID = *tagID*256 + rfidReader.uid.uidByte[i];
  }
  rfidReader.PICC_HaltA();
  return 1;
}

void printInfo1(unsigned char* info) {
    tftScreen.text(utf8rus(info), 2, 22);
}

void printBlanckToTFT() {
  tftScreen.stroke(0, 0, 0);
  tftScreen.fill(0, 0, 0);
  tftScreen.rect(0, 0, 160, 128);
  tftScreen.stroke(255, 255, 255);

  tftScreen.text(utf8rus("Ассистент 1.0"), 2, 2); // X, Y
}

void beeBoo() {
  TCCR2B = TCCR2B & 0b11111000 | 0x03; // PWM frequency 980Hz
  analogWrite(BUZZER_PIN, 128);
  
  // timer; interrupt every 0.4 sec; callback - beepPWM()
  Timer1.initialize();
  Timer1.attachInterrupt(beepPWM, 200000);
}

void beepPWM() {
  static byte count = 0;

  switch (count) {
    case 0:
      count++;
      break;
    case 1:
      TCCR2B = TCCR2B & 0b11111000 | 0x05; // PWM frequency 245Hz
      analogWrite(BUZZER_PIN, 128);
      count++;
      break;
    default:
      Timer1.detachInterrupt();
      digitalWrite(BUZZER_PIN, LOW);
      count = 0;
  }
}

// SETUP METHODS
void prepareScreen() {
  tftScreen.begin();
  tftScreen.background(0, 0, 0);
  tftScreen.setTextSize(2); // 20 pixels
  printBlanckToTFT();
  printInfo1("готов");
}

void prepareRFID() {
  SPI.begin();
  rfidReader.PCD_Init();
  rfidReader.PCD_SetAntennaGain(rfidReader.RxGain_max);
}

void prepareSDcard() {
  SDReady = false;
  if (SD.begin(SD_SS)) {
    SDReady = true;
  }
}

void printToSerial(unsigned char *arrayToPrint) {
  for (int i = 0; i < 5; i++) {
    Serial.print(arrayToPrint[i], HEX);
    Serial.print(":");
  }
  Serial.println("");
}
