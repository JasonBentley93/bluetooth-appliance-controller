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
  lcdBootSequence();

  pinMode(txBT,  INPUT);
  pinMode(onOff, INPUT);
  pinMode(state, INPUT);

  pinMode(rxBT,     OUTPUT);
  pinMode(powerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(9600);
  delay(6000);
  
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
    delay(3000);
    
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
 
  delay(3000);
}


void loop() {

  //delays in loop() help to alleviate bugs in lcd communication
  int status = switchStateHelper();
  delay(5);

  switchStatus(status);
  delay(5);

  lcdBTConnectionStatus(status);
  delay(75);
  
  stateCommunicator();
  delay(5);

  onOffBTSerialReader();
  delay(5);
  

}

/*returns the state of the switch
 *switchState() == HIGH --> HC-05 module has power
 *switchState() == LOW  -->  HC-05 module has no power*/
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

/*writes bootsequence to the LCD*/
void lcdBootSequence(){
   delay(100);
  lcd.begin(16, 2);
  lcd.print("Setting Up!");
  lcd.setCursor(0, 1);
  lcd.print("please wait...");
}

/*writes intro sequence to LCD*/
void lcdIntroSequence(){
  lcd.clear();
 
  lcd.print("Setup complete!");
  delay(2000);
  
  lcd.clear();
  
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

/*Logic for controlling relay*/
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
 *flashing feature*/
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



