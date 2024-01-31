/* Author: Remi / RxmTaco https://github.com/RxmTaco
MIT License

Copyright (c) 2024 Remi Dubrovics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Dependencies
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <ESPmDNS.h>

// Custom headers
#include "Pix_Ascii.h"

// RESET pin
#define RST_PIN         7
#define RST_SRC         8

// LED options
#define LED_PIN         3                   // Data output pin for LED display
#define LED_ROWS        8                   // Number of rows in the matrix
#define LED_COLS        32                  // Number of columns in the matrix
#define LED_NUM         (LED_ROWS*LED_COLS)

// WIFI options
const uint8_t PORT = 80;                    // Server port (HTTP default 80)
const int keyIndex = 0;                     // Needed for WEP
const char* hostname = "esp";               // Name for the host device. You can connect to the esp with <hostname>.local
const IPAddress local_ip(192,168,1,1);      // Server IP
const IPAddress gateway(192,168,1,1);       // Gateway IP
const IPAddress subnet(255,255,255,0);      // Subnet mask

// Preferences can be reset to "factory" by shorting pin 5 to GND
// Settings structure
typedef struct {
  String  submittedText;  // The initial text to be shown
  uint    scrollDelay;    // Time between column change while scrolling in milliseconds
  uint8_t flipped;        // Flipped text
  uint8_t textR;          // Text color RGB values
  uint8_t textG;
  uint8_t textB;
  uint8_t bgR;            // Background color RGB values
  uint8_t bgG;
  uint8_t bgB;
  uint8_t gap;
  String  ssid;           // Default SSID:      ESP-32-HaalariLED
  String  passwd;         // Default Password:  12345678
} settings_t;

settings_t settings;

Preferences prefs;

WiFiServer server(PORT);

Adafruit_NeoPixel pixels(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Convert hostname to lower case
  String htemp = (String)hostname;
  htemp.toLowerCase();
  hostname = htemp.c_str();

  pinMode(RST_PIN, INPUT_PULLDOWN);
  pinMode(RST_SRC, OUTPUT);
  digitalWrite(RST_SRC, HIGH);      // Write high at setup to keep rst source positive

  pixels.begin();
  pixels.clear();

  initPrefs(prefs, settings); // Load / initialize variables from flash

  // Set all leds to 20,20,20
  for(int i = 0; i < LED_NUM; i ++){
    pixels.setPixelColor(i, pixels.Color(1, 1, 1));
    pixels.show();
    delay(1);
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAPsetHostname(hostname);
  WiFi.softAP(settings.ssid, settings.passwd);

  server.begin();

  // DNS resolver
  MDNS.begin(hostname);
}

bool wrongpw = false;
bool credchanged = false;
void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
void loop() {
  if(digitalRead(RST_PIN) == HIGH){
    prefs.begin("configs");
    prefs.clear();
    resetFunc(); // call reset
  }

  if(credchanged){
    credchanged = false;
    resetFunc(); // call reset on credential change
  }

  WiFiClient client = server.available(); // Check if a client has connected

  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\0');
        client.readStringUntil('\0');
        // Read the content length from the request
        int contentLength = client.parseInt();

        // Extract parameters from POST data
        extractParameters(request);
        
        // GET request handling
        // Send a response to the client
        client.println("HTTP/1.1 200 OK");  // respond 200 for OK
        client.println("Content-Type: text/html");  // Return page HTML in the response
        client.println("");
        client.println("<html><body>");
        client.println("<div id='cont'>");
        client.println("<form action='submit' method='post'>"); // Form parameters
        client.printf("Text: <input type='text' name='inputText' placeholder='%s'>", settings.submittedText); client.println("<br>");
        client.println("<br>");
        client.printf("Scroll Delay: <input type='text' name='scrollDelay' placeholder='%i'>", settings.scrollDelay); client.println("<br>");
        client.printf("Letter gap: <input type='text' name='gap' placeholder='%i'>", settings.gap); client.println("<br>");
        client.printf("Flipped: <input type='checkbox' name='flip' %s>", settings.flipped?"checked":""); client.println("<br>");
        client.println("<br>");
        client.println("Color values 0 - 255");
        client.println("<br>");
        client.println("Text color properties:");
        client.println("<br>");
        client.printf("Text R: <input type='text' name='colorR' placeholder='%i'>", settings.textR); client.println("<br>");
        client.printf("Text G: <input type='text' name='colorG' placeholder='%i'>", settings.textG); client.println("<br>");
        client.printf("Text B: <input type='text' name='colorB' placeholder='%i'>", settings.textB); client.println("<br>");
        client.println("<br>");
        client.println("Background:"); 
        client.println("<br>");
        client.printf("Background R: <input type='text' name='bgR' placeholder='%i'>", settings.bgR); client.println("<br>");
        client.printf("Background G: <input type='text' name='bgG' placeholder='%i'>", settings.bgG); client.println("<br>");
        client.printf("Background B: <input type='text' name='bgB' placeholder='%i'>", settings.bgB); client.println("<br>");
        client.println("<br>");
        client.println("WiFi:"); 
        client.println("<br>");
        client.println("New SSID: <input type='text' name='ssid'>"); client.println("<br>");
        client.println("New Password: <input type='password' name='passwd'>"); client.println("<br>");
        client.println("Old Password: <input type='password' name='passwdold'>"); client.println("<br>");
        if(wrongpw){
          client.println("WRONG PASSWORD<br>");
          wrongpw = false;
        }
        client.println("<input type='submit' value='Submit'>");
        client.println("</form>");
        client.println("</div>");
        client.println("<br>");
        //client.println(request);  // Display request for debugging
        client.println("<br>");
        client.println("<style>");  // CSS styling
        client.println("*{background:#131313;color:#fff;}body{scale:2;width:100%;height:100vh;display:flex;justify-content:center;align-items:center;}");
        client.println("#cont{border:1px solid #ff00dd;padding:20px;border-radius:20px;box-shadow:0 0 10px #ff00dd;}");
        client.println("</style>");
        client.println("</body></html>");
        break; // Exit the loop after sending the response
      }
    }
    delay(100); // Small delay to allow the client to process the response
    client.stop(); // Disconnect the client
  }
  savePrefs(prefs, settings);
  displayText(settings.submittedText);
}

uint8_t updateIndex = 0;
unsigned long currentMillis = 0;

void displayText(String text) {

  if(millis() - currentMillis > settings.scrollDelay){
    pixels.clear();

    int charNum = text.length();
    uint8_t cols = 5 + settings.gap; // 5 for character and 1 for a blank space between characters

    // create display matrix buffer
    int columnLimit = (charNum * cols);
    int dispMatrix[ROWS][columnLimit]; // make a 2D array of the enabled pixels

    // fill matrix buffer
    for(int k = 0; k < text.length(); k++){ // iterate over characters
      // get the matrix representing the current character
      const uint8_t (&currentChar)[ROWS][5] = getPixelMatrix(text.charAt(k));
      //int colIndex = k * cols; // shift index for each character

      for(int i = 0; i < cols; i++){
        for(int j = 0; j < ROWS; j++){
          if(i >= cols - settings.gap)
            dispMatrix[j][k * cols + i] = 0;
          else
            dispMatrix[j][k * cols + i] = currentChar[j][i];
        }
      }
    }

    // Flip the matrix if settings.flipped is 1
    if(settings.flipped){
      for (int i = 0; i < ROWS / 2; i++) {
        for (int j = 0; j < columnLimit; j++) {
          // Swap elements from top and bottom rows
          int temp = dispMatrix[i][j];
          dispMatrix[i][j] = dispMatrix[ROWS - 1 - i][columnLimit - 1 - j];
          dispMatrix[ROWS - 1 - i][columnLimit - 1 - j] = temp;
        }
      }
    }

    // Check if the generated matrix is larger than the display to enable scrolling
    if(columnLimit > LED_COLS){
      for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < LED_COLS; j++){
          int sourceColumn = (j + updateIndex) % columnLimit;
          int row;
          if (j % 2 == 0)
            row = i;
          else
            row = ROWS - 1 - i;
          int enabled = dispMatrix[row][sourceColumn];
          pixels.setPixelColor(i + j * ROWS, pixels.Color(
            (enabled)? settings.textR : settings.bgR, 
            (enabled)? settings.textG : settings.bgG,
            (enabled)? settings.textB : settings.bgB));
        }
      }
    } else { // static display
      // display
      for(int i = 0; i < cols * charNum; i++){ // horizontal
        for(int j = 0; j < ROWS; j++){ // vertical
          int row;
          if(i % 2 == 0)
            row = j;
          else
            row = ROWS - 1 - j;
          int enabled = dispMatrix[row][i];
          pixels.setPixelColor(j + i * ROWS, pixels.Color(
            (enabled)? settings.textR : settings.bgR, 
            (enabled)? settings.textG : settings.bgG,
            (enabled)? settings.textB : settings.bgB));
        }
      }
    }

    pixels.show();
    if(settings.flipped){
      if(updateIndex == 0)
        updateIndex = columnLimit;
      else
        updateIndex--;
    } else {
      if(updateIndex == columnLimit)
        updateIndex = 0;
      else
        updateIndex++;
    }
    currentMillis = millis();
  }
}

// Decode special characters from the URL
String urldecode(const String &input) {
  String decoded = "";
  char a, b;
  for (size_t i = 0; i < input.length(); i++) {
    if (input[i] == '%') {
      // Decode the % encoded value
      a = input[i + 1];
      b = input[i + 2];
      decoded += char(hexToDec(a) * 16 + hexToDec(b));
      i += 2; // Skip two characters after %
    } else if (input[i] == '+') {
      // Replace '+' with space
      decoded += ' ';
    } else if (input[i] == '\'') {
      // Keep single quote unchanged
      decoded += '\'';
    } else {
      // Keep other characters unchanged
      decoded += input[i];
    }
  }
  return decoded;
}

int hexToDec(char c) {
  return (c >= '0' && c <= '9') ? c - '0' : c - 'A' + 10;
}

// Extract the passed parameters from the HTTP POST request
void extractParameters(String request) {
  // Extract the value for "inputText"
  int textPosition = request.indexOf("inputText="); // Search for the index of the parameter
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition); // Parameters are separated by an ampersand, so read until one appears
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 10, nextParameterPosition);  // Apply the value to a global parameter
    if (!inputValue.isEmpty()) {                                                      // If the parameter is null, continue to next
      settings.submittedText = urldecode(inputValue);
    }
  }

  // Extract the value for "scrollDelay"
  textPosition = request.indexOf("scrollDelay=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 12, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.scrollDelay = inputValue.toInt();
    }
  }

  // extract value for text gap enable
  textPosition = request.indexOf("gap=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 4, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.gap = inputValue.toInt();
    }
  }

  // extract value for text flip enable
  textPosition = request.indexOf("flip=");
  int post = request.indexOf("POST");
  if(post != -1){
    if (textPosition != -1){
      settings.flipped = 1;
    } else {
      settings.flipped = 0;
    }
  }

  // Extract the value for "colorR"
  textPosition = request.indexOf("colorR=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 7, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.textR = inputValue.toInt();
    }
  }

  // Extract the value for "colorG"
  textPosition = request.indexOf("colorG=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 7, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.textG = inputValue.toInt();
    }
  }

  // Extract the value for "colorB"
  textPosition = request.indexOf("colorB=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 7, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.textB = inputValue.toInt();
    }
  }

  // Extract the value for "bgR"
  textPosition = request.indexOf("bgR=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 4, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.bgR = inputValue.toInt();
    }
  }

  // Extract the value for "bgG"
  textPosition = request.indexOf("bgG=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 4, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.bgG = inputValue.toInt();
    }
  }

  // Extract the value for "bgB"
  textPosition = request.indexOf("bgB=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 4, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      settings.bgB = inputValue.toInt();
    }
  }

  // WiFi
  // Password
  bool pwset = false;
  textPosition = request.indexOf("passwdold=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 10, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      if(inputValue != settings.passwd){
        wrongpw = true;
      }
      pwset = true;
    }

    // continue to change password
    textPosition = request.indexOf("passwd=");
    if (textPosition != -1){
      int nextParameterPosition = request.indexOf("&", textPosition);
      if (nextParameterPosition == -1) {
        nextParameterPosition = request.length();
      }
      String inputValue = request.substring(textPosition + 7, nextParameterPosition);
      if (!inputValue.isEmpty()) {
        if(!wrongpw && pwset){
          settings.passwd = inputValue; // Change passwd on success
          credchanged = true;
        }
        else wrongpw = true;
      }
    }

    // continue to check for a changed SSID
    textPosition = request.indexOf("ssid=");
    if (textPosition != -1){
      int nextParameterPosition = request.indexOf("&", textPosition);
      if (nextParameterPosition == -1) {
        nextParameterPosition = request.length();
      }
      String inputValue = request.substring(textPosition + 5, nextParameterPosition);
      if (!inputValue.isEmpty()) {
        if(!wrongpw && pwset){
          settings.ssid = inputValue;
          credchanged = true;
        }
        else wrongpw = true;
      }
    }
  }
}

void savePrefs(Preferences& pref, settings_t& set){
  prefs.begin("configs", false); // Open preferences: settings namespace, false for RW mode

  pref.putString("submittedText", set.submittedText);
  pref.putUInt("scrollDelay", set.scrollDelay);
  pref.putUInt("gap", set.gap);
  pref.putUInt("flipped", set.flipped);

  pref.putUInt("textR", set.textR);
  pref.putUInt("textG", set.textG);
  pref.putUInt("textB", set.textB);

  pref.putUInt("bgR", set.bgR);
  pref.putUInt("bgG", set.bgG);
  pref.putUInt("bgB", set.bgB);

  pref.putString("ssid", set.ssid);
  pref.putString("passwd", set.passwd);

  pref.end();
}

void initPrefs(Preferences& pref, settings_t& set){
  prefs.begin("configs", false); // Open preferences: settings namespace, false for RW mode

  // WIFI
  if(not pref.isKey("ssid")) {
    set.ssid = "ESP-32-HaalariLED";
    pref.putString("ssid", set.ssid);
  } else {
    set.ssid = pref.getString("ssid");
  }

  if(not pref.isKey("passwd")) {
    set.passwd = "12345678";
    pref.putString("passwd", set.passwd);
  } else {
    set.passwd = pref.getString("passwd");
  }

  // TEXT
  if(not pref.isKey("submittedText")) {
    set.submittedText = "<SOURCE>   ";
    pref.putString("submittedText", set.submittedText);
  } else {
    set.submittedText = pref.getString("submittedText");
  }

  if(not pref.isKey("scrollDelay")) {
    set.scrollDelay = 150;
    pref.putUInt("scrollDelay", set.scrollDelay);
  } else {
    set.scrollDelay = pref.getUInt("scrollDelay");
  }

  if(not pref.isKey("gap")) {
    set.gap = 1;
    pref.putUInt("gap", set.gap);
  } else {
    set.gap = (uint8_t)pref.getUInt("gap");
  }

  if(not pref.isKey("flipped")) {
    set.flipped = 0;
    pref.putUInt("flipped", set.flipped);
  } else {
    set.flipped = (uint8_t)pref.getUInt("flipped");
  }

  // TEXT COLOR
  if(not pref.isKey("textR")){
    set.textR = 0;
    pref.putUInt("textR", set.textR);
  } else {
    set.textR = (uint8_t)pref.getUInt("textR");
  }

  if(not pref.isKey("textG")){
    set.textG = 0;
    pref.putUInt("textG", set.textG);
  } else {
    set.textG = (uint8_t)pref.getUInt("textG");
  }

  if(not pref.isKey("textB")){
    set.textB = 250;
    pref.putUInt("textB", set.textB);
  } else {
    set.textB = (uint8_t)pref.getUInt("textB");
  }

  // BACKGROUND COLOR
  if(not pref.isKey("bgR")){
    set.bgR = 0;
    pref.putUInt("bgR", set.bgR);
  } else {
    set.bgR = (uint8_t)pref.getUInt("bgR");
  }

  if(not pref.isKey("bgG")){
    set.bgG = 0;
    pref.putUInt("bgG", set.bgG);
  } else {
    set.bgG = (uint8_t)pref.getUInt("bgG");
  }

  if(not pref.isKey("bgB")){
    set.bgB = 0;
    pref.putUInt("bgB", set.bgB);
  } else {
    set.bgB = (uint8_t)pref.getUInt("bgB");
  }

  pref.end(); // Close preferences
}