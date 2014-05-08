/*
 Send an email when the laundry is done
 
 Written by David Wegmuller, May 2014
 
 Based on:

  Write to file using FileIO classes.

 This sketch demonstrate how to write file into the Yún filesystem.
 A shell script file is created in /tmp, and it is executed afterwards.

 created 7 June 2010
 by Cristian Maglie 

 This example code is in the public domain.

 */
//#define DEBUG

#include <FileIO.h>
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

#define WASHER 0
#define WASHER_PIN A1
#define DRYER 1
#define DRYER_PIN A5
#define END_OF_CYCLE_TIMEOUT 60000

// States
enum
{
  stateInit_e = 0,
  stateRunning_e,
  stateUnknown_e
}; 

unsigned long counter[2];
unsigned char state[2];
const int pins[2] = {WASHER_PIN, DRYER_PIN};
const String message[2]={"washer","dryer"};

void setup()
{
  // Setup Bridge (needed every time we communicate with the Arduino Yún)
  Bridge.begin();
  // Initialize the Console
#ifdef DEBUG
  Console.begin();

  while(!Console);  // wait for console port to connect.
  Console.println("File Write Script example\n\n");
#endif 
  // Setup File IO
  FileSystem.begin();

  // Upload script used to gain network statistics  
  uploadScript();
  runScript("boot");
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Dryer:  ");
  lcd.setCursor(0, 1);
  lcd.print("Washer: ");
  
  counter[WASHER] = 0;
  counter[DRYER] = 0;
  state[WASHER] = stateInit_e;
  state[DRYER] = stateInit_e;
  pinMode(WASHER_PIN, INPUT);
  pinMode(DRYER_PIN, INPUT);
}  


void loop()
{
  unsigned char i;
  for(i = 0; i < 2; i++)
  {
    switch(state[i])
    {
      default:
      case stateInit_e:
        if(LOW == digitalRead(pins[i]))
        {
          state[i] = stateRunning_e;
          lcd.setCursor(8, 1 - i);
          lcd.print("Running");
        }
      break;
      case stateRunning_e:
        if(HIGH == digitalRead(pins[i]))
        {
          counter[i] = millis();
          state[i] = stateUnknown_e;
        }
      break;
      case stateUnknown_e:
        if(LOW == digitalRead(pins[i]))
        {
          state[i] = stateRunning_e;
        }
        if((millis() - counter[i]) > END_OF_CYCLE_TIMEOUT)
        {
          // One minute without any current pulse.
          lcd.setCursor(8, 1 - i);
          lcd.print("Done   ");  
          runScript(message[i]);    
          state[i] = stateInit_e;
        }
      break;
    }
  }
}

// this function creates a file into the linux processor that contains a shell script
// to check the network traffic of the WiFi interface
void uploadScript() 
{
  // Write our shell script in /tmp
  // Using /tmp stores the script in RAM this way we can preserve 
  // the limited amount of FLASH erase/write cycles
  File script = FileSystem.open("/tmp/sendmail.sh", FILE_WRITE);
  // Shell script header 
  script.print("#!/bin/sh\n");
  script.print("echo -e \"FROM: laundry@suds.com\nreply-to: laundry@suds.com\nThe $1 is done. \" | ssmtp -vvv test@gmail.com\n");
  script.close();  // close the file

  // Make the script executable
  Process chmod;
  chmod.begin("chmod");      // chmod: change mode
  chmod.addParameter("+x");  // x stays for executable
  chmod.addParameter("/tmp/sendmail.sh");  // path to the file to make it executable
  chmod.run();
}


// this function run the script and read the output data
void runScript(String message)
{
  // Run the script and show results on the Serial
  Process myscript;
  myscript.begin("/tmp/sendmail.sh");
  myscript.addParameter(message);
  myscript.runAsynchronously();

  String output = "";

  // read the output of the script
  while (myscript.available()) {
    output += (char)myscript.read();
  }
  // remove the blank spaces at the beginning and the ending of the string
  output.trim();
#ifdef DEBUG
  Console.println(output);
  Console.flush();
#endif
}
