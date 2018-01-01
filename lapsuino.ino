// ===================================================================
// Version Info
// v 2.3: In case of negative delay system stopped working: fixed.
// v 2.4: Fast Move is changed with real measures
// ===================================================================
#include <stdio.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Motor Shield Drive
int ENA=11;//connected to Arduino's port 5(output pwm)
int IN1=2;//connected to Arduino's port 2
int IN2=3;//connected to Arduino's port 3

// ===================================================================
// hardware settings
#define rightKey  0
#define upKey     1
#define downKey   2
#define leftKey   3
#define selectKey 4
#define NUM_KEYS  5
// ===================================================================
// Program parameters
//Time for mode TL
#define minTime   1
#define maxTime 999
int valueTime = 30;

//Distance for Mode TL
#define minDist   1
#define maxDist  90
int valueDist = 2;

//Digit Step for changing time, distance etc.
#define minDS   1
#define maxDS  50
int DS = 1;

int testValue = 1;
int awValue = 200;

//Direction
#define rightDirection 0
#define leftDirection 1
int motorDirection = leftDirection;

#define programTimeLapse 0
#define programTime 1
#define programDistance 2
#define programFastMove 3
#define programDigitStep 4
#define programTestDistanceAW 5

int programMode    =  programTimeLapse;

boolean inMainProgram  =  false;

int programmSequence = 0;
#define sequenceModeSelect  0
#define sequenceModeOption  1
#define sequenceModeProgram 2

int analogMotorDelay = 100;


// first menu line - description of menu level
char sequenceHeader[3][17]  = {"Mode select     ", "Option select   ", "Program start   "};

// second menu line - mode select
char sequenceModes[6][17]   = {"  Time Lapse   >",  "< Time         >","< Distance     >", "< Fast Move    >", "< Digit Step   >","< Test Dist     "};
// second menu line - options settings
char sequenceOptions[6][17] = {"Direction:","Time:", "Distance:", "Distance:","DS:","Test.mode:"};

void setMotorLeftDirection(){
  digitalWrite(ENA,LOW);
  digitalWrite(IN1,LOW); 
  digitalWrite(IN2,HIGH);
}
void setMotorRightDirection(){
  digitalWrite(ENA,HIGH);
  digitalWrite(IN1,HIGH); 
  digitalWrite(IN2,LOW);
}

void stepDCMotor(){  
   analogWrite(ENA,awValue);//start driving DC motorA
   delay(analogMotorDelay);
   analogWrite(ENA,0);//stop driving DC motorA
}

void fastMoveDCMotor(){  
   analogWrite(ENA,255);//start driving DC motorA
}

void stopDCMotor(){
  analogWrite(ENA,0);
}

void moveDCMotorAW(){
  analogWrite(ENA,awValue);
}


void setup() 
{ 
    Serial.begin(9600);
    // brighness. The backlight brightness is tied to Arduino’s pin 10 (a PWM output)
    analogWrite (10, 10);
    pinMode(ENA,OUTPUT);//output
    pinMode(IN1,OUTPUT);
    pinMode(IN2,OUTPUT);
    // initial lcd display while initialize and pc detection
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Lapsuino  2.4");
    
    programmSequence = sequenceModeSelect;
    inMainProgram = false;
    
    delay(1000);
    updateScreen();
}

// ===================================================================
// main loop with new key detection
int keyEvent  = -1;
int lastEvent = -1;
int countEvent = 0;

//Key message
char msgs[5][3] = {
    "> ", 
    "^ ", 
    "v ", 
    "< ", 
    "* " };

void updateScreen()
{
    lcd.clear();
    lcd.print(sequenceHeader[programmSequence]);
    lcd.setCursor(0,1);
    switch(programmSequence)
    {
        case sequenceModeSelect:
            menuModeSelect( keyEvent );
            break;
        case sequenceModeOption:
            menuOptionSelect( keyEvent );
            break;
        case sequenceModeProgram:
            break;        
    }
}

void loop() 
{
    int keyEvent = detectKey();
    if (keyEvent >= 0)
    {
        switch (keyEvent)
        {
            case upKey:
                if (!inMainProgram)
                {
                    if (programmSequence > sequenceModeSelect)
                        programmSequence--;
                    updateScreen();
                }
                break;
            case downKey:
                if (!inMainProgram)
                {
                    if (programmSequence < sequenceModeProgram)
                        programmSequence++;
                    updateScreen();
                }
                break;
            case rightKey:
                if (!inMainProgram)
                {
                    switch (programmSequence)
                    {
                        case sequenceModeSelect:
                            menuModeSelect( keyEvent );
                            break;
                        case sequenceModeOption:
                            menuOptionSelect( keyEvent );
                            break;
                        case sequenceModeProgram:                            
                            break;
                    }
                }
                break;            
            case leftKey:
                if (!inMainProgram)
                {
                    switch (programmSequence)
                    {
                        case sequenceModeSelect:
                            menuModeSelect( keyEvent );
                            break;
                        case sequenceModeOption:
                            menuOptionSelect( keyEvent );
                            break;
                        case sequenceModeProgram:                            
                            break;
                    }
                }
                break;
            case selectKey:
                if(!inMainProgram){
                  inMainProgram = true;
                  startMainProgram( keyEvent);
                }else{
                  inMainProgram = false;
                  lcd.setCursor(0,1);
                  lcd.print( sequenceModes[programMode] );
                }                
                break;
        }
    }
}

// ===================================================================
// Menu tools
unsigned long timeStart;
unsigned long timeEnd;
unsigned long timeElapsed;


void startMainProgram( int keyEvent )
{
    int i = 0;
    float n = 0;
    float d = 0;
    float e = 0.12; // set for static 5V DC;
    n=valueDist/e*10;
    d=valueTime/n*60000;     
    float fastDistanceUnit = 509.25; // set for static 5V DC, estimation, no real measure done
    float insideTimeStart = 0;
    float insideTimeEnd = 0;
    float insideTimeElapse = 0;
    int fastMoveDelay = 0; 
    String msg = "";
    int in = n;
    int minLeft = 0;
    int dl = 0;  //dl is for delay - inside elapsed time to calculate with inside actions
    switch (keyEvent)
    {
    
        case selectKey: 
          switch(programMode)
          {
              case programTimeLapse:
                lcd.clear();
//                lcd.setCursor(0,0);
                
//                lcd.print("Program Started");
                if(motorDirection < leftDirection){
                  setMotorRightDirection();
                  lcd.setCursor(0,0);
                  lcd.print("TimeLapse Right");
                } else  {
                  setMotorLeftDirection();
                  lcd.setCursor(0,0);
                  lcd.print("TimeLapse Left");
                }                  
                timeStart = millis();
                insideTimeElapse = 0;
                
                for(i=0;i<n;i++){
                   insideTimeStart = millis();
                   minLeft = valueTime - (insideTimeStart-timeStart)/60000;                   
                   msg = "";
                   msg.concat(i);
                   msg.concat("/");
                   msg.concat(in);
                   msg.concat(" ");
                   msg.concat(minLeft);
                   stepDCMotor(); // by default it is 100 delay
                   lcd.setCursor(0,1);
                   lcd.print("                ");
                   insideTimeEnd = millis();
                   insideTimeElapse = insideTimeEnd - insideTimeStart;
                   dl = d-insideTimeElapse;
                   msg.concat(" ");
                   msg.concat(dl);
                   lcd.setCursor(0,1);
                   lcd.print(msg);
                   if(dl < 0)
                     delay(0);
                   else
                     delay(dl);
                }
                timeEnd = millis();
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Elapsed Time:");
                lcd.setCursor(0,1);
                timeElapsed=timeEnd-timeStart;
                lcd.print(timeElapsed);                
                break;
              case programFastMove:
                if(motorDirection < leftDirection){
                  setMotorRightDirection();
                  lcd.setCursor(0,0);
                  lcd.print("Fast Mv: Right");
                } else  {
                  setMotorLeftDirection();
                  lcd.setCursor(0,0);
                  lcd.print("Fast Mv: Left");
                }                  
                  fastMoveDelay = valueDist*fastDistanceUnit*10;
                  lcd.setCursor(0,1);
                  msg = "Dealy: ";
                  msg.concat(fastMoveDelay);
                  lcd.print(msg);                    
                  fastMoveDCMotor();
                  delay(fastMoveDelay);
                  stopDCMotor();
                  break;
              case programTestDistanceAW:
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Dist meas:");
                  if(motorDirection < leftDirection){
                    setMotorRightDirection();
                    lcd.setCursor(12,0);
                    lcd.print("R");
                  } else  {
                    setMotorLeftDirection();
                    lcd.setCursor(12,0);
                    lcd.print("L");
                  }
                  lcd.setCursor(0,1);
                  long delayAW = 0;
                  delayAW = valueTime * 60000;                  
                  msg = "T:";
                  msg.concat(delayAW);
                  msg.concat("msAW:");
                  msg.concat(awValue);
                  lcd.print(msg);                  
                  timeStart = millis();
                  if(testValue == 1){
                    int n;
                    n = delayAW/analogMotorDelay;
                    for(i=0;i<n;i++){
                      stepDCMotor();
                      delay(500);
                    }
                  } else {                      
                    moveDCMotorAW();
                    delay(delayAW);
                    stopDCMotor();
                  } 
                  timeEnd = millis();
                  timeElapsed=timeEnd-timeStart;
                  msg = "Time: ";
                  msg.concat(timeElapsed);                  
                  lcd.setCursor(0,1);
                  lcd.print("                ");
                  lcd.setCursor(0,1);
                  lcd.print(msg);
          }
          break;  
        }
}

void menuModeSelect( int keyEvent )
{
    switch (keyEvent)
    {
        case rightKey:
            if (programMode < 5)
                programMode++;
            break;
        case leftKey:
            if (programMode > 0)
                programMode--;
            break;
    }
    lcd.setCursor(0,1);
    lcd.print( sequenceModes[programMode] );
}

