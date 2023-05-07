#include <SPI.h> 
#include <MFRC522.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <Firebase.h> 
#include <FirebaseArduino.h> 
#include <FirebaseCloudMessaging.h> 
#include <FirebaseError.h> 
#include <FirebaseHttpClient.h> 
#include <FirebaseObject.h> 
#include <ESP8266WiFi.h> 
#include <FirebaseArduino.h> 
LiquidCrystal_I2C lcd(0x3F, 16, 2); 
#define FIREBASE_HOST "penpal-eb050-default-rtdb.firebaseio.com" 
#define FIREBASE_AUTH "8Bgf0ZG8pNkc9RHhOKVWxVQQEZDmNWMHgQVJz6CL" 
#define WIFI_SSID "Rice" 
#define WIFI_PASSWORD "12345678" 
#define RST_PIN D3 // Configurable, see typical pin layout above 
#define SS_PIN D4 // Configurable, see typical pin layout above 
#define buzzer_PIN 1 
#define whiteled_PIN 9 
#define greenled_PIN 10 
#define input1Pin 3 
#define input2Pin D8 
String content = ""; 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance 
void setup() 
{ 
 pinMode(greenled_PIN, OUTPUT); 
 pinMode(whiteled_PIN, OUTPUT); 
 pinMode(buzzer_PIN, OUTPUT); 
 pinMode(input2Pin, INPUT); 
 pinMode(input1Pin, INPUT); 
 lcd.begin(); 
 lcd.backlight(); 
 Serial.begin(115200); 
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
 Serial.print("connecting"); 
 while (WiFi.status() != WL_CONNECTED) 
 { 
 Serial.print("-"); 
 delay(50); 
 } 
 SPI.begin(); // Init SPI bus 
 mfrc522.PCD_Init(); // Init MFRC522 card 
 Serial.print("STATUS : CONNECTED TO "); 
 Serial.println(WiFi.localIP()); 
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
 Serial.println("FIREBASE CONNECTED"); 
 Serial.println(F("Read personal data on a MIFARE PICC:")); // shows in serial that it is ready to read 
} 
void LCDPrint(String Phrase) 
{ 
 lcd.clear(); 
 lcd.setCursor(0, 0); 
 lcd.print(Phrase); 
 delay(2000); 
} 
void scanRFID(String *ID) 
{ 
 content.clear(); 
 byte letter; 
 LCDPrint("SCAN RFID"); 
 if (!mfrc522.PICC_IsNewCardPresent()) 
 { 
 return; 
 } 
 // Select one of the cards 
 if (!mfrc522.PICC_ReadCardSerial()) 
 { 
 return; 
 } 
 Serial.print("UID tag :"); 
 for (byte i = 0; i < mfrc522.uid.size; i++) 
 { 
 Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "); 
 Serial.print(mfrc522.uid.uidByte[i], HEX); 
 content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); 
 content.concat(String(mfrc522.uid.uidByte[i], HEX)); 
 } 
 content.toUpperCase(); 
 *ID = content.substring(1); 
 delay(100); 
} 
void loop() 
{ 
 firebaseMaster(); 
 delay(1000); 
} 
void firebaseNewUser(String ID) 
{ 
 String Name = ""; 
 Serial.println("NAME : "); 
 while (Serial.available() == 0) 
 { 
 } 
 Name = Serial.readStringUntil('\n'); 
 ; 
 delay(250); 
 Serial.println(Name); 
 String details = Name + ":0"; 
 Firebase.setString(ID, details); 
 if (Firebase.failed()) 
 { 
 Serial.print("STATUS: INSERT ERROR"); 
 Serial.println(Firebase.error()); 
 return; 
 } 
 delay(1000); 
} 
String fetchIDFromConsole() 
{ 
 String ID = content.substring(1); 
 Serial.println("RFID ID : "); 
 return ID; 
} 
void firebaseCheckStatus(String ID, String *Name, int *fetchedPens) 
{ 
 String details = ""; 
 String pens = ""; 
 int i = 0; 
 details = Firebase.getString(ID); 
 if (details.length() == 0) 
 { 
 Serial.print("STATUS: ERROR IN DATABASE"); 
 return; 
 } 
 int n = details.length(); 
 for (i = 0; i < n; i++) 
 { 
 if (details[i] != ':') 
 *Name = *Name + details[i]; 
 else 
 break; 
 } 
 for (int j = i + 1; j < n; j++) 
 pens += details[j]; 
 *fetchedPens = pens.toInt(); 
} 
void printStatus(String ID, String *Name, int *fetchedPens) 
{ 
 lcd.clear(); 
 lcd.setCursor(0, 0); 
 lcd.print("Checking status"); 
 delay(3000); 
 Serial.println("\n\n"); 
 Serial.println("USER DETAILS "); 
 Serial.println("RFID NUMBER : "); 
 Serial.print(ID); 
 Serial.print("NAME OF THE EMPLOYEE : "); 
 Serial.println(*Name); 
 Serial.print("NUMBER OF PENS DISPENSED : "); 
 Serial.println(*fetchedPens); 
 Serial.println("\n\n"); 
 delay(100); 
 LCDPrint(*Name); 
 digitalWrite(whiteled_PIN, LOW); 
 delay(2000); 
} 
void dispensePen(String ID, String *Name, int *fetchedPens) 
{ 
 int newPens = *fetchedPens + 1; 
 String temp = *Name + ":" + String(newPens); 
 Firebase.setString(ID, temp); 
 lcd.clear(); 
 Serial.println("STATUS: DISPENSED SUCCESSFULLY"); 
 delay(100); 
 LCDPrint("STATUS: DISPENSED SUCCESSFULLY"); 
 delay(2000); 
 digitalWrite(greenled_PIN, LOW); 
} 
void firebaseMaster() 
{ 
 String ID = ""; 
 String Name = ""; 
 int fetchedPens = 0; 
 Serial.println("\n\n"); 
 Serial.println("--------------PEN-PAL-----------------"); 
 Serial.println("--------------------------------------"); 
 Serial.println("\n\n"); 
 Serial.println("1. INSERT AN EMPLOYEE\n2. CHECK STATUS OF AN EMPLOYEE\n3. DISPENSE A PEN\n: "); 
 while (!Serial.available()) 
 { 
 } 
 int option = Serial.parseInt(); 
 Serial.print("You have selected : "); 
 Serial.println(option); 
 switch (option) 
 { 
 case 1: 
 { 
 ID.clear(); 
 scanRFID(&ID); 
 firebaseNewUser(ID); 
 break; 
 } 
 case 2: 
 { 
 ID.clear(); 
 scanRFID(&ID); 
 delay(1000); 
 firebaseCheckStatus(ID, &Name, &fetchedPens); 
 printStatus(ID, &Name, &fetchedPens); 
 delay(1000); 
 digitalWrite(whiteled_PIN, HIGH); 
 break; 
 } 
 case 3: 
 { 
 ID.clear(); 
 scanRFID(&ID); 
 delay(1000); 
 firebaseCheckStatus(ID, &Name, &fetchedPens); 
 dispensePen(ID, &Name, &fetchedPens); 
 delay(1000); 
 digitalWrite(greenled_PIN, HIGH); 
 break; 
 } 
 default: 
 Serial.println("Wrong Choice"); 
 } 
}
