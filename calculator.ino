#include <ESP8266WiFi.h>  
#include <WifiClient.h> 
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FS.h>
#include <OneButton.h>

#ifndef APSSID 
#define APSSID "TEST_AP" //CHANGE THIS
#define APPSK  "test12345" //CHANGE THIS
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

const int buttonpin1 = 14;
const int buttonpin2 = 12;
const int oledpin1 = 5;
const int oledpin2 = 4;

int digitalbutton1 = 0;
int digitalbutton2 = 0;
int Cursor = 0;
int isScreenOn = 0;
int isWifiOn = 0;

String textFile = "";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

File fsUploadFile;

const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

OneButton upButton(14, true);
OneButton downButton(12, true);

void handleFileUpload(){ 
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); 
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    
      fsUploadFile.close();                              
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);    
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    
    }
  }
  Cursor =0;
}
void handleIndexFile()
{
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleFileList()
{
  String path = "/";
  // Assuming there are no subdirectories
  Dir dir = SPIFFS.openDir(path);
  String output = "[";
  while(dir.next())
  {
    File entry = dir.openFile("r");
    // Separate by comma if there are multiple files
    if(output != "[")
      output += ",";
    output += String(entry.name()).substring(1);
    entry.close();
  }
  output += "]";
  server.send(200, "text/plain", output);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: You probably specified the wrong URL, do you mean 192.168.1.4/list?");
  }

void readDownloads()
{
  File spiffTextFile = SPIFFS.open("/data.txt", "r");
  if(spiffTextFile)
  {
    int i;
    for(i=0;i<spiffTextFile.size();i++) //Read upto complete file size
      {
        textFile += (char)spiffTextFile.read();
      }
      spiffTextFile.close();  //Close file
  }
}

void checkData() {
  Dir dir = SPIFFS.openDir ("/");
  while (dir.next ()) {
    if (dir.fileName ()== "/data.txt") {
      readDownloads();
      }
  }
  }

void refreshScreen() {
  if (handleFileUpload) {
    textFile = "";
    readDownloads();
    }
  }

void wifiOn() {
  
    Serial.print("Configuring Access Point...");
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleIndexFile);
    server.on("/fupload", HTTP_POST,[](){ server.send(200, "text/html", "<h1><font size=+5>Success, turn off the wifi please</font></h1>"); }, handleFileUpload);
    server.on("/list", HTTP_GET ,handleFileList);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void wifiOff() {
  WiFi.mode(WIFI_OFF);
  }

void wifiTurn_on() {
  if ( isWifiOn == 0 ) {
  downButton.attachDoubleClick([]() {
    wifiOn();
    Serial.println("Z has changed to 1"); //This line is for debugging
    isWifiOn = 1;
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("WIFI ON");
    display.display();
    delay(2000);
    Serial.println("Wifi_On");});
  }
}

void wifiTurn_off() {
  if ( isWifiOn == 1 ){
    downButton.attachDoubleClick([]() {
    wifiOff();
    Serial.println("Z has changed to 1"); //This line is for debugging
    isWifiOn = 1;
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("WIFI OFF");
    display.display();
    delay(2000);
    Serial.println("Wifi_OFF");});
}
    }


void textDisplay() {
  
  if (digitalbutton1 == LOW) {
    Cursor = Cursor+1;
    display.setCursor(0,Cursor);
    Serial.println(Cursor);
    display.clearDisplay();
    display.print(textFile);
    display.display();
    delay(50);
  }

  if (digitalbutton2 == LOW) {
    Cursor =Cursor-1;
    display.setCursor(0,Cursor);
    Serial.println(Cursor);
    display.clearDisplay();
    display.print(textFile);
    display.display();
    delay(50);
  }
  if (digitalbutton1 == HIGH and digitalbutton2 == HIGH) {
    display.setCursor(0,Cursor);
    display.print(textFile); 
  }
  }

void screenOff() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  Cursor = 1;
  Serial.println("DSIPLAY_OFF");
  }

void screenOn() {
  display.ssd1306_command(SSD1306_DISPLAYON);
  Cursor = 0;
  Serial.println("DISPLAY_ON");
}

void screenTurn_off() {
  if (isScreenOn == 0) {
    upButton.attachDoubleClick([]() {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.println("Y has changed to 1"); //This line is for debugging
    isScreenOn = 1;
    Serial.println(isScreenOn);
    Serial.println("DSIPLAY_OFF");});
}
}

void screenTurn_on() {
  if (isScreenOn == 1) {
    upButton.attachDoubleClick([]() {
    display.ssd1306_command(SSD1306_DISPLAYON);
    Serial.println("Y has changerd to 0"); //This line is for debugging
    isScreenOn = 0;
    Serial.println(isScreenOn);
    Cursor =0;
    textDisplay();
    Serial.println("DISPLAY_ON");});
    }
  }
  
void setup() {
  SPIFFS.begin();
  pinMode(buttonpin1, INPUT);
  pinMode(buttonpin2, INPUT);
  pinMode(oledpin1, OUTPUT);
  pinMode(oledpin2, OUTPUT);
  delay(500);
  Serial.begin(115200);
  Serial.println();
  upButton.setPressTicks(5000);
  downButton.setPressTicks(5000);
  upButton.setDebounceTicks(80);
  downButton.setDebounceTicks(80);
  upButton.setClickTicks(500);
  downButton.setClickTicks(500);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  delay(20);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  checkData();
  Serial.println(textFile);
  
}
  
void loop() {
  upButton.tick();
  downButton.tick();
  server.handleClient();
  digitalbutton1 = digitalRead(buttonpin1);
  digitalbutton2 = digitalRead(buttonpin2);
  textDisplay();
  refreshScreen();
  screenTurn_on();
  screenTurn_off();
  wifiTurn_on();
  wifiTurn_off();
}
