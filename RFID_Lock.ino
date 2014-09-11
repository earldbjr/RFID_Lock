#include <SPI.h>
#include "MFRC522.h"
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.

int isLocked = 0;
int redLed1 = A4;//Door locked, Analog pin0.
int greenLed1 = A5;//Door unlocked, Analog pin1.
int doorSensor = 2;
int lockPin1 = 7;
int lockPin2 = 6;
String Cards;
//char* Cards[]= {"2454512237", "461691025450129", "451661025450129", "421631025450129", "4121661025450129"}; //To be implemented: Read long string from EEPROM, tokenize string to fill this var.
/*
String card1 = "2454512237";   //Given to Sun Bay office
 String card2 = "461691025450129";  //Gizmo's Phone
 String card3 = "451661025450129"; //Zahrah's Phone
 String card4 = "421631025450129";  //Zahrah's Purse?
 String card5 = "4121661025450129";   //Gizmo's Wallet
 Blacklisted numbers:
 2445113213 Blank white card - Lost
 */
int intISR = 0;

void offLock();
void unlockDoor();
void lockDoor();
String readCard();
void reProgramISR();
void Reprogram();
void setEEPROM(String card);
int validateCard(String card);

String getEEPROM();

void setup() {
  Cards = getEEPROM(); // Work off of this string instead of always using ROM
  SPI.begin();                // Initialize SPI bus
  mfrc522.PCD_Init();        // Initialize MFRC522 card
  pinMode(lockPin1, OUTPUT);
  pinMode(lockPin2, OUTPUT);
  pinMode(redLed1, OUTPUT);
  pinMode(greenLed1, OUTPUT);
  pinMode(doorSensor, INPUT);
  attachInterrupt(0, reProgramISR, CHANGE); //0?
}

void loop() {
  if(intISR == 1){
    int sentinel = 1;
    unsigned long time = millis();
    while(millis()-time <= 200){
      if(digitalRead(6) == 0){
        sentinel = 0;
      }
    }
    if(sentinel == 1){
      Reprogram();
      intISR = 0;
    }
  }
  String cardNumber = readCard(); //checkDoorSensor();
  if(cardNumber != "invalid" && validateCard(cardNumber) == 1){
    unlockDoor();
  }
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

void checkDoorSensor(){
  int safe = 1;
  static int named = 0;
  int sensorReading = digitalRead(doorSensor);
  if(sensorReading == 0){
    isLocked = 0;
    named = 0;
    digitalWrite(redLed1, LOW);
    digitalWrite(greenLed1, HIGH);
  }
  else if (sensorReading == 1 && isLocked == 0 && named == 0){  //If door is closed(guaranteed, but checked for sanity), and door wasn't locked last iteration

    unsigned long time = millis();
    while(millis()-time <= 2000){ //Test for two seconds.
      if(digitalRead(doorSensor) == 0){ //If door reads open during two seconds...
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

String readCard(){
  String idRead = "invalid";

  // New card found && new card selected
  if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    for (byte i = 0; i < mfrc522.uid.size; i++) { // Dump UID for authentication
      idRead.concat(mfrc522.uid.uidByte[i]);
    }
    return(idRead);
  }
}

void reProgramISR(){
  if(digitalRead(6) == HIGH){
    int intISR = 1;
  }
  else{
    intISR = 0;
  }
}

void Reprogram(){
  if(intISR == 1){ //If reprogram button is depressed
    digitalWrite(redLed1, LOW);
    digitalWrite(greenLed1, LOW);
    setEEPROM(readCard());
  }
}


//EEPROM: \n separated values, \0 terminated.
void setEEPROM(String card){
  if(card == "invalid"){return;}
  //Check to see if presented card is already in ROM.
  //If yes: Remove card from ROM by rebuilding Cards and rewriting to ROM. Turn on red LED.
  //If no: Add card to ROM by same method. Turn off red LED, turn off red LED, turn on green LED
  if(validateCard(card) == 1){
    //Remove Card Here
    int index = Cards.indexOf(card); //Points to the card marked for removal
    int length = card.length();      //Length of the card marked for removal
    String newCards;
    if(index > 1){                   //If card to be removed isn't the first card...
      newCards.concat(Cards.substring(0, index-1));//Add all cards that come before it.
    }
    newCards.concat(Cards.substring(index+length+2, Cards.length()-1));//Add all cards that come after it, less \0.
    newCards.concat('\0');
    digitalWrite(greenLed1, LOW);
    digitalWrite(redLed1, HIGH);
    for(int i=0; i < newCards.length();i++){
      EEPROM.write(i, newCards[i]);
      Cards = newCards; //Update working table without needing to powercycle.
    }
    delay(2000);
    } 
  else {
    //Add Card Here, dont forget \n and \0!
    String newCards = Cards.substring(0,Cards.length()-1);
    newCards.concat(card);
    newCards.concat('\n');
    newCards.concat('\0');
    digitalWrite(redLed1, LOW);
    digitalWrite(greenLed1, HIGH);
    for(int i=0; i < newCards.length();i++){
      EEPROM.write(i,newCards[i]);
      Cards = newCards; //Update working table without needing to powercycle.
    }
    delay(2000);
  }
}

String getEEPROM(){
  String contents;
  int bytePlace = 0;
  char chByte = EEPROM.read(bytePlace++);
  while(chByte != '\0'  && chByte != 255){ //ROM bytes never written to contain 'ÿ' or 255. This prevents returning 1024 'ÿ's if nothing has been written to ROM.
    contents.concat(chByte);
    chByte = EEPROM.read(bytePlace++);
  }
  contents.concat('\0');
  return(contents);
}

int validateCard(String card){
  String nextCardToCheck;
  int intPosition = 0;
  while(Cards[0] != '\0' && Cards[0] != 255){
    nextCardToCheck = Cards.substring(intPosition, Cards.indexOf('\n')-1);
    intPosition += nextCardToCheck.length()+1; //+1 to include \n.
    if(card == nextCardToCheck){
      return 1;
    }
  }
  return 0;
}








