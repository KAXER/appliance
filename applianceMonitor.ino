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

// States
enum
{
  stateInit_e = 0,
  stateRunning_e,
  stateUnknown_e,
  stateDone_e
}; 

unsigned long counter[2];
unsigned char state[2];

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
  runScript();
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world.");
  
  counter[WASHER] = 0;
  counter[DRYER] = 0;
  state[WASHER] = stateInit_e;
  state[DRYER] = stateInit_e;
  pinMode(WASHER_PIN, INPUT);
  pinMode(DRYER_PIN, INPUT);
}  

void enterInit(unsigned char appliance)
{
  counter[appliance] = 0;
}

void enterRunning(unsigned char appliance)
{
  counter[appliance] = millis();
}

void enterUnknown(unsigned char appliance)
{
}

void enterDone(unsigned char appliance)
{
}

void loop()
{
#ifdef DEBUG
  Console.println("still alive...");
#endif
  delay(5000);
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
  script.print("echo -e \"FROM: test@example.com\nreply-to: test@example.com\nHello, world \" | ssmtp -vvv test@gmail.com\n");
  script.close();  // close the file

  // Make the script executable
  Process chmod;
  chmod.begin("chmod");      // chmod: change mode
  chmod.addParameter("+x");  // x stays for executable
  chmod.addParameter("/tmp/sendmail.sh");  // path to the file to make it executable
  chmod.run();
}


// this function run the script and read the output data
void runScript()
{
  // Run the script and show results on the Serial
  Process myscript;
  myscript.begin("/tmp/sendmail.sh");
  myscript.run();

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
