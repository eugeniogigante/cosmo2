

#include <ESP32_Servo.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SSD1283A.h> //Hardware-specific library
#include "motors_control.h"
#include <esp32-hal-ledc.h> 
#include "html.h"
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
#if (defined(TEENSYDUINO) && (TEENSYDUINO == 147))
// for Mike's Artificial Horizon
SSD1283A screen(/*CS=*/ 10, /*DC=*/ 15, /*RST=*/ 14, /*LED=*/ -1); //hardware spi,cs,cd,reset,led

// for my wirings used for e-paper displays:
#elif defined (ESP8266)
SSD1283A screen(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*LED=D2*/ 4); //hardware spi,cs,cd,reset,led
#elif defined(ESP32)
SSD1283A screen(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*LED=*/ 4); // 18 sck 23 SDA  hardware spi,cs,cd,reset,led
#elif defined(_BOARD_GENERIC_STM32F103C_H_)
SSD1283A screen(/*CS=4*/ SS, /*DC=*/ 3, /*RST=*/ 2, /*LED=*/ 1); //hardware spi,cs,cd,reset,led
#elif defined(__AVR)
SSD1283A screen(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*LED=*/ 7); //hardware spi,cs,cd,reset,led
#else
// catch all other default
SSD1283A screen(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*LED=*/ 7); //hardware spi,cs,cd,reset,led
#endif

//GFXcanvas16 canvas(130, 130); // uses heap space

// let the linker complain if not enough ram
GFXcanvas16T<130, 130> canvas; // uses dynamic memory space


void show_canvas_on_screen()
{
  screen.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}

void show_canvas_on_screen_timed()
{
  uint32_t start = micros();
  screen.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
  uint32_t elapsed = micros() - start;
  Serial.print(F("show_canvas_on_screen    ")); Serial.println(elapsed);
}

#if !defined(ESP8266)
#define yield()
#endif

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

//----------------------------------------

// Motor A
//int motor1Pin1 = 27; 
//int motor1Pin2 = 26; 
//int enable1Pin = 14; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;



WebServer server(80);
Servo myservo;

void handleRoot() {
  String html = "<html><body><form>";
html += "<input type='range' min='0' max='180' value='90' id='servoSlider'>";
  html += "<p>Servo Angle: <span id='servoValue'>90</span></p>";
  
  html += "<p>Button 1: <span id='button1State'>" + String(button1State) + "</span></p>";
  html += "<button onclick='toggleButton1()'>Toggle Button 1</button>";
  html += "<p>Button 2: <span id='button2State'>" + String(button2State) + "</span></p>";
  html += "<button onclick='toggleButton2()'>Toggle Button 2</button>";
  html += "<p>Button 3: <span id='button3State'>" + String(button3State) + "</span></p>";
  html += "<button onclick='toggleButton3()'>Toggle Button 3</button>";
  html += "<p>Button 4: <span id='button4State'>" + String(button4State) + "</span></p>";
  html += "<button onclick='toggleButton4()'>Toggle Button 4</button>";


  /*
<div class="parent">
<div class="div1"> </div>
<div class="div2"> </div>
<div class="div3"> </div>
<div class="div4"> </div>
<div class="div5"> </div>
<div class="div6"> </div>
<div class="div7"> </div>
<div class="div8"> </div>
<div class="div9"> </div>
</div>
   */

  
  html += "</form></body>";
  html += "<script>";
  html += "document.querySelector('#servoSlider').addEventListener('input', function() {";
  html += "  document.querySelector('#servoValue').textContent = this.value;";
  html += "  fetch('servo?angle=' + this.value);";
  html += "});";

  html += "function toggleButton1() {";
  html += "  fetch('/button1');";
  html += "}";
  html += "function toggleButton2() {";
  html += "  fetch('/button2');";
  html += "}";
  html += "function toggleButton3() {";
  html += "  fetch('/button3');";
  html += "}";
  html += "function toggleButton4() {";
  html += "  fetch('/button4');";
  html += "}";
  
  html += "</script></html>";
  //server.send(200, "text/html", html); //index2_html
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

  
  delay(15);
  if (angle==0){
    //moveForWard();
    delay(50);
    //stopDC();
  }
  else if (angle==180) {
    //moveBackWard();
    delay(50);
    //stopDC(); 
  }
}


void handleButton1 (){
  button1State = !button1State;
  canvas.fillRect(10, 10, 110, 110, BLACK);
  //show_canvas_on_screen();
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println("Button1=" + String(button1State));
  show_canvas_on_screen();
  Serial.println("Button1 pressed");
  move_front();
  delay(1000);
  move_stop();
}

void handleButton2 (){
  button2State = !button2State;
  canvas.fillRect(10, 10, 110, 110, BLACK);
  //show_canvas_on_screen();
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println("Button2=" + String(button2State));
  show_canvas_on_screen();
  Serial.println("Button2 pressed");
  move_back();
  delay(1000);
  move_stop();
}

void handleButton3 (){
  button3State = !button3State;
  canvas.fillRect(10, 10, 110, 110, BLACK);
  //show_canvas_on_screen();
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println("Button3=" + String(button3State));
  show_canvas_on_screen();
  Serial.println("Button3 pressed");
  move_right();
  delay(1000);
  move_stop();
}

void handleButton4 (){
  button4State = !button4State;
  canvas.fillRect(10, 10, 110, 110, BLACK);
  //show_canvas_on_screen();
  canvas.setCursor(55,60);
  canvas.setTextColor(RED);  
  canvas.setTextSize(1);
  canvas.println("Button4=" + String(button4State));
  show_canvas_on_screen();
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
  screen.init();
  Serial.println("screen.init() done");
  testMioCircles();
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

  //int MotorPin1 = 32; //12;
  //int MotorPin2 = 33; //13;
  //int MotorPin3 = 35; //15;
  //int MotorPin4 = 34; //14;
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

/*
void moveForWard(){
// Move the DC motor forward at maximum speed
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH); 
  Serial.println("MoveForWard");
  delay(500);
}

void stopDC(){
  // Stop the DC motor
  Serial.println("Motor stopped");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  Serial.println("Stop");
  delay(500);
}

void moveBackWard(){
  // Move DC motor backwards at maximum speed
  Serial.println("Moving Backwards");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); 
  Serial.println("MoveBackWard");
  delay(500);
}

void moveForwardRun(){
  // Move DC motor forward with increasing speed
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  while (dutyCycle <= 255){
    ledcWrite(pwmChannel, dutyCycle);   
    Serial.print("Forward with duty cycle: ");
    Serial.println(dutyCycle);
    dutyCycle = dutyCycle + 5;
    delay(500);
  }
  dutyCycle = 200;
}  

*/

void loop() {
  server.handleClient();
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  canvas.fillScreen(BLACK);
  show_canvas_on_screen();
  yield();
  //if (controller.ID == 0x8875) delay(200); // too fast to be seen
  canvas.fillScreen(RED);
  show_canvas_on_screen();
  yield();
  //if (controller.ID == 0x8875) delay(200); // too fast to be seen
  canvas.fillScreen(GREEN);
  show_canvas_on_screen();
  yield();
  //if (controller.ID == 0x8875) delay(200); // too fast to be seen
  canvas.fillScreen(BLUE);
  show_canvas_on_screen();
  yield();
  //if (controller.ID == 0x8875) delay(200); // too fast to be seen
  canvas.fillScreen(BLACK);
  show_canvas_on_screen();
  yield();
  return micros() - start;
}

unsigned long testText() {
  canvas.fillScreen(BLACK);
  unsigned long start = micros();
  canvas.setCursor(0, 0);
  canvas.setTextColor(WHITE);  canvas.setTextSize(1);
  canvas.println("Hello World!");
  canvas.setTextColor(YELLOW); canvas.setTextSize(2);
  canvas.println(1234.56);
  canvas.setTextColor(RED);    canvas.setTextSize(3);
  canvas.println(0xDEADBEEF, HEX);
  canvas.println();
  canvas.setTextColor(GREEN);
  canvas.setTextSize(5);
  canvas.println("Groop");
  canvas.setTextSize(2);
  canvas.println("I implore thee,");
  canvas.setTextSize(1);
  canvas.setTextColor(GREEN);
  canvas.println("my foonting turlingdromes.");
  canvas.println("And hooptiously drangle me");
  canvas.println("with crinkly bindlewurdles,");
  canvas.println("Or I will rend thee");
  canvas.println("in the gobberwarts");
  canvas.println("with my blurglecruncheon,");
  canvas.println("see if I don't!");
  show_canvas_on_screen();
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = canvas.width(),
                h = canvas.height();

  canvas.fillScreen(BLACK);
  yield();

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  t     = micros() - start; // fillScreen doesn't count against timing

  canvas.fillScreen(BLACK);
  yield();

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  show_canvas_on_screen();
  t    += micros() - start;

  canvas.fillScreen(BLACK);
  yield();

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  show_canvas_on_screen();
  t    += micros() - start;

  canvas.fillScreen(BLACK);
  yield();

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) canvas.drawLine(x1, y1, x2, y2, color);
  yield();
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = canvas.width(), h = canvas.height();

  canvas.fillScreen(BLACK);
  yield();
  start = micros();
  for (y = 0; y < h; y += 5) canvas.drawFastHLine(0, y, w, color1);
  yield();
  for (x = 0; x < w; x += 5) canvas.drawFastVLine(x, 0, h, color2);
  yield();
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = canvas.width()  / 2,
                cy = canvas.height() / 2;

  canvas.fillScreen(BLACK);
  n     = min(canvas.width(), canvas.height());
  start = micros();
  for (i = 2; i < n; i += 6) {
    i2 = i / 2;
    canvas.drawRect(cx - i2, cy - i2, i, i, color);
  }
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = canvas.width()  / 2 - 1,
                cy = canvas.height() / 2 - 1;

  canvas.fillScreen(BLACK);
  n = min(canvas.width(), canvas.height());
  for (i = n; i > 0; i -= 6) {
    i2    = i / 2;
    start = micros();
    canvas.fillRect(cx - i2, cy - i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    canvas.drawRect(cx - i2, cy - i2, i, i, color2);
    yield();
  }
  show_canvas_on_screen();

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = canvas.width(), h = canvas.height(), r2 = radius * 2;

  canvas.fillScreen(BLACK);
  start = micros();
  for (x = radius; x < w; x += r2) {
    for (y = radius; y < h; y += r2) {
      canvas.fillCircle(x, y, radius, color);
    }
    yield();
  }
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testMioCircles() {
  canvas.fillCircle(65, 65, 25, MAGENTA);
  canvas.fillTriangle(
      45 ,45 , // peak
      45, 85, // bottom left
      80 , 65, // bottom right
      YELLOW);
  show_canvas_on_screen();
}
unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                      w = canvas.width()  + radius,
                      h = canvas.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for (x = 0; x < w; x += r2) {
    for (y = 0; y < h; y += r2) {
      canvas.drawCircle(x, y, radius, color);
    }
    yield();
  }
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = canvas.width()  / 2 - 1,
                      cy = canvas.height() / 2 - 1;

  canvas.fillScreen(BLACK);
  n     = min(cx, cy);
  start = micros();
  for (i = 0; i < n; i += 5) {
    canvas.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      screen.color565(0, 0, i));
  }
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = canvas.width()  / 2 - 1,
                   cy = canvas.height() / 2 - 1;

  canvas.fillScreen(BLACK);
  start = micros();
  for (i = min(cx, cy); i > 10; i -= 5) {
    start = micros();
    canvas.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                        screen.color565(0, i, i));
    t += micros() - start;
    canvas.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                        screen.color565(i, i, 0));
    yield();
  }
  show_canvas_on_screen();

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = canvas.width()  / 2 - 1,
                cy = canvas.height() / 2 - 1;

  canvas.fillScreen(BLACK);
  w     = min(canvas.width(), canvas.height());
  start = micros();
  for (i = 0; i < w; i += 6) {
    i2 = i / 2;
    canvas.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, screen.color565(i, 0, 0));
  }
  show_canvas_on_screen();

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = canvas.width()  / 2 - 1,
                cy = canvas.height() / 2 - 1;

  canvas.fillScreen(BLACK);
  start = micros();
  for (i = min(canvas.width(), canvas.height()); i > 20; i -= 6) {
    i2 = i / 2;
    canvas.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, screen.color565(0, i, 0));
    yield();
  }
  show_canvas_on_screen();

  return micros() - start;
}

