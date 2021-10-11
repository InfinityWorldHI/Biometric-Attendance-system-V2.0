/* -----------------------------------------------------------------------------
  - Project: Biometric attendance system using NodeMCU
  - Author:  https://www.youtube.com/ElectronicsTechHaIs
  - Date:  29/02/2020
   -----------------------------------------------------------------------------
  This code was created by Electronics Tech channel for 
  the Biometric attendance project with NodeMCU.
   ---------------------------------------------------------------------------*/
//*******************************libraries********************************
//NodeMCU--------------------------
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <SimpleTimer.h>           //https://github.com/jfturcot/SimpleTimer
//OLED-----------------------------
#include <SPI.h>
#include <Wire.h>
#include "icons.h"
#include <Adafruit_GFX.h>          //https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>      //https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_Fingerprint.h>  //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
//************************************************************************
//Fingerprint scanner Pins
#define Finger_Rx D7 //D5
#define Finger_Tx D6 //D6
// Declaration for SSD1306 display connected using software I2C pins are(22 SCL, 21 SDA)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     0 // Reset pin # (or -1 if sharing Arduino reset pin)
//************************************************************************
WiFiClient client;
SimpleTimer timer;
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "SSID";
const char *password = "password";
const char* device_token  = "xxxxxxxx";
//************************************************************************
String getData, Link;
String URL = "http://192.168.xxx.xxx/biometricattendancev2/getdata.php"; //computer IP or the server domain
//************************************************************************
int FingerID = 0, t1, t2;                           // The Fingerprint ID from the scanner 
bool device_Mode = false;                           // Default Mode Enrollment
bool firstConnect = false;
uint8_t id;
unsigned long previousMillis = 0;
//************************************************************************
void setup() {
  Serial.begin(115200);
  delay(1000);
  //-----------initiate OLED display-------------
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // you can delet these three lines if you don't want to get the Adfruit logo appear
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  //---------------------------------------------
  connectToWiFi();
  //---------------------------------------------
  // Set the data rate for the sensor serial port
  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
    while (1) { delay(1); }
  }
  //---------------------------------------------
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  //Timers---------------------------------------
  timer.setInterval(25000L, CheckMode);
  t1 = timer.setInterval(10000L, ChecktoAddID);      //Set an internal timer every 10sec to check if there a new fingerprint in the website to add it.
  t2 = timer.setInterval(15000L, ChecktoDeleteID);   //Set an internal timer every 15sec to check wheater there an ID to delete in the website.
  //---------------------------------------------
  CheckMode();
}
//************************************************************************
void loop() {
  timer.run();      //Keep the timer in the loop function in order to update the time as soon as possible
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
    if (millis() - previousMillis >= 10000) {
      previousMillis = millis();
      connectToWiFi();    //Retry to connect to Wi-Fi
    }
  }
  CheckFingerprint();   //Check the sensor if the there a finger.
  delay(10);
}
//************************************************************************
void CheckFingerprint(){
//  unsigned long previousMillisM = millis();
//  Serial.println(previousMillisM);
  // If there no fingerprint has been scanned return -1 or -2 if there an error or 0 if there nothing, The ID start form 1 to 127
  // Get the Fingerprint ID from the Scanner
  FingerID = getFingerprintID();
  DisplayFingerprintID();
//  Serial.println(millis() - previousMillisM);
  
}
//************Display the fingerprint ID state on the OLED*************
void DisplayFingerprintID(){
  //Fingerprint has been detected 
  if (FingerID > 0){
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    SendFingerprintID( FingerID ); // Send the Fingerprint ID to the website.
    delay(2000);
  }
  //---------------------------------------------
  //No finger detected
  else if (FingerID == 0){
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_start_bits, FinPr_start_width, FinPr_start_height, WHITE);
    display.display();
  }
  //---------------------------------------------
  //Didn't find a match
  else if (FingerID == -1){
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
  }
  //---------------------------------------------
  //Didn't find the scanner or there an error
  else if (FingerID == -2){
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
  }
}
//************send the fingerprint ID to the website*************
void SendFingerprintID( int finger ){
  Serial.println("Sending the Fingerprint ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?FingerID=" + String(finger) + "&device_token=" + device_token; // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request   //Specify content-type header
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
    
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    Serial.println(finger);     //Print fingerprint ID
  
    if (payload.substring(0, 5) == "login") {
      String user_name = payload.substring(5);
  //  Serial.println(user_name);
      
      display.clearDisplay();
      display.setTextSize(2);             // Normal 2:2 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(15,0);             // Start at top-left corner
      display.print(F("Welcome"));
      display.setCursor(0,20);
      display.print(user_name);
      display.display();
    }
    else if (payload.substring(0, 6) == "logout") {
      String user_name = payload.substring(6);
  //  Serial.println(user_name);
      
      display.clearDisplay();
      display.setTextSize(2);             // Normal 2:2 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(10,0);             // Start at top-left corner
      display.print(F("Good Bye"));
      display.setCursor(0,20);
      display.print(user_name);
      display.display();
    }
    delay(10);
    http.end();  //Close connection
  }
}
//********************Get the Fingerprint ID******************
int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return -2;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //Serial.println("Did not find a match");
    return -1;
  } else {
    //Serial.println("Unknown error");
    return -2;
  }   
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}
//******************Check if there a Fingerprint ID to delete******************
void ChecktoDeleteID(){
  Serial.println("Check to Delete ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?DeleteID=check&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request,
//    Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
  
    if (payload.substring(0, 6) == "del-id") {
      String del_id = payload.substring(6);
      Serial.println(del_id);
      http.end();  //Close connection
      deleteFingerprint( del_id.toInt() );
      delay(1000);
    }
    http.end();  //Close connection
  }
}
//******************Delete Finpgerprint ID*****************
uint8_t deleteFingerprint( int id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    //Serial.println("Deleted!");
    display.clearDisplay();
    display.setTextSize(2);             // Normal 2:2 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(F("Deleted!\n"));
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(F("Communication error!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not delete in that location");
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(F("Could not delete in that location!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(F("Error writing to flash!\n"));
    display.display();
    return p;
  } else {
    //Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    display.clearDisplay();
    display.setTextSize(2);             // Normal 2:2 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(F("Unknown error:\n"));
    display.display();
    return p;
  }   
}
//******************Check if there a Fingerprint ID to add******************
void ChecktoAddID(){
//  Serial.println("Check to Add ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?Get_Fingerid=get_id&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request,
//    Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
  
    if (payload.substring(0, 6) == "add-id") {
      String add_id = payload.substring(6);
      Serial.println(add_id);
      id = add_id.toInt();
      http.end();  //Close connection
      getFingerprintEnroll();
    }
    http.end();  //Close connection
  }
}
//******************Check the Mode*****************
void CheckMode(){
  Serial.println("Check Mode");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?Check_mode=get_mode&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request,
//    Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
  
    if (payload.substring(0, 4) == "mode") {
      String dev_mode = payload.substring(4);
      int devMode = dev_mode.toInt();
      if(!firstConnect){
        device_Mode = devMode;
        firstConnect = true;
      }
//      Serial.println(dev_mode);
      if(device_Mode && devMode){
        device_Mode = false;
        timer.disable(t1);
        timer.disable(t2);
        Serial.println("Deivce Mode: Attandance");
      }
      else if(!device_Mode && !devMode){
        device_Mode = true;
        timer.enable(t1);
        timer.enable(t2);
        Serial.println("Deivce Mode: Enrollment");
      }
      http.end();  //Close connection
    }
    http.end();  //Close connection
  }
//  Serial.print("Number of Timers: ");
//  Serial.println(timer.getNumTimers());
}
//******************Enroll a Finpgerprint ID*****************
uint8_t getFingerprintEnroll() {
  int p = -1;
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  while (p != FINGERPRINT_OK) {
          
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
      display.setTextSize(1);             // Normal 2:2 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.print(F("scanning"));
      display.display();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEMESS:
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  display.clearDisplay();
  display.setTextSize(2);             // Normal 2:2 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Remove"));
  display.setCursor(0,20);
  display.print(F("finger"));
  display.display();
  //Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
      display.setTextSize(1);             // Normal 2:2 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.print(F("scanning"));
      display.display();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      Serial.println("Fingerprints did not match");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
    return p;
  } else {
      Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    confirmAdding(id);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}
//******************Check if there a Fingerprint ID to add******************
void confirmAdding(int id){
  Serial.println("confirm Adding");
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?confirm_id=" + String(id) + "&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    
    http.begin(client, Link); //initiate HTTP request,
//    Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
    if(httpCode == 200){
      display.clearDisplay();
      display.setTextSize(1.5);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.print(payload);
      display.display();
      Serial.println(payload);
      delay(2000);
    }
    else{
      Serial.println("Error Confirm!!");      
    }
    http.end();  //Close connection
  }
}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.print(F("Connecting to \n"));
    display.setCursor(0, 50);   
    display.setTextSize(2);          
    display.print(ssid);
    display.drawBitmap( 73, 10, Wifi_start_bits, Wifi_start_width, Wifi_start_height, WHITE);
    display.display();
    
    uint32_t periodToConnect = 30000L;
    for(uint32_t StartToConnect = millis(); (millis()-StartToConnect) < periodToConnect;){
      if ( WiFi.status() != WL_CONNECTED ){
        delay(500);
        Serial.print(".");
      } else{
        break;
      }
    }
    
    if(WiFi.isConnected()){
      Serial.println("");
      Serial.println("Connected");
      
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(8, 0);             // Start at top-left corner
      display.print(F("Connected \n"));
      display.drawBitmap( 33, 15, Wifi_connected_bits, Wifi_connected_width, Wifi_connected_height, WHITE);
      display.display();
      
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    }
    else{
      Serial.println("");
      Serial.println("Not Connected");
      WiFi.mode(WIFI_OFF);
      delay(1000);
    }
    delay(1000);
}
//=======================================================================
