/*=================================================*/
/*==== B L U E T O O T H   S M A R T   P L U G ====*/
/*=================================================*/

/**AUTHOR: Jason Bentley**/
/**DATE:   12/31/2021**/

/***************************************************************************
 *** The firmware allows for two-way communication between a a bluetooth ***
 *** device and the arduino for the purpose of operating a relay         ***
 *** and controling a smartplug                                          ***
 ***************************************************************************/

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial BTserial(9, 10); //(Rx, Tx) on arduino
LiquidCrystal  lcd(4, 13, 12, 11, 6, 5);

bool BTconnection = false; 

const int txBT       = 9;
const int rxBT       = 10;
const int powerPin   = 2;
const int state      = 3;
const int relayPin   = 7;
const int onOff      = 8;

int stateCommunicatorCounter  = 0;
int switchState               = LOW;
int prevSwitchState           = LOW; 
int power                     = LOW;

unsigned long buttonPress = 0;

char btData = ' ';

void setup() {
  lcd.begin(16,2);

  pinMode(txBT,  INPUT);
  pinMode(onOff, INPUT);
  pinMode(state, INPUT);

  pinMode(rxBT,     OUTPUT);
  pinMode(powerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(9600);
 
  lcdIntroSequence();

  Serial.println("");
  Serial.println("Arduino is ready...");
  Serial.println("Turn on BT now...");
      
  while(power == LOW){
      if (switchStateHelper() == HIGH){
      switchStatus(HIGH);
      }
  }

  Serial.println("BT is powered up...");
  
  lcd.clear();
  lcd.print("BT is turned on!");
  lcd.setCursor(0,1);
  lcd.print("connect BT now...");

  while(BTconnection == false){
    Serial.println("connecting...");  
    delay(3000); //gives user time to read LCD messages
    
    if (digitalRead(state) == HIGH){
      BTconnection = true;
      
      lcd.clear();
      lcd.print("SUCCESS!");
      lcd.setCursor(0, 1);
      lcd.print("device connected");
    }
  }
  
  delay (10);
  BTserial.begin(9600);
  
  delay(10);
  BTserial.println("Connected to Arduino!");
 
  delay(3000); //gives user time to read LCD messages
}

void loop() {
  //delays help to alleviate bugs in LCD communication
  int status = switchStateHelper();
  delay(5);

  switchStatus(status);
  delay(5);

  lcdBTConnectionStatus(status);
  delay(75);   //lcd write speed is slower than arduino clock
               //allows enought time for LCD to display message
  stateCommunicator();
  delay(5);

  onOffBTSerialReader();
  delay(5);
}

/*returns the value of  power (HIGH or LOW)
 *allows for push-button swtich operation*/
int switchStateHelper(){
  switchState = digitalRead(onOff);
  
  if (switchState != prevSwitchState){
    if (switchState == HIGH){
      power = !power;
    }    
  }
  prevSwitchState = switchState;
  return power;
}

/*logic to switch power on/off 
 *to the HC-05 module*/
void switchStatus(int status){
  if (status == HIGH){
    digitalWrite(powerPin, status);
  }else {
    digitalWrite(powerPin, status);
  }
}

/*writes intro sequence to LCD*/
void lcdIntroSequence(){
  lcd.print("Hello!");
  delay(3000);
  
  lcd.clear();
  lcd.print("Arduino ready...");
  lcd.setCursor(0, 1);
  lcd.print("BT module is off");
}

/*finds, and writes, connection and 
 *power status of the HC-05 module
 *to the LCD*/
void lcdBTConnectionStatus(int status){
   if (digitalRead(state) == HIGH){
    lcd.clear();
    lcd.print("POWER: on");
    lcd.setCursor(0,1);
    lcd.print("BT: connected");

  }else{
    if (status == LOW){
    lcd.clear();
    lcd.print("POWER: off");
    lcd.setCursor(0,1);
    lcd.print("BT: disconnected");
    }else{
    lcd.clear();
    lcd.print("POWER: on");
    lcd.setCursor(0,1);
    lcd.print("BT: disconnected");
    }
  }
}

/*Writes confrimation message of 
 *successful connection onto the 
 *BTserial port*/ 
void stateCommunicator(){
  int status = digitalRead(state);
  
  if (status == HIGH){
    if (stateCommunicatorCounter > 0){
      BTserial.println("Connected to Arduino!");
      stateCommunicatorCounter = 0;
    }
  }else{
    stateCommunicatorCounter = stateCommunicatorCounter + 1;
  }
}

/*Logic for controlling relay, reads
 *BTserial. 4 availble functions:
 *  CODE  ||      FUNCTION
 *===============================
 *   '0'  |  relayPin --> LOW
 *   '1'  |  relayPin --> HIGH
 *   '2'  |  flash 3 times
 *           (.5 second delay)
 *   '3'  |  call flashHelper() */
void onOffBTSerialReader(){
  
  while(BTserial.available()){
    btData = BTserial.read();
   
    if(btData == '1'){
       digitalWrite(relayPin, HIGH);

    }else if (btData == '0'){
      digitalWrite(relayPin, LOW);
      
    }else if (btData == '2'){
      for (int i = 0; i < 3; i++){
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
        delay(500);
      }
   
    }else if (btData == '3'){
      flashHelper();
    } 
  }
}

/*helper function for non-stop 
 *flashing feature-loops until
 *passed the "realyPin --> LOW"
 *command from BTserial*/
void flashHelper(){

  while(BTserial.read() != '0'){
    unsigned long timeSincePress  = millis() - buttonPress;
    
    if((timeSincePress >= 489) && (timeSincePress <= 514)){
      digitalWrite(relayPin, HIGH);   

    }else if (timeSincePress >= 1000){
      digitalWrite(relayPin, LOW);
      buttonPress = millis();
    }
  }
  digitalWrite(relayPin, LOW);
}