#if 0
void testEllipses()
{
  canvas.fillScreen(BLACK);
  for (int i = 0; i < 40; i++)
  {
    int rx = random(60);
    int ry = random(60);
    int x = rx + random(480 - rx - rx);
    int y = ry + random(320 - ry - ry);
    canvas.fillEllipse(x, y, rx, ry, random(0xFFFF));
  }
  show_canvas_on_screen();
  delay(2000);
  canvas.fillScreen(BLACK);
  for (int i = 0; i < 40; i++)
  {
    int rx = random(60);
    int ry = random(60);
    int x = rx + random(480 - rx - rx);
    int y = ry + random(320 - ry - ry);
    canvas.drawEllipse(x, y, rx, ry, random(0xFFFF));
  }
  show_canvas_on_screen();
  delay(2000);
}

void testCurves()
{
  uint16_t x = canvas.width() / 2;
  uint16_t y = canvas.height() / 2;
  canvas.fillScreen(BLACK);

  canvas.drawEllipse(x, y, 100, 60, PURPLE);
  canvas.fillCurve(x, y, 80, 30, 0, CYAN);
  canvas.fillCurve(x, y, 80, 30, 1, MAGENTA);
  canvas.fillCurve(x, y, 80, 30, 2, YELLOW);
  canvas.fillCurve(x, y, 80, 30, 3, RED);
  delay(2000);
  show_canvas_on_screen();

  canvas.drawCurve(x, y, 90, 50, 0, CYAN);
  canvas.drawCurve(x, y, 90, 50, 1, MAGENTA);
  canvas.drawCurve(x, y, 90, 50, 2, YELLOW);
  canvas.drawCurve(x, y, 90, 50, 3, RED);
  canvas.fillCircle(x, y, 30, BLUE);
  delay(5000);
  show_canvas_on_screen();
}
#endif
