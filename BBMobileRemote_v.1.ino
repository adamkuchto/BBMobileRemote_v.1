#include <Arduino.h>

//---------------------------------------------------------------------------
// BBMobile_Remote_Switch
//---------------------------------------------------------------------------
// This is demo sketch
// you can freely change and redistribute it - dont forget to add link to our website:
// www.bbmagic.net
// To jest demonstracyjny sketch.
// możesz go dowolnie modyfikować i redystrybuować - nie zapomnij tylko o zawarciu linku:
// www.bbmagic.net
//---------------------------------------------------------------------------

#include <avr/pgmspace.h>
#include <SoftwareSerial.h>
#include "bbmobile_arduino_01.h"

// JSON definition of mobile interface - it is size optimized
const char AppJson[] PROGMEM = "{\"ty\":\"lout\",\"or\":\"V\",\"bg\":\"26,0,0\",\"cs\":[  \
{\"ty\":\"TextView\",\"te\":\"Inteligentny dom v.1\",\"tc\":\"255,255,255\",\"ts\":\"30\",\"w\":\"1\"},  \
{\"ty\":\"TextView\",\"n\":\"t1\",\"ts\":\"35\",\"tl\":\"bold\",\"w\":\"1\"}, \
{\"ty\":\"lout\",\"or\":\"H\",\"cs\":[  \
{\"ty\":\"Button\",\"n\":\"b1\",\"te\":\"ON\",\"tc\":\"255,255,255\",\"bg\":\"255,0,0\",\"ts\":\"25\"},  \
{\"ty\":\"Button\",\"n\":\"b2\",\"te\":\"OFF\",\"tc\":\"255,255,255\",\"bg\":\"0,0,0\",\"ts\":\"25\"}]}, \
{\"ty\":\"lout\",\"or\":\"H\",\"cs\":[  \
{\"ty\":\"Button\",\"n\":\"b3\",\"te\":\"ON\",\"tc\":\"255,255,255\",\"bg\":\"0,255,0\",\"ts\":\"25\"},  \
{\"ty\":\"Button\",\"n\":\"b4\",\"te\":\"OFF\",\"tc\":\"255,255,255\",\"bg\":\"0,0,0\",\"ts\":\"25\"}]}, \
{\"ty\":\"lout\",\"or\":\"H\",\"cs\":[  \
{\"ty\":\"Button\",\"n\":\"b5\",\"te\":\"ON\",\"tc\":\"255,255,255\",\"bg\":\"0,0,255\",\"ts\":\"25\"},  \
{\"ty\":\"Button\",\"n\":\"b6\",\"te\":\"OFF\",\"tc\":\"255,255,255\",\"bg\":\"0,0,0\",\"ts\":\"25\"}]}, \
{\"ty\":\"lout\",\"or\":\"H\",\"cs\":[  \
{\"ty\":\"Button\",\"n\":\"b7\",\"te\":\"ON\",\"tc\":\"255,255,255\",\"bg\":\"100,100,100\",\"ts\":\"25\"},  \
{\"ty\":\"Button\",\"n\":\"b8\",\"te\":\"OFF\",\"tc\":\"255,255,255\",\"bg\":\"0,0,0\",\"ts\":\"25\"}]}, \
{\"ty\":\"lout\",\"or\":\"H\",\"cs\":[  \
{\"ty\":\"Button\",\"n\":\"b9\",\"te\":\"ON\",\"tc\":\"255,255,255\",\"bg\":\"100,100,100\",\"ts\":\"25\"},  \
{\"ty\":\"Button\",\"n\":\"b0\",\"te\":\"OFF\",\"tc\":\"255,255,255\",\"bg\":\"0,0,0\",\"ts\":\"25\"}]}]}";

// software serial port for communication with BBMobile BLE module
SoftwareSerial bbmSerial(2, 3); // RX, TX

//PINS for BBMobile powering
#define BBMOBILE_GND_PIN 4
#define BBMOBILE_POWER_PIN 5

#define RELAY_PIN 12
#define RELAY_PIN1 11
#define RELAY_PIN2 10
#define RELAY_PIN3 9
#define RELAY_PIN4 8
// state variable values
#define S_START 0
#define S_WAIT_CONN 1
#define S_CONNECTED 2

int state = 0;
boolean rel_state;
unsigned int timer;

//--------------------------------------------------
// setup code here, to run once
//--------------------------------------------------
void setup()
{
  //initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // for powering BBMobile from defined pins
  pinMode(BBMOBILE_GND_PIN, OUTPUT);
  digitalWrite(BBMOBILE_GND_PIN, LOW); //-GND for BBMobile module
  pinMode(BBMOBILE_POWER_PIN, OUTPUT);
  digitalWrite(BBMOBILE_POWER_PIN, HIGH); //-VCC for BBMobile module

  //for relay driving
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); //-turn relay off
  rel_state = false;            //-reset flag

  //set the data rate for the Hardware Serial port
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("\n\nSTART REMOTE SWITCH");

  // set the data rate for the BBMobile SoftwareSerial port and set size of bbm_buf
  bbmSerial.begin(9600);
  bbm_buf.reserve(_SS_MAX_RX_BUFF);

  //-check serial communication with BBMobile module
  //------------------------------------------------
  Serial.print("Searching for BBMobile");
  do
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH); //-turn the LED on
    delay(500);
    digitalWrite(LED_BUILTIN, LOW); //-turn the LED off
    delay(500);
  } while (BBMobileSend(&bbmSerial, "<hello"));
  Serial.println("FOUND");

  //-set BBMobile modules name
  //----------------------------------
  Serial.print("Setting mobile name - ");
  if (BBMobileSend(&bbmSerial, "<name,ARDUINO REMOTE SWITCH"))
  {
    Serial.println("err");
  }
  else
    Serial.println("OK");

  //-set BBMobile modules PIN
  //----------------------------------
  //Serial.print("Setting PIN - ") ;
  //if( BBMobileSend(&bbmSerial, "<pin,123") )  //-set PIN to 123
  Serial.print("Setting no PIN - ");
  if (BBMobileSend(&bbmSerial, "<pin,0")) //-delete PIN
  {
    Serial.println("err");
  }
  else
    Serial.println("OK");
}

