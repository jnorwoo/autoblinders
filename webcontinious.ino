/*
  Arduino Webserver using ESP8266
  Displays temperature in a webpage

  Arduino Mega has three Serial communication ports,this code works well with 
  Arduino Mega.For UNO users,use Softserial library to connect ESP8266 with
  Arduino UNO 
  If you're unsure about your Arduino model or its pin cofigurations,please check
  the documentation at http://www.arduino.cc
 
  modified August 2016
  By Joyson Chinta and Gladson Kujur
 */

#define DEBUG true
#include <SoftwareSerial.h>
SoftwareSerial Serial1(4, 5); // RX | TX
#include <Servo.h>

//servo globals
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
int maxpos = 180;
int STOP = 93;
int CW = 0;
int CCW = 180;

void setup()
{
  Serial.begin(9600);    ///////For Serial monitor 
  Serial1.begin(9600); ///////ESP Baud rate
  pinMode(11,OUTPUT);    /////used if connecting a LED to pin 11
  digitalWrite(11,LOW);
 
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80

  myservo.attach(3);  // attaches the servo on pin 9 to the servo object
}


float sensetemp() ///////function to sense temperature.
 {
  int val = analogRead(A0);
  float mv = ( val/1024.0)*5000; 
  float celcius = mv/10;
  return(celcius);
 }
 
int connectionId;
int trigger1 = 0;
void loop()
{
  if(Serial1.available())
  {
    /////////////////////Recieving from web browser to toggle led
    if(Serial1.find("+IPD,"))
    {
      delay(300);
      connectionId = Serial1.read()-48;
      Serial.println(connectionId);
      if(Serial1.find("servo="))
      { 
        Serial.println("recieving data from web browser trigger");
        trigger1 = 1;
      }
      String webpage = "<html><head><style>.button { background-color: #1c87c9; border: none;color: white;padding: 20px 34px;text-align: center; text-decoration: none;display: inline-block; font-size: 20px;margin: 4px 2px;cursor: pointer; }</style></head><body><a href=\"http://192.168.4.1/?servo=on\" class=\"button\">Move Servo</a></body></html>";
      espsend(webpage);
      
      String closeCommand = "AT+CIPCLOSE=";  ////////////////close the socket connection////esp command 
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      sendData(closeCommand,3000,DEBUG);
    }
  }

  sweepServo();
}
 
  //////////////////////////////sends data from ESP to webpage///////////////////////////
 
 void espsend(String d)
{
   String cipSend = " AT+CIPSEND=";
   cipSend += connectionId; 
   cipSend += ",";
   cipSend +=d.length();
   cipSend +="\r\n";
   sendData(cipSend,1000,DEBUG);
   sendData(d,1000,DEBUG); 
}

//////////////gets the data from esp and displays in serial monitor///////////////////////         
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial1.print(command);
    long int time = millis();
    while( (time+timeout) > millis())
    {
       while(Serial1.available())
          {
             char c = Serial1.read(); // read the next character.
             response+=c;
          }  
    }
    
    if(debug)
         {
         Serial.print(response); //displays the esp response messages in arduino Serial monitor
         }
    return response;
}

void sweepServo() {
  if (trigger1){
    myservo.write(CW);  // set servo to mid-point
    delay(3000);
    myservo.write(STOP);  // set servo to mid-point
    trigger1 = 0;
  }
}