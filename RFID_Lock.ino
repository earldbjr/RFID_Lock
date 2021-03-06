#include <SPI.h>
#include "MFRC522.h"

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.

int isLocked = 0;
int redLed1 = A4;//Door locked, Analog pin0.
int greenLed1 = A5;//Door unlocked, Analog pin1.
int doorClosed = 2;
int lockPin1 = 7;
int lockPin2 = 6;
String card1 = "2454512237";   //Given to Sun Bay office
String card2 = "461691025450129";  //Gizmo's Phone
String card3 = "451661025450129"; //Zahrah's Phone
String card4 = "421631025450129";  //Zahrah's Purse?
String card5 = "4121661025450129";   //Gizmo's Wallet
String card6 = "2454512237";
/*Blacklisted numbers:
 2445113213 Blank white card - Lost
 */
void offLock();
void unlockDoor();
void lockDoor();
void timeoutTimer();
void setup() {
  SPI.begin();                // Initialize SPI bus
  mfrc522.PCD_Init();        // Initialize MFRC522 card
  pinMode(lockPin1, OUTPUT);
  pinMode(lockPin2, OUTPUT);
  pinMode(redLed1, OUTPUT);
  pinMode(greenLed1, OUTPUT);
  pinMode(doorClosed, INPUT);
}

void loop() {
  String idRead;
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // New card found && new card selected
  if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    for (byte i = 0; i < mfrc522.uid.size; i++) { // Dump UID for authentication
      idRead.concat(mfrc522.uid.uidByte[i]);
    }
    if (idRead == card1 || idRead == card2 || idRead == card3 || idRead == card4 || idRead == card5 || idRead == card6){ //Authenticate
      if(isLocked == 1){
        unlockDoor();
      }
    }
  }

  checkdoorClosed();
  timeoutTimer();
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
  isLocked = 0;
  delay(1000); //Allow motor to engage.
  offLock();
}

void lockDoor(){
  digitalWrite(lockPin1, HIGH);
  digitalWrite(lockPin2, LOW);
  digitalWrite(redLed1, HIGH);
  digitalWrite(greenLed1, LOW);
  isLocked = 1;
  delay(1000); //Allow motor to engage.
  offLock();
}

void checkdoorClosed(){
  int safe = 1;
  static int named = 0;
  int sensorReading = digitalRead(doorClosed);
  if(sensorReading == 0){
    isLocked = 0;
    named = 0;
    digitalWrite(redLed1, LOW);
    digitalWrite(greenLed1, HIGH);
  }
  else if (sensorReading == 1 && isLocked == 0 && named == 0){  //If door is closed(guaranteed, but checked for sanity), and door wasn't locked last iteration

    unsigned long time = millis();
    while(millis()-time <= 2000){ //Test for two seconds.
      if(digitalRead(doorClosed) == 0){ //If door reads open during two seconds...
        isLocked = 0;       //door must be unlocked if open
        safe = 0;
      }
    }
    if(safe == 1){
      lockDoor();             //Lock the door. 
      named = 1;
    }
  }
  else {    //Just to document if Reading == 1, isLocked == 1;

  }
}

void timeoutTimer(){
  signed static long oldTime = millis();
  if(isLocked == 1 || digitalRead(doorClosed) == 0){
    oldTime = -1; //lockout this function
    return;
  }
  if(oldTime == -1){ //Implies door is unlocked
    oldTime = millis(); //Start the clock
  }
  if(millis()-oldTime >= 10000){ //If door has been unlocked for greater than 10 seconds, but not opened...
    lockDoor();
    oldTime = -1; //lockout this function until door is closed but unlocked.
  }
}




