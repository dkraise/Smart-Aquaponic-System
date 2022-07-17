#include <DallasTemperature.h>
#include <OneWire.h>
#define ONE_WIRE_BUS 7
#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Servo.h>

//----NeoPixel----//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 16 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels

//----WiFi credential----//
char auth[] = "Zl4LNQ3Kkrb8PZyMm9nhMWQRKEdxhYdu";
char ssid[] = "khor_2.4G";
char pass[] = "T09070162613r";
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(2, 3); // RX, TX
#define ESP8266_BAUD 9600
ESP8266 wifi(&EspSerial);
Servo servo;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//----Pin attached to the sensors & output hardware----//
int ldr = A0;
int moisture = A1;
int water = A2;
int pump1 = 9;
int pump2 = 10;

//----Variable for the sensors & output hardware----//
int light_value = 0;
int moisture_value = 0;
int water_value = 0;
int pos = 0;
float Celsius = 0;
int button1 = 0;
int button2 = 0;

unsigned long currentMillis;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
int stat = 0;
int stat2 = 0;

void setup()
{
  //----Debug console----//
  Serial.begin(9600);
  delay(10);
  //----Set ESP8266 baud rate----//
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  //----Set sensors & output hardware mode----//
  pinMode(ldr, INPUT);
  pinMode(moisture, INPUT);
  pinMode(water, INPUT);
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  servo.attach(8);
  servo.write(0);
  delay(1000);
  Blynk.begin(auth, wifi, ssid, pass);
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

void loop()
{
  currentMillis = millis();
  
  //----sensors data collection every 2 s----//
  if (currentMillis - previousMillis1 >= 100) {
    previousMillis1 = currentMillis;
    sensor_check(); 
  }
  
  //----servo will rotate either 180 or 0 ever 5 minutes----//
  if (currentMillis - previousMillis2 >= 300000) {
    previousMillis2 = currentMillis;
    if (pos == 0){
      servo.write(180);
      pos = 180;
      Serial.print("Position : 180");
      delay(1000);
    }
    else{
      servo.write(0);
      pos = 0;
      Serial.print("Position : 0");
      delay(1000);
    }
  }
  ldr_stat();
  pump1_stat();
  pump2_stat();
  pump1_func();
  pump2_func();
  Blynk.run();
}

//----sensor data collection function----//
void sensor_check() {
  light_value = analogRead(ldr);
  sensors.requestTemperatures();
  moisture_value = analogRead(moisture);
  water_value = analogRead(water);
  Celsius = sensors.getTempCByIndex(0);

  Serial.print("light_value:");
  Serial.println(light_value);
  Serial.print("moisture_value:");
  Serial.println(moisture_value);
  Serial.print("water_value:");
  Serial.println(water_value);
  Serial.print("temperature:");
  Serial.println(Celsius);

  Blynk.virtualWrite(V0, light_value);
  Blynk.virtualWrite(V1, moisture_value);
  Blynk.virtualWrite(V2, water_value);
  Blynk.virtualWrite(V3, Celsius);
}

//----ldr sensor status check----//
void ldr_stat() {
  if (light_value < 120) {
    led_on();
    Serial.println("led on");
  }
  else {
    led_off();
    Serial.println("led off");
  }
}

//----pump1 status check----//
void pump1_stat() {
  if (moisture_value < 100) {
    stat = 1;
  }
  else {
    stat = 0;
  }
}

//----pump2 status check----//
void pump2_stat() {
  if (water_value < 450) {
    stat2 = 1;
  }
  else {
    stat2 = 0;
  }
}

//----blynk button virtual pin value taken----//
BLYNK_WRITE(V4)
{
  button1 = param.asInt();
  Serial.print("180: ");
  Serial.println(button1);
  if (button1 == 1) {
    pos = 180;
    servo.write(pos);
    delay(1000);
  }
}
BLYNK_WRITE(V5)
{
  button2 = param.asInt();
  Serial.print("0: ");
  Serial.println(button2);
  if (button2 == 1) {
    pos = 0;
    servo.write(pos);
    delay(1000);
  }
}

//----led NeoPixel ON function----//
void led_on() {
  pixels.clear(); // Set all pixel colors to 'off'
  for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(150, 150, 70));
    pixels.show();   // Send the updated pixel colors to the hardware.
    //delay(DELAYVAL); // Pause before next pass through loop
  }
}

//----led NeoPixel OFF function----//
void led_off() {
  pixels.clear(); // Set all pixel colors to 'off'
  for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();   // Send the updated pixel colors to the hardware.
    //delay(DELAYVAL); // Pause before next pass through loop
  }
}

//----pump1 ON/OFF function----//
void pump1_func() {
  if (stat == 1) {
    digitalWrite(pump1, LOW);
    Serial.println("pump1 on");
    stat = 0;
  }
  else {
    stat = 1;
    digitalWrite(pump1, HIGH);
    Serial.println("pump1 off");
  }
}

//----pump2 ON/OFF function----//
void pump2_func() {
  if (stat2 == 1) {
    digitalWrite(pump2, LOW);
    Serial.println("pump2 on");
    stat2 = 0;
  }
  else {
    stat2 = 1;
    digitalWrite(pump2, HIGH);
    Serial.println("pump2 off");
  }
}