void menuOptionSelect( int keyEvent )
{
    char cbuf[4] = "   ";
    lcd.setCursor(0,1);
    lcd.print(sequenceOptions[programMode]);
    switch (keyEvent)
    {
        case rightKey:
            switch (programMode)
            {
                case programTimeLapse:
                    if (motorDirection < leftDirection)
                        motorDirection++;
                    break;                    
                case programTime:
                    if (valueTime < maxTime)
                        valueTime+=DS;
                    break;
                case programDistance:
                    if (valueDist < maxDist)
                        valueDist+=DS;
                    break;                    
                case programFastMove:
                    if (valueDist < maxDist)
                        valueDist+=DS;
                    break;    
                case programDigitStep:
                    if (DS < maxDS)
                        DS++;
                    break;  
                case programTestDistanceAW:
                    if (testValue < 1)
                        awValue++;
                    if (testValue = 1)
                        testValue = 0;
                    break;                
            }
            break;
        case leftKey:
            switch (programMode)
            {
                case programTimeLapse:
                    if (motorDirection > rightDirection)
                        motorDirection--;
                    break;
                case programTime:
                    if (valueTime > minTime)
                        valueTime-=DS;
                    break;
                case programDistance:
                    if (valueDist > minDist)
                        valueDist-=DS;
                    break;                    
                case programFastMove:
                    if (valueDist > minDist)
                        valueDist-=DS;
                    break;
                case programDigitStep:
                    if (DS > minDS)
                        DS--;
                    break;  
                  case programTestDistanceAW:
                    if (testValue > 0)
                        testValue--;
                    if (testValue = 0)
                        testValue = 1;
                    break;                       
            }
            break;
    }
    switch(programMode)
    {
        case programTimeLapse:
            if (motorDirection > rightDirection)
                lcd.print("< L");
            else
                lcd.print(" ");
            
 //           sprintf(cbuf,"%3d",motorDirection);
            if (motorDirection < leftDirection)
                lcd.print("R >");
            else
                lcd.print(" ");                
             break;
             
        case programTime:
            if (valueTime > minTime)
                lcd.print("<");
            else
                lcd.print(" ");
            sprintf(cbuf,"%3d",valueTime);
            lcd.print(cbuf);
            if (valueTime < maxTime)
                lcd.print(">");
            else
                lcd.print(" ");
            break;
        
        case programDistance:
            if (valueDist > minDist)
                lcd.print("<");
            else
                lcd.print(" ");
            sprintf(cbuf,"%2d",valueDist);
            lcd.print(cbuf);
            if (valueDist < maxDist)
                lcd.print(">");
            else
                lcd.print(" ");                
            break;
        
        case programFastMove:
            if (valueDist > minDist)
                lcd.print("<");
            else
                lcd.print(" ");
            sprintf(cbuf,"%2d",valueDist);
            lcd.print(cbuf);
            if (valueDist < maxDist)
                lcd.print(">");
            else
                lcd.print(" ");                
            break;
        
        case programDigitStep:
            if (DS > minDS)
                lcd.print("<");
            else
                lcd.print(" ");
            sprintf(cbuf,"%2d",DS);
            lcd.print(cbuf);
            if (DS < maxDS)
                lcd.print(">");
            else
                lcd.print(" ");
            break; 
 
         case programTestDistanceAW:
           if (testValue > 0)
                lcd.print("<");
            else
                lcd.print(".");
            sprintf(cbuf,"%3d",testValue);
            lcd.print(cbuf);
            if (testValue < 1)
                lcd.print(">");
            else
                lcd.print(".");
            break;               
    }
}

// ===================================================================
// Lcd tools

void clearLine(int line)
{
    lcd.setCursor(0,line);
    lcd.print("                ");
    lcd.setCursor(0,line);
}

// ===================================================================
// Define a custom char in lcd
int defineCharacter(int ascii, int *data) {
    int baseAddress = (ascii * 8) + 64;  
    // baseAddress = 64 | (ascii << 3);
    lcd.command(baseAddress);
    for (int i = 0; i < 8; i++)
        lcd.write(data[i]);
    lcd.command(128);
    return ascii;
}

// ===================================================================
// Convert ADC value to key number

int adc_key_val[NUM_KEYS] ={ 30, 150, 360, 535, 760 };
int get_key(unsigned int input)
{
    int k;
    for (k = 0; k < NUM_KEYS; k++)
    {
        if (input < adc_key_val[k])
            return k;
    }
    if (k >= NUM_KEYS)
        k = -1; // No valid key pressed
    return k;
}

// ===================================================================
// new key detection routine, without delays!

int     lastKeyEvent = 0;
int     curKeyEvent  = 0;
int     keyToReturn  = 0;
boolean keyToProcess = false;
int     adc_key_in   = 0;

int detectKey()
{
    keyToReturn = -1;
    adc_key_in = analogRead(0);         // read the value from the sensor  
    curKeyEvent = get_key(adc_key_in);      // convert into key press
    if (curKeyEvent != lastKeyEvent)
    {
        if (!keyToProcess) 
        {
            lastKeyEvent = curKeyEvent;
            keyToProcess = true;
        }
        else
        {
            keyToReturn = lastKeyEvent;
            lastKeyEvent = -1;
            keyToProcess = false;
        }
    }
    return keyToReturn;
}

// ===================================================================
