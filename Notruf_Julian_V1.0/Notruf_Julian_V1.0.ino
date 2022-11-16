#include <WiFi.h>
#include "time.h"
// Einstellungen für NTP-Server setzen
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600; 
// Set WiFi Eingangsdaten
#define WIFI_SSID "SSID"
#define WIFI_PASS "****"
 
// Setze IFTTT Webhooks Ereignisname und Schlüssel
#define IFTTT_Key "IFTTTKey!!!"
#define IFTTT_Event "esp_sms" // Name des Applets
#define IFTTT_Value1 "Moin, hier bin ich selbst!"
#define IFTTT_Value2 "Die SMS über den ESP32"
#define IFTTT_Value3 "funktioniert!"

WiFiClient client;

#include <LiquidCrystal_I2C.h>
// Anzahl der Spalten und Reihen setzen
const int lcd_spalten = 20, lcd_reihen = 4;
LiquidCrystal_I2C lcd(0x27, lcd_spalten, lcd_reihen);  
const int analogPin=39;
uint8_t a_std=12, a_min=30, va_std=12, va_min=29, va_zaehler=0;
byte Haken [8] =
{
0b00000,
0b00000,
0b00000,
0b00001,
0b00010,
0b10100,
0b01000,
0b00000
};
byte Pfeil_r [8] =
{
0b00000,
0b00100,
0b00010,
0b11111,
0b00010,
0b00100,
0b00000,
0b00000
};
byte Pfeil_l [8] =
{
0b00000,
0b00100,
0b01000,
0b11111,
0b01000,
0b00100,
0b00000,
0b00000
};

#include "Button2.h"
#define Tasterpin_rot  16
#define Tasterpin_schwarz  17
#define Tasterpin_blau  27
#define Tasterpin_gruen  14
Button2 taster_rot, taster_schwarz, taster_gruen, taster_blau;
bool alarm_setzen=false, alarm_wechsel=false, alarm_ap=false, alarm_aktiv=false;

const uint8_t led_rot=25, led_gruen=26, led_blau=12, buzzer=18;
const uint16_t note_cs6=1047, note_c8=4186;
uint8_t letzte_sek=-1, jetzt_sek, jetzt_min, jetzt_std, jetzt_tag, jetzt_mon;
uint8_t weck_std=12, weck_min=12, freq=587, channel=0, resolution=8;
uint16_t jetzt_jahr;
bool wzsz=false, rot_ea=false, gruen_ea=false, blau_ea=false;
bool wecken_setzen=false, wecken_wechsel=false, wecken_ap=false, wecken_aktiv=false;

