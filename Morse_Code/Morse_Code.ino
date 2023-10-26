
#define rsPin 12
#define enablePin 10
#define dataPin1 5
#define dataPin2 4
#define dataPin3 3
#define dataPin4 2

char text[15]= "";
char newChar;

#include <LiquidCrystal.h>
LiquidCrystal lcd(rsPin, enablePin, dataPin1, dataPin2, dataPin3, dataPin4);



//--------------------------------//
#include <string.h>

#define TRUE 1
#define FALSE 0

//--------------------------//

#define TIMEOUT 1500 //timeout time in ms
#define STRING_LENGTH 6//
#define DOT_MAX 250 //max time for a dot, min time for a dash, (in ms)
#define BACKSPACE_MAX 400 //max time to press reset button for a backspace

const int BUTTON_PIN = 13;
const int RESET_PIN = 11;

//---------------------------------------//

int buttonState; //input from button


unsigned long inputTime;
unsigned long releaseTime;
unsigned int inputLength; //time difference between press and release

char inputString[STRING_LENGTH]; //String of "-" & "."
char nextChar[2];//char to add to string

int isTimeout;
int skipDecode;


//FONCTIONS
void receiveInput();
char Decode_Input(unsigned int);
unsigned int getTime(unsigned long);


void setup() {
    
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT); //Initialize button input pin
    pinMode(RESET_PIN, INPUT); //Reset and Backspace button
    Serial.println("Setup Complete");
    
    inputString[0] = '\0';


    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
}

void loop() {

  skipDecode = FALSE;
  
  receiveInput();
  
  //SEND inputString to function to get a letter
  Serial.print("Morse string = ");
  Serial.println(inputString);

  if(skipDecode == FALSE) //Decode if we don't have a backspace or a clear entry
    newChar = convertChar(inputString);
   else
      newChar = '\0';

  Serial.println(newChar);
  
  writeLCD(newChar);
  
  inputString[0] = '\0';
  
}

//---- FOR REST AND BACKSPACE --- //
int checkInput()
{

  if(digitalRead(BUTTON_PIN)== HIGH) //no input
  {
     return(TRUE); //
  }
  else if(digitalRead(RESET_PIN)== HIGH) //no input
  {
    return(TRUE);
  }
  else
    return(FALSE); //if there is an input
}

void readReset()
{
    inputLength = getTime(millis(), RESET_PIN);
    Serial.print(inputLength);
    Serial.println(" = RESET BUTTON");

    int nullIndex;
    nullIndex = strlen(text);
    Serial.print("nullIndex at ");
    Serial.println(nullIndex);

    if(inputLength <= BACKSPACE_MAX) //we have backspace
    {
      //remove last char
      if(nullIndex > 0)
      {
        text[nullIndex - 1] = '\0'; //Backspace
        Serial.println(text);
        writeText(); //THIS WILL BE DONE AFTER 
      }
    }
    else //input greater than BACKSPACE_MAX, we want a reset
    {
      int i;
      //clear string
      for(i = nullIndex-1; i >= 0; i = i -1)
      {
          text[i] = '\0';
      }

      writeText();
    }
}


//-----------------------------------

void receiveInput()
{
  
  isTimeout = FALSE;
  
  while(checkInput() == FALSE) //while no input, standby
    {
      //waiting for first input of a morse code character or a reset/backspace input
      //Serial.println("waiting first");
      //delay(1000);
    }

    if(digitalRead(RESET_PIN) == HIGH)
    {
      Serial.println("RESET OR BACKSPACE");
      readReset();
      isTimeout = TRUE;
      skipDecode = TRUE;
    }
    else
    {
      //we have first input
      inputTime = millis();
      inputLength = getTime(inputTime, BUTTON_PIN);
      
      nextChar[0] = Decode_Input(inputLength); //decode dot or dash
      
      strcat(inputString, nextChar); //add char to string
          
    }
    
    
    
  while(isTimeout == FALSE) //checks if input is finished, REST AND BACKSPACE NOT CONSIDERED HERE
  {
    
    //Serial.println("waiting second");
    
    if(digitalRead(BUTTON_PIN)==HIGH) //check for next input, if yes time, decode, and add to string
      {
        inputLength = getTime(millis(), BUTTON_PIN); //get time button is pressed
        nextChar[0] = Decode_Input(inputLength); //decode dot or dash
        strcat(inputString, nextChar); //add char to string
      }
      
    if(millis()-releaseTime > TIMEOUT) //CHECKS if time has expired (since last release)
    {
      isTimeout = TRUE;      
    }
      
  }
  
  Serial.println("TIMEOUT");
  //WE have entered the full desired morse code and we have the string of dots and dashes

 
}

char Decode_Input(unsigned int inputT)
  {
     if(inputT <= DOT_MAX) //if short press
      {
        Serial.print(". = ");
        Serial.println(inputT);
        return('.');
      }
      else //if long press
      {
        Serial.print("- = ");
        Serial.println(inputT);
        return('-');
      }
    
  }
  
unsigned int getTime(unsigned long t1, int pin) //get length of press, t1 is initial time
  {
        
    while(digitalRead(pin) == HIGH)
    {
      //wait for release
  
      //Serial.println("WAIT FOR RELEASE");
      
      //delay(500);
    }
    
    //Button is released

    //Serial.println("Button is released");
    releaseTime = millis(); //get release time

    //DELAY TO AVOID REPEAT DOTS
    delay(10);
    
    
    
    return(releaseTime - t1); //return the time the button was held for
  }

//---------------------//

void writeLCD(char newchar)
{
  int i;
  
  for(i=0; i<=15; i++)
  {
    if(text[i] == '\0' && newchar == ' ') 
    {
      text[i] = '_';
      text[i+1] = '\0';
      break;
    }
    
    if(text[i] == '\0') 
    {
      text[i] = newchar;
      text[i+1] = '\0';
      break;
    }
  }
  writeText();

  if (newchar == '?')
  {
    delay(1500);
    text[i] = '\0';
    writeText();
  }

  if (newchar == ' ')
  {
    delay(1000);
    text[i] = ' ';
    writeText();
  }
  
}

char convertChar(char morsecode[])
{
   
  int i;
  // assign a pointer for the letters represented in morse code
  char letters[26][STRING_LENGTH] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." };
  // assign a pointer for the numbers represented in morsecode
  char numbers[10][STRING_LENGTH] = {"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};
  // assign a pointer for the special characters represented in morsecode
  char specialChar[2][STRING_LENGTH] = {"----","fewa"};

  //Serial.print("Converting morse code to a character: ");
  //Serial.println(morsecode);

   
  
  for(i=0; i<=25; i= i + 1)
  {
    if(strcmp(morsecode, letters[i]) == 0) //letters[i] == morsecode
    {
      //Serial.println(i);
      return(65+i);
    }
  }
  for(i=0; i<=9; i++)
  {
    if(strcmp(morsecode, numbers[i]) == 0)
    {
      
      //Serial.println('i');
      Serial.println(char(48+i));
      return(48 + i);
    }
  }
  for(i=0; i<=26; i++)
  {
    if(strcmp(morsecode, specialChar[i]) == 0)
    {
      if ( i == 0)
        return(' ');

    }
  }

  return('?');
  
}

void writeText()
{
  lcd.clear();
  lcd.print(text);
  
  Serial.print("LCD SCREEN: ");
  Serial.println(text);
}
