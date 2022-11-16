#include <WiFi.h>
// Set WiFi Eingangsdaten
#define WIFI_SSID "Hier den Zugangsnamen eingeben"
#define WIFI_PASS "Hier das Passowort eingeben"
// Setze IFTTT Webhooks Ereignisname und Schlüssel
#define IFTTT_Key "Hier den key eingeben"
#define IFTTT_Event "" // Name des Applets
#define IFTTT_Value1 "Moin, dies ist ein Test!"
#define IFTTT_Value2 "Die SMS über den ESP32"
#define IFTTT_Value3 "funktioniert!"
WiFiClient client;
void setup() {
  Serial.begin(115200); // Nur optional für evtl. Ausgaben
  WiFi.begin(WIFI_SSID, WIFI_PASS); 
  // Mit WiFi verbinden...
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
}
void loop() {
    //  Sende Alarmruf über IFTTT
          send_webhook();  
          while(1);
}
// Nachricht senden
void send_webhook(){
  // konstruiere die JSON Variable zum Datenaustausch
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