void setup() {
  Serial.begin(115200); // Nur optional für evtl. Ausgaben
  WiFi.begin(WIFI_SSID, WIFI_PASS); 
  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
// Mit WiFi verbunden
  Serial.println();
  Serial.print("Verbunden! IP address: ");
  Serial.println(WiFi.localIP());
// Initialisieren und Zeit holen
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  aktuelle_Zeit();
//Tonausgabe vorbereiten 
  ledcSetup(channel, freq, resolution); 
  ledcAttachPin(buzzer, channel);
// Taster vorbereiten
  taster_rot.begin(Tasterpin_rot);
  taster_rot.setLongClickTime(1000);
  taster_rot.setLongClickDetectedHandler(longClickDetected); 
  taster_rot.setDoubleClickHandler(doppelklick);   
  taster_schwarz.begin(Tasterpin_schwarz);
  taster_schwarz.setLongClickTime(1000);  
  taster_schwarz.setLongClickDetectedHandler(longClickDetected);   
  taster_gruen.begin(Tasterpin_gruen);
  taster_gruen.setLongClickTime(1000);
  taster_gruen.setLongClickDetectedHandler(longClickDetected); 
  taster_gruen.setDoubleClickHandler(doppelklick);   
  taster_blau.begin(Tasterpin_blau);
  taster_blau.setLongClickTime(1000);  
  taster_blau.setLongClickDetectedHandler(longClickDetected);   
// LCD initialisieren
  lcd.init();
  lcd.backlight(); 
  lcd.createChar(0, Haken); 
  lcd.createChar(1, Pfeil_r); 
  lcd.createChar(2, Pfeil_l);     
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("Alarm -     Wecken -");
  lcd.setCursor(3, 1);
  lcd.print(":");
  lcd.setCursor(1, 1);  
  lcd.print(a_std); 
  lcd.setCursor(4, 1);  
  lcd.print(a_min);  
  lcd.setCursor(13, 1);
  lcd.print(weck_std); 
  lcd.setCursor(15, 1);
  lcd.print(":");
  lcd.setCursor(16, 1);
  lcd.print(weck_min);  
// LED-Ausgänge definieren   
  pinMode(led_rot, OUTPUT);
  pinMode(led_gruen, OUTPUT);
  pinMode(led_blau, OUTPUT); 
}
void aktuelle_Zeit(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  char zeit_Std[3];
  jetzt_std=umwandlung(3, zeit_Std, "%H", timeinfo);
  char zeit_Min[3];
  jetzt_min=umwandlung(3, zeit_Min, "%M", timeinfo);
  char zeit_Sek[3];
  jetzt_sek=umwandlung(3, zeit_Sek, "%S", timeinfo);
  char zeit_Jahr[5];
  jetzt_jahr=umwandlung(5, zeit_Jahr, "%Y", timeinfo);
  char zeit_Monat[3];
  jetzt_mon=umwandlung(3, zeit_Monat, "%m", timeinfo);  
  char zeit_Tag[3];
  jetzt_tag=umwandlung(3, zeit_Tag, "%d", timeinfo);
}
int umwandlung(int laenge, char zeit[ ], char detail[], struct tm timeinfo){
  int intzeit;
  strftime(zeit, laenge, detail, &timeinfo);
  if(laenge == 3){
    intzeit=(zeit[0]-48)*10;
    intzeit+=zeit[1]-48;  
  }
  else{
    intzeit=(zeit[0]-48)*1000;
    intzeit+=(zeit[1]-48)*100;  
    intzeit+=(zeit[2]-48)*10;
    intzeit+=zeit[3]-48;          
  }
  
return intzeit;
} 
void loop() {
  aktuelle_Zeit();
  if(letzte_sek!=jetzt_sek){
    letzte_sek=jetzt_sek;
    lcd.setCursor(0, 2);
    lcd.print("Uhrzeit:   :  :");
    ausrichten(9, 2, jetzt_std);
    ausrichten(12, 2, jetzt_min);
    ausrichten(15, 2, jetzt_sek);
    lcd.setCursor(2, 3);
    lcd.print("/  /");
    ausrichten(0, 3, jetzt_tag);
    ausrichten(3, 3, jetzt_mon);
    lcd.setCursor(6, 3);
    lcd.print(jetzt_jahr);
    if(!alarm_ap){
      rot_ea=!rot_ea;
      digitalWrite(led_rot, rot_ea);
    }
    if(!wecken_ap){
      blau_ea=!blau_ea;
      digitalWrite(led_blau, blau_ea);
    } 
    if(wecken_ap || alarm_ap) alarm_wecken();
  }
  taster_rot.loop();
  taster_schwarz.loop();
  taster_blau.loop(); 
  taster_gruen.loop();      
  if(alarm_setzen) alarm_stellen();
  if(!alarm_ap){
     lcd.setCursor(6, 0);
     lcd.print("-");  
     digitalWrite(led_gruen, LOW);
     alarm_aktiv=false;
     if(!wecken_aktiv) ledcWriteTone(channel, 0);        
  }
  else{
     lcd.setCursor(6, 0);
     lcd.write(byte(0)); 
     digitalWrite(led_gruen, HIGH); 
     digitalWrite(led_rot, LOW);         
  }
  if(wecken_setzen) wecken_stellen();
  if(!wecken_ap){
     lcd.setCursor(19, 0);
     lcd.print("-"); 
     wecken_aktiv=false; 
     if(!alarm_aktiv) ledcWriteTone(channel, 0);     
  }
  else{
     lcd.setCursor(19, 0);
     lcd.write(byte(0)); 
     digitalWrite(led_blau, LOW);         
  }
}
//
//  Alarmieren oder Weckruf
//
void alarm_wecken(){
  // Weckruf
  if(jetzt_std==weck_std && jetzt_min==weck_min && jetzt_sek==0) wecken_aktiv=true;
  if(wecken_aktiv) {
    ledcWriteTone(channel, freq);
    if(freq==note_cs6 ? freq=note_cs6 : freq=note_c8);
  }
  //Alarm Vorwarnung
  if(jetzt_std==va_std && jetzt_min==va_min && jetzt_sek==0) alarm_aktiv=true;
  if(alarm_aktiv) {
    ledcWriteTone(channel, 550);
    if(va_zaehler<61) va_zaehler+=1;
    else{
      alarm_aktiv=false;
      va_zaehler=0;
      ledcWriteTone(channel, 0); 
    //  Sende Alarmruf über IFTTT
      send_webhook(); 
      delay(1000);
      send_webhook();   
    }
  }  
}
//
//  Uhrzeit/Datum ausrichten
//
void ausrichten(int x, int y, int wert){
  lcd.setCursor(x, y); 
  if(wert<10){
    lcd.print("0");
    lcd.setCursor(x+1, y);
    lcd.print(wert);
    }
    else lcd.print(wert);  
}
//
// Alarmzeit stellen
//
void alarm_stellen(){
  while(alarm_setzen){
    if(!alarm_wechsel){
      a_std=map(analogRead(analogPin), 0, 4095, 0, 23);
      lcd.setCursor(0, 1);
      lcd.write(byte(1)); 
      lcd.setCursor(6, 1);
      lcd.print(" ");      
      if(a_std<10){
        lcd.setCursor(1, 1);
        lcd.print(0);
        lcd.setCursor(2, 1);
        lcd.print(a_std); 
      }
      else{
        lcd.setCursor(1, 1);
        lcd.print(a_std);            
      }
    }
    else{
      a_min=map(analogRead(analogPin), 0, 4095, 0, 59);
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(6, 1);
      lcd.write(byte(2));     
      if(a_min<10){
        lcd.setCursor(4, 1);
        lcd.print(0);
        lcd.setCursor(5, 1);
        lcd.print(a_min); 
      }
      else{
        lcd.setCursor(4, 1);
        lcd.print(a_min);            
      }      
    }
    taster_rot.loop();
  }
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(6, 1);
  lcd.print(" ");    
  if(a_min==0 ? va_min=59 : va_min=a_min-1);
  if(va_min==59 && a_std==0 ? va_std=23 : va_std=a_std); 
  if(alarm_wechsel) alarm_wechsel=false; 
}
//
// Weckzeit stellen
//
void wecken_stellen(){
  while(wecken_setzen){
    if(!wecken_wechsel){
      weck_std=map(analogRead(analogPin), 0, 4095, 0, 23);
      lcd.setCursor(12, 1);
      lcd.write(byte(1)); 
      lcd.setCursor(18, 1);
      lcd.print(" ");      
      if(weck_std<10){
        lcd.setCursor(13, 1);
        lcd.print(0);
        lcd.setCursor(14, 1);
        lcd.print(weck_std); 
      }
      else{
        lcd.setCursor(13, 1);
        lcd.print(weck_std);            
      }
    }
    else{
      weck_min=map(analogRead(analogPin), 0, 4095, 0, 59);
      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.setCursor(18, 1);
      lcd.write(byte(2));     
      if(weck_min<10){
        lcd.setCursor(16, 1);
        lcd.print(0);
        lcd.setCursor(17, 1);
        lcd.print(weck_min); 
      }
      else{
        lcd.setCursor(16, 1);
        lcd.print(weck_min);            
      }      
    }
    taster_gruen.loop();  
  }
  lcd.setCursor(12, 1);
  lcd.print(" ");
  lcd.setCursor(18, 1);
  lcd.print(" ");     
  if(wecken_wechsel) wecken_wechsel=false; 
}
//
//  Langer Tastendruck Taster 1
//
void longClickDetected(Button2& btn) {
  if (btn == taster_rot) alarm_setzen=!alarm_setzen;      // Alarmzeit stellen/stellen beenden
  if (btn == taster_schwarz) alarm_ap=!alarm_ap;          // Alarm einschalten/ausschalten
  if (btn == taster_blau) wecken_ap=!wecken_ap;           // Wecken einschalten/ausschalten
  if (btn == taster_gruen) wecken_setzen=!wecken_setzen;  // Weckzeit stellen/ausschalten 
}
//
//  Tastendruck Taster 2
//
void doppelklick(Button2& btn) {
  if (btn == taster_rot) alarm_wechsel=!alarm_wechsel; 
  if (btn == taster_gruen) wecken_wechsel=!wecken_wechsel;   
}
//
// Nachricht senden
//
void send_webhook(){
  // construct the JSON payload
  String jsonString = "";
  jsonString += "{\"value1\":\"";
  jsonString += IFTTT_Value1;
  jsonString += "\",\"value2\":\"";
  jsonString += IFTTT_Value2;
  jsonString += "\",\"value3\":\"";
  jsonString += IFTTT_Value3;
  jsonString += "\"}";
  int jsonLength = jsonString.length();  
  String lenString = String(jsonLength);
  // connect to the Maker event server
  client.connect("maker.ifttt.com", 80);
  // construct the POST request
  String postString = "";
  postString += "POST /trigger/";
  postString += IFTTT_Event;
  postString += "/with/key/";
  postString += IFTTT_Key;
  postString += " HTTP/1.1\r\n";
  postString += "Host: maker.ifttt.com\r\n";
  postString += "Content-Type: application/json\r\n";
  postString += "Content-Length: ";
  postString += lenString + "\r\n";
  postString += "\r\n";
  postString += jsonString; // combine post request and JSON
  
  client.print(postString);
  delay(500);
  client.stop();
  Serial.println("Nachricht gesendet!");
}