//--------------------------------------------------
// main code here, to run repeatedly
//--------------------------------------------------
void loop()
{

  delay(50);
  timer++;
  if (timer == 4)
    digitalWrite(LED_BUILTIN, LOW); //-turn the LED off
  if (timer > 20)
  {
    timer = 0;
    digitalWrite(LED_BUILTIN, HIGH); //-turn the LED on
  }

  switch (state)
  {
  case S_START:
    Serial.print("Waiting for BLE connection...");
    state = S_WAIT_CONN;
    break;

  case S_WAIT_CONN: //-no BLE connection - waiting
    BBMobileGetMessage(&bbmSerial);
    if (BBMobileIsConnected() != 0)
    {
      Serial.print("App Connected\nSending JSON...");
      if (BBMobileSendJson(&bbmSerial, AppJson))
      {
        Serial.println("err");
      }
      else
      {
        //-update status
        if (rel_state) //-if switch is on
        {
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"100,100,100\"");
        }
        else //-if switch is off
        {
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
        }
        Serial.println("OK\nPlaying with interface...");
      }
      state = S_CONNECTED;
    }
    break;

  case S_CONNECTED: //-connected to mobile App
    if (BBMobileIsConnected() == 0)
    {
      Serial.println("App disconnected");
      state = S_START;
    }
    else
    {
      //-get messages from mobile interface
      //------------------------------------
      while (BBMobileGetMessage(&bbmSerial) > 0)
      {
        Serial.println("msg: " + bbm_buf);
        if (bbm_buf.substring(4, 6) == "b1") //- after b1 button tap - turn relay ON
        {
          digitalWrite(RELAY_PIN, HIGH); //-turn relay on
          rel_state = true;              //-set flag
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"200,200,200\"");
          Serial.println("BBMOBILE SWITCH TURNED ON");
        }
        else if (bbm_buf.substring(4, 6) == "b2") //- after b2 button tap - turn relay OFF
        {
          digitalWrite(RELAY_PIN, LOW); //-turn relay of
          rel_state = false;            //-reset flag
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
          Serial.println("BBMOBILE SWITCH TURNED OFF");
        }

        else if (bbm_buf.substring(4, 6) == "b3")
        {
          digitalWrite(RELAY_PIN1, HIGH);
          rel_state = true;
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"200,200,200\"");
          Serial.println("BBMOBILE SWITCH TURNED ON");
        }
        else if (bbm_buf.substring(4, 6) == "b4")
        {
          digitalWrite(RELAY_PIN1, LOW);
          rel_state = false;

          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
          Serial.println("BBMOBILE SWITCH TURNED OFF");
        }
         else if (bbm_buf.substring(4, 6) == "b5")
        {
          digitalWrite(RELAY_PIN2, HIGH);
          rel_state = true;
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"200,200,200\"");
          Serial.println("BBMOBILE SWITCH TURNED ON");
        }
        else if (bbm_buf.substring(4, 6) == "b6")
        {
          digitalWrite(RELAY_PIN2, LOW);
          rel_state = false;

          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
          Serial.println("BBMOBILE SWITCH TURNED OFF");
        }


        /***
         */ 
          else if (bbm_buf.substring(4, 6) == "b7")
        {
          digitalWrite(RELAY_PIN3, HIGH);
          rel_state = true;
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"200,200,200\"");
          Serial.println("BBMOBILE SWITCH TURNED ON");
        }
        else if (bbm_buf.substring(4, 6) == "b8")
        {
          digitalWrite(RELAY_PIN3, LOW);
          rel_state = false;

          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
          Serial.println("BBMOBILE SWITCH TURNED OFF");
        }
          else if (bbm_buf.substring(4, 6) == "b9")
        {
          digitalWrite(RELAY_PIN4, HIGH);
          rel_state = true;
          //-send data to BBMobile
          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- ON --\"tc=\"255,255,255\",t2:bg=\"200,200,200\"");
          Serial.println("BBMOBILE SWITCH TURNED ON");
        }
        else if (bbm_buf.substring(4, 6) == "b0")
        {
          digitalWrite(RELAY_PIN4, LOW);
          rel_state = false;

          BBMobileSend(&bbmSerial, "$set,t1:te=\"-- OFF --\"tc=\"150,150,150\",t2:bg=\"0,0,0\"");
          Serial.println("BBMOBILE SWITCH TURNED OFF");
        }
      };
    }
    break;

  default:
    break;
  };
}
