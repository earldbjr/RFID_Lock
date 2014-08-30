#include <SPI.h>
#include "MFRC522.h"

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.

int redLed1 = A4;//Door locked, Analog pin0.
int greenLed1 = A5;//Door unlocked, Analog pin1.
int doorSensor = 2;
int lockPin1 = 7;
int lockPin2 = 6;
//boolean safeToLock = 0;
String card1 = "2454512237";
String card2 = "2454512237";
String card3 = "2454512237";
String card4 = "2454512237";
String card5 = "2454512237";
String card6 = "2454512237";
/*Blacklisted numbers:
 2445113213 Blank white card - Lost
 */
void parseLock();
void offLock();
void unlockDoor();
void lockDoor();
void doorSensorDetect();

void setup() {
  //Serial.begin(9600);        // Initialize serial communications with the PC
  SPI.begin();                // Initialize SPI bus
  mfrc522.PCD_Init();        // Initialize MFRC522 card
  pinMode(lockPin1, OUTPUT);
  pinMode(lockPin2, OUTPUT);
  pinMode(redLed1, OUTPUT);
  pinMode(greenLed1, OUTPUT);
  pinMode(doorSensor, INPUT);
  //attachInterrupt(0,doorSensorDetect,CHANGE); //  0=pin2, 1=pin3
}

void loop() {
  static int firstRun = 0;
  if(firstRun == 0){ 
    firstRun = 1;
    lockDoor();
  }
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Now a card is selected. The UID and SAK is in mfrc522.uid.

  // Dump UID
  //Serial.print("Card UID:");
  String idRead;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //Serial.print(mfrc522.uid.uidByte[i]);//, HEX);
    //idRead.concat(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    idRead.concat(mfrc522.uid.uidByte[i]);
  } 
  //Serial.print(idRead);
  //Serial.println();
  //if (idRead == officeCard) {
  //    digitalWrite(redLed1, HIGH);
  //    digitalWrite(greenLed1, HIGH);
  //}

  if (idRead == card1 || idRead == card2 || idRead == card3 || idRead == card4 || idRead == card5 || idRead == card6){
    unlockDoor();
  }
  checkDoorSensor();
}

void offLock(){
  digitalWrite(lockPin1, LOW);
  digitalWrite(lockPin2, LOW);
  delay(1000); //Give relay time to switch back to ground and settle before another operation can be made.
}

void unlockDoor(){
  digitalWrite(lockPin1, LOW);
  digitalWrite(lockPin2, HIGH);
  digitalWrite(redLed1, LOW);
  digitalWrite(greenLed1, HIGH);
  delay(1000); //Allow motor to engage.
  offLock();
}

void lockDoor(){
  digitalWrite(lockPin1, HIGH);
  digitalWrite(lockPin2, LOW);
  digitalWrite(redLed1, HIGH);
  digitalWrite(greenLed1, LOW);
  delay(1000); //Allow motor to engage.
  offLock();
}

//void doorSensorDetect(){
//safeToLock = digitalRead(doorSensor);
//}

void checkDoorSensor(){
  while(digitalRead(doorSensor) == 0){
    //There's nothing to do until the door is closed...
  }
  static int isLocked = false; //Analagous to "lock state".
  if(digitalRead(doorSensor) == 1 && isLocked !=1){  //If door is closed(guaranteed, but checked for sanity), and door wasn't locked last iteration
    int time = millis();
    boolean safe = true;
    while(millis()-time <= 2000){ //Test for two seconds.
      if(digitalRead(doorSensor) == 0){ //If door reads open during two seconds...
        safe = false;             //evaluate false
      }
    }
    if(safe == true){             //If test succeeded
      isLocked = true;       //If test didn't evaluate false (door measured closed for 2 seconds)
      lockDoor();                 //Lock the door.
    } 
    else if (safe == false){    //If door read open during the two seconds...
      isLocked = false;           //Set result to failed, to indicate test unsuccessful, and that it's ok to test again next iteration.
    }
  }
}





