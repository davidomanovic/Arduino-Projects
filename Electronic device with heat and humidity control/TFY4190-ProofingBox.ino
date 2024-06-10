#include <DHT_U.h> // Temperature/Humidity sensor Library
#include <Wire.h> // I2C Communications library
#include <Adafruit_GFX.h> // OLED Display library
#include <Adafruit_SSD1306.h>  // OLED Display library

// ARDUINO DIGITAL PINOUT
#define OLED_RESET -1 // Reset pin 
#define BUTTON_PIN 2  // OK button
#define INCREMENT_PIN 3 // Plus button
#define SCROLL_PIN 4 // Scroll button
#define HEAT_PIN 5 // Relay
#define DHTPIN 6   // DHT sensor
#define RED_RGB 7 // Red RGB pin
#define GREEN_RGB 8 // Green RGB pin
#define BLUE_RGB 9 // Blue RGB pin
#define LATCH_PIN 10 // 74HC595 Latch
#define DATA_PIN 11 // 74HC595 Data
#define CLOCK_PIN 12 // 74HC595 Clock

// OLED CONFIGURATION
#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//  PROGRAM CONFIGURATION
float T_c = 22; // degrees celsius 
float H_c = 40; // humidity in percent
int Time_c = 60;
int currentSelection = 0; // Display
int heatOn = LOW;
boolean registers[10]; // heatbar pins

// OUTPUT STATES
int displayState = 0; // (0 = Menu, 1 = Program)
int heatState = LOW;     
int lastButtonState;      
int currentButtonState; 
int currentScroll;
int lastScroll;
int currentIncrement;
int lastIncrement;

// Small arrow bitmap (8x8)
static const unsigned char PROGMEM arrowIcon[] = {
  0b00100, 0b01110, 0b11111,
  0b00100, 0b00100, 0b00100,
  0b00100, 0b00100,
};

// Temperature Icon bitmap (8x8)
static const unsigned char PROGMEM tempIcon[] =
{
  0b00100, 0b01010, 0b01010,
  0b01010, 0b01010, 0b10001,
  0b10001, 0b01110,
};

// Water Droplet Icon bitmap (8x8)
static const unsigned char PROGMEM waterIcon[] =
{
  0b00100, 0b00100, 0b01110, 
  0b01110, 0b11111, 0b11111,
  0b11111, 0b01110,
};
// MISC
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long previousMillis = 0; // stores time
long initialTime = 10; // Countdown time in s

// RUN CODE ON STARTUP
void setup() {
  // LCD SCREEN SETUP
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  // START TEMP&HUMIDITY SENSOR
  dht.begin();

  // SET DIGITAL PIN MODES AS I/O ARDUINO UNO
  pinMode(BUTTON_PIN, INPUT);  // DIGITAL PIN 2  
  pinMode(SCROLL_PIN, INPUT); // DIGITAL PIN 3
  pinMode(INCREMENT_PIN, INPUT); // DIGITAL PIN 4
  pinMode(HEAT_PIN, OUTPUT); // DIGITAL PIN 5
  pinMode(RED_RGB,  OUTPUT); // DIGITAL PIN 7            
  pinMode(GREEN_RGB, OUTPUT); // DIGITAL PIN 8
  pinMode(BLUE_RGB, OUTPUT); // DIGITAL PIN 9
  pinMode(DATA_PIN, OUTPUT); // DIGITAL PIN 10
  pinMode(LATCH_PIN, OUTPUT); // DIGITAL PIN 11
  pinMode(CLOCK_PIN, OUTPUT); // DIGITAL PIN 12
  // ==========================================
  currentButtonState = digitalRead(BUTTON_PIN);
  displayMenu();
}

// ___ FUNCTION SECTION ___

// CONTROL "MENU" DISPLAY STATE (HELPER FUNCTION)
void displayMenu(){

  // Display design
  display.clearDisplay();
  display.setTextSize(1);      // Double the normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);
  display.println(F("Settings"));

  // Temperature
  display.setCursor(0,16);
  if (currentSelection == 0) display.print("> "); // Arrow for selection
  else display.print("  ");
  display.println("Temp: " + String(T_c) + " C");

  // Print Humidity on display
  display.setCursor(0,32);
  if (currentSelection == 1) display.print("> "); // Arrow for selection
  else display.print("  ");
  display.println("Humidity: " + String(H_c) + "%");

  // Print Time on display
  display.setCursor(0,48);
  if (currentSelection == 2) display.print("> "); // Arrow for selection
  else display.print("  ");
  display.println("Time: " + String(Time_c) + ":00:00");
}

//
//

// VALUE CONTROLLER FOR INCREMENT BUTTON
void increment() {
  switch (currentSelection) {
    case 0: // Temperature
      T_c += 1;
      if (T_c > 32) T_c = 22;
      break;
    case 1: // Humidity
      H_c += 10; // Increment by 10 for example
      if (H_c > 80) H_c = 40;
      break;
    case 2: // Time
      Time_c += 1; // Increment by 1 hour for example
      if (Time_c > 12) Time_c = 1; // Reset to 1 if it exceeds 12
      // Convert Time_c to seconds and update initialTime
      initialTime = Time_c * 3600;
      break;
  }
}

// DISPLAY DHT11 SENSOR READINGS 
void displaySensorReadings()
{
  display.clearDisplay();
  display.setTextSize(1);      // Double the normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Display Temperature Icon
  display.drawBitmap(0, 0, tempIcon, 8, 8, SSD1306_WHITE);
  display.setCursor(14,0);     // Start text right after the icon
  display.print(F("Temp: "));
  display.print(t);
  int x = display.getCursorX() + 2; // Adjust X as needed
  int y = display.getCursorY() + 1; // Adjust Y to align with the top of the text
  int radius = 1; // Small radius for the degrees circle
  display.drawCircle(x, y, radius, SSD1306_WHITE);

  // Move the cursor to the right of the circle before printing "C"
  display.setCursor(x + 4, 0); // Adjust X to leave space after the circle
  display.println(F("C"));

  // Display Humidity Icon
  display.drawBitmap(0, 16, waterIcon, 8, 8, SSD1306_WHITE);
  display.setCursor(14,16); // Start text right after the icon
  display.print(F("Humidity: "));
  display.print(h);
  display.println(F(" %"));
}

// TIMER COUNTDOWN FOR DISPLAY
void displayCountdown() {
    if (initialTime > 0) { // Calculate time
        initialTime--;
        int hours = initialTime / 3600;
        int minutes = (initialTime % 3600) / 60;
        int seconds = initialTime % 60;

        // Display logic
        display.setTextSize(1);  
        display.setCursor(0, 42);
        display.print(F("Time left: "));
        if(hours < 10) display.print('0');
        display.print(hours);
        display.print(':');
        if(minutes < 10) display.print('0');
        display.print(minutes);
        display.print(':');
        if(seconds < 10) display.print('0');
        display.print(seconds);
        delay(1000);
    } else {
        // When time is 0
        display.setTextSize(1);
        display.setCursor(0, 42); 
        display.print(F("                "));
        display.setCursor(0, 42);
        display.print(F("Program over!"));
        delay(200);
        displayState = 0; // Return to main menu
    }
}

// DISPLAY UPDATER (MAIN LOOP FUNCTION)
void updateDisplay() 
{
    display.clearDisplay();
    if (displayState == 0) {
        RGB(0,0,255); // Blue at menu
        displayMenu(); 
        display.display();
    } else {
        RGB(0,255,0); // Green during program
        displaySensorReadings();
        displayCountdown();
        display.display();
    }
}

// BUTTON CONTROLLER (MAIN LOOP FUNCTION)
void buttons(){
    // CONFIRM BUTTON //
    lastButtonState = currentButtonState;
    currentButtonState = digitalRead(BUTTON_PIN);
    if (lastButtonState == HIGH && currentButtonState == LOW){
        // Toggle displayState between menu (0) and sensor readings (1)
        displayState ^= 1;
        heatState = !heatState;
        digitalWrite(HEAT_PIN, heatState);
    }

    // SCROLL MENU OPTIONS BUTTON //
    int scrollReading = digitalRead(SCROLL_PIN);
    if (scrollReading == LOW && lastScroll == HIGH) {
        // Add a small delay for button debounce
        delay(50);
        if (displayState == 0) {
            currentSelection = 
            (currentSelection + 1) % 3;
        }
    }
    
    lastScroll = scrollReading;
    // INCREMENT BUTTON //
    int currentIncrementReading = digitalRead(INCREMENT_PIN);
    if (lastIncrement == HIGH && currentIncrementReading == LOW){
        // Add a small delay for button debounce
        delay(50);
        increment();
    }
    lastIncrement = currentIncrementReading;
}

// HEAT ELEMENT CONTROLLER (MAIN LOOP FUNCTION)
void heatControl(){    
  if((dht.readTemperature() >= T_c || dht.readHumidity() >= H_c) && displayState == 1){
      digitalWrite(HEAT_PIN, LOW);
    }
    else if (displayState == 1)
    {
      digitalWrite(HEAT_PIN, HIGH);
    }
    else if (displayState == 0){
      digitalWrite(HEAT_PIN, LOW);
    }
}

// RGB CONTROLLER (MAIN LOOP FUNCTION)
void RGB(int r,int g,int b){
   digitalWrite(RED_RGB, r & 0x04 ? HIGH : LOW);
   digitalWrite(GREEN_RGB, g & 0x02? HIGH : LOW);
   digitalWrite(BLUE_RGB, b & 0x01 ? HIGH : LOW);
}

// Control the shift register to manipulate the led bar graph showing temperature
void writereg(){
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 9; i >= 0; i--){
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, registers[i]);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

// 10-segment LED bar logic for temperature indicator
void heatBar(){
  int T = dht.readTemperature();

  // if DHT11 reads less than 25 celsius, we only light up the first four LEDs
  if(T < 25 && displayState == 1){ 
    for (int i = 0; i < 4; i++) {
        registers[i] = HIGH; // Color = (1x blue, 3x green)
        writereg();
        delay(10);
      }
    }
    // if DHT11 reads between 25 to 28, we light up the first seven
    else if(T > 25 && T < 28  && displayState == 1){
      for (int i = 0; i < 7; i++) {
        registers[i] = HIGH; // Color = (1x blue, 3x green, 3x orange)
        writereg();
        delay(10);
      }
    }
    // anything above 28 is too hot for the dough to handle long periods
    else if((T > 28 && displayState == 1)){
      for (int i = 0; i < 10; i++) {
        registers[i] = HIGH; // Lights up all the LEDs
        writereg();
        delay(10);
      }
    }
    else if (displayState == 0){ // If you are in main menu > no light
      for(int i = 0; i < 10; i++){
        registers[i] = LOW;
        writereg();
        delay(10);
      }
    }
}
// _________ MAIN LOOP _____________________
void loop() {
    heatControl(); // Heat element controller
    buttons(); // Button UI controller
    updateDisplay(); // Display update controller
    heatBar(); // LED Bar graph controller  
}