

#include <ESP32_Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
//#include <SSD1283A.h> //Hardware-specific library
#include "motors_control.h"
#include <esp32-hal-ledc.h> 
#include "html.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_SCLK 14 // SCL
#define TFT_MOSI 13 // SDA
#define TFT_RST  12 // RES (RESET)
#define TFT_DC    2 // Data Command control pin
#define TFT_CS   15 // Chip select control pin
                    // BL (back light) and VCC -> 3V3
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
#include "dispMetod.h"


const char* ssid = "CozmoV2";
const char* password = "12345678";
const float Pi = 3.141593;
#define SENSOR_PIN 15 //pin infrared input

int MotorPin1 = 26; //32; //12; Blue
int MotorPin2 = 27; //33; //13;Grey
int MotorPin3 = 33; //35; //15; green
int MotorPin4 = 32; //34; //14; Whait
int BUTTON_PIN = 35;


//------------DISP config------------------

//GFXcanvas16 canvas(130, 130); // uses heap space

// let the linker complain if not enough ram
//-------------------

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

bool button1State = false;
bool button2State = false;
bool button3State = false;
bool button4State = false;
bool button5State = false;
bool checkInf = false;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

WebServer server(80);
Servo myservo;

void handleRoot() {
  //String html = "<html><body><form>";
  //html += "<input type='range' min='0' max='180' value='90' id='servoSlider'>";
  server.send(200, "text/html", index2_html); 
}

void handleServo() {
  int angle = server.arg("angle").toInt();

  myservo.write(angle);
  
  float radianA = (angle * 71) / 4068; 
  float radianB = ((angle+60) * 71) / 4068; 
  float radianC = ((angle+120) * 71) / 4068;  
  int rCosA =round (cos (radianA)*25+65);
  int rSinA =round (sin (radianA)*25+65);
  int rCosB =round (cos (radianB)*25+65);
  int rSinB =round (sin (radianB)*25+65);
  int rCosC =round (cos (radianC)*25+65);
  int rSinC =round (sin (radianC)*25+65);
/*  
  canvas.drawRoundRect(3,3,127,127,10,YELLOW);
  canvas.fillRect(10, 10, 110, 110, BLACK);
  show_canvas_on_screen();
  canvas.fillCircle(65, 65, 25, MAGENTA);
  canvas.fillTriangle(
      rCosA ,rSinA , // peak
      rCosB ,rSinB , // bottom left
      rCosC ,rSinC, // bottom right
      YELLOW);
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println(angle);
  show_canvas_on_screen();
  
  Serial.print("cosA= "); 
  Serial.print(rCosA);
  Serial.print("sinA= ");
  Serial.println(rSinA );

  Serial.print("cosB= "); 
  Serial.print(rCosB);
  Serial.print("sinB= ");
  Serial.println(rSinB);

  Serial.print("cosC= "); 
  Serial.print(rCosC);
  Serial.print("sinC= ");
  Serial.println(rSinC);

*/  
}


void handleButton1 (){
  button1State = !button1State;
/*  
  canvas.fillRect(10, 10, 110, 110, BLACK);
  //show_canvas_on_screen();
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println("Button1=" + String(button1State));
  show_canvas_on_screen();
*/
  Serial.println("Button1 pressed");
  move_front();
  delay(1000);
  move_stop();
}

void handleButton2 (){
  button2State = !button2State;
  Serial.println("Button2 pressed");
  move_back();
  delay(1000);
  move_stop();
}

void handleButton3 (){
  button3State = !button3State;
  Serial.println("Button3 pressed");
  move_right();
  delay(1000);
  move_stop();
}

void handleButton4 (){
  button4State = !button4State;
  Serial.println("Button4 pressed");
  move_left();
  delay(1000);
  move_stop();
}

void checkInfrared(){    
  int state = digitalRead(SENSOR_PIN);
  if (state == LOW){
    checkInf = true;
    Serial.println("The obstacle is present");
    move_back();
    delay(500);
    move_stop();
  }
  else{
    checkInf = false;
    Serial.println("The obstacle is NOT present");
  }
}

void checkButton(){
  int state = digitalRead(BUTTON_PIN);
  if (state == LOW){
    button5State = true;
    Serial.println("button not pressed");
  }
  else{
    button5State = false;
    Serial.println("button pressed");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
//------------Disp--------------
  Serial.println("screen.init() done");
  tft.initR(INITR_144GREENTAB);
  Serial.println(F("Initialized"));
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
  Serial.println(time, DEC);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);
  testlines(ST77XX_BLUE);
  mediabuttons();
//---------------------------------

  
  //testMioCircles();
  server.on("/", handleRoot);
  server.on("/servo", handleServo);

  server.on("/button1",  handleButton1);
  server.on("/button2",  handleButton2);
  server.on("/button3",  handleButton3);
  server.on("/button4",  handleButton4);


  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, checkButton , FALLING);

  server.begin();
  Serial.println("Web server started");

  myservo.attach(13);
  pinMode(SENSOR_PIN, INPUT); //set pin imput used for infrared
  attachInterrupt(SENSOR_PIN, checkInfrared, FALLING);
  
//------------------------
  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  pinMode(MotorPin3, OUTPUT);
  pinMode(MotorPin4, OUTPUT);
  ledcAttachPin(MotorPin1, 5);
  ledcSetup(5, 2000, 8);      
  ledcAttachPin(MotorPin2, 6);
  ledcSetup(6, 2000, 8);
  ledcWrite(5, 0);  //gpio13
  ledcWrite(6, 0);
  
  ledcAttachPin(MotorPin3, 7);
  ledcSetup(7, 2000, 8);      
  ledcAttachPin(MotorPin4, 8);
  ledcSetup(8, 2000, 8);
  ledcWrite(7, 0);  
  ledcWrite(8, 0);
 
}

void loop() {
  server.handleClient();
}
