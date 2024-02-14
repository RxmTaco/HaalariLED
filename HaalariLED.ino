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
#define RST_PIN_AUX     5
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
  String  submittedText;    // The initial text to be shown
  uint    scrollDelay;      // Time between column change while scrolling in milliseconds
  uint8_t flipped;          // Flipped text
  uint8_t vertical;         // Vertical text
  uint8_t textR;            // Text color RGB values
  uint8_t textG;
  uint8_t textB;
  uint8_t bgR;              // Background color RGB values
  uint8_t bgG;
  uint8_t bgB;
  uint8_t gap;
  String  effect;           // Using effects or not
  uint8_t effectBrightness; // Effect parameters
  uint8_t effectSpeed;      
  String  ssid;             // Default SSID:      ESP-32-HaalariLED
  String  passwd;           // Default Password:  12345678
} settings_t;

settings_t settings;

Preferences prefs;

WiFiServer server(PORT);

Adafruit_NeoPixel pixels(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  delay(2000);
  // Convert hostname to lower case
  String htemp = (String)hostname;
  htemp.toLowerCase();
  hostname = htemp.c_str();

  pinMode(RST_PIN, INPUT_PULLUP);
  pinMode(RST_PIN_AUX, INPUT_PULLUP);
  pinMode(RST_SRC, OUTPUT);
  digitalWrite(RST_SRC, LOW);      // Write high at setup to keep rst source positive

  pixels.begin();
  pixels.clear();

  initPrefs(settings); // Load / initialize variables from flash

  // Test LEDs with HSL values
  for(int i = 0; i < LED_NUM; i ++){
    int r, g, b;
    hsl_to_rgb(((float)i/(float)LED_NUM) * 360.0f, 1, 0.2, &r, &g, &b);
    pixels.setPixelColor(i, pixels.Color(r, g, b));
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
  if(digitalRead(RST_PIN) == LOW || digitalRead(RST_PIN_AUX) == LOW){
    prefs.begin("configs", false);
    prefs.clear();
    prefs.end();
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
        // Dropdown menu
        client.println("Effect: <select name='effect'>");
        client.printf("<option value='text' %s>Text</option>", (settings.effect=="text")?"selected":"");
        client.printf("<option value='flush' %s>Color Flush</option>", (settings.effect=="flush")?"selected":"");
        client.printf("<option value='vortex' %s>Color Vortex</option>", (settings.effect=="vortex")?"selected":"");
        client.println("</select><br><br>");

        // text
        if(settings.effect == "text"){
          client.printf("Text: <input type='text' name='inputText' placeholder='%s'>", settings.submittedText); client.println("<br>");
          client.println("<br>");
          client.printf("Scroll Delay: <input type='text' name='scrollDelay' placeholder='%i'>", settings.scrollDelay); client.println("<br>");
          client.printf("Letter gap: <input type='text' name='gap' placeholder='%i'>", settings.gap); client.println("<br>");
          client.printf("Flipped: <input type='checkbox' name='flip' %s>", settings.flipped?"checked":""); //client.println("<br>");
          client.printf("Vertical: <input type='checkbox' name='vert' %s>", settings.flipped?"checked":""); client.println("<br>");
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
        } else if(settings.effect=="flush" || settings.effect=="vortex"){
          client.println("Effect Brightness %: ");
          client.printf("<input type='text' name='effB' placeholder='%i'>", settings.effectBrightness); client.println("<br>");
          client.println("Effect Speed %: ");
          client.printf("<input type='text' name='effS' placeholder='%i'>", settings.effectSpeed); client.println("<br>");
        }
        
        //Wifi
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

        // CSS styling
        client.println("<style>");
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
  savePrefs(settings);


  // Switch for the current effect
  if(settings.effect == "text")
      displayText(settings.submittedText);
  else if (settings.effect == "flush")
      displayColorFlush();
  else if (settings.effect == "vortex")
      displayColorVortex();
  else
      displayText(settings.submittedText);

}


uint16_t updateIndex = 0;
unsigned long currentMillis = 0;



void flashLeds(int r, int g, int b){
  delay(100);
  pixels.clear();
  for(int i = 0; i < LED_NUM; i ++){
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
  delay(100);
}

void displayColorFlush(){
  int speed = map(settings.effectSpeed, 0, 100, 100, 0);

  if (millis() - currentMillis < speed)
    return;
  currentMillis = millis();

  float l = (float)settings.effectBrightness / 100.0f;

  if(updateIndex >= LED_NUM)
    updateIndex = 0;
  for(int i = 0; i < LED_NUM; i ++){
    int r, g, b;
    hsl_to_rgb(((float)i/(float)LED_NUM) * 360.0f, 1, l, &r, &g, &b);

    int index = updateIndex + i;
    if(index >= LED_NUM)
      index -= LED_NUM;

    pixels.setPixelColor(index, pixels.Color(r, g, b));
  }

  pixels.show();

  updateIndex++;
}

void displayColorVortex(){
  pixels.clear();
  int speed = map(settings.effectSpeed, 0, 100, 100, 1);
  if (millis() - currentMillis < 1)
    return;
  currentMillis = millis();

  float displayMatrix[LED_ROWS][LED_COLS] = {0};
  float brightness = (float)settings.effectBrightness / 100.0f;


  int centerX = LED_COLS / 2;
  int centerY = LED_ROWS / 2;

  float angle = currentMillis / (float)speed; // Adjust speed of rotation if needed
  
  for(int i = 0; i < LED_COLS; i++){ // horizontal
    for(int j = 0; j < LED_ROWS; j++){ // vertical
      float dx = i - centerX;
      float dy = j - centerY;
      float distance = sqrt(dx * dx + dy * dy);
      float angle_to_center = atan2(dy, dx) * 180.0 / PI;
      float sector = fmod(angle_to_center + angle, 360.0);
      float hue = fmod(sector * 3.0, 360.0); // Divide by 3 to spread colors evenly

      // Clamp hue value within range [0, 360]
      if (hue < 0)
        hue += 360;
      else if (hue >= 360)
        hue -= 360;

      displayMatrix[j][i] = hue;
    }
  }

  // DISPLAY
  for(int i = 0; i < LED_COLS; i++){ // horizontal
    for(int j = 0; j < LED_ROWS; j++){ // vertical
      int row;
      if(i % 2 == 0)
        row = j;
      else
        row = LED_ROWS - 1 - j;
      float hue = displayMatrix[row][i];
      int r, g, b;
      hsl_to_rgb(hue, 1.0f, brightness, &r, &g, &b);
      pixels.setPixelColor(j + i * LED_ROWS, pixels.Color(r, g, b));
    }
  }
  
  pixels.show();
  updateIndex++;
}

void displayText(String text) {
  if(millis() - currentMillis > settings.scrollDelay){
    pixels.clear();

    int charNum = text.length();
    uint8_t cols = 5 + settings.gap + (settings.vertical?3:0); // 5 for character and 1 for a blank space between characters

    // create display matrix buffer
    int columnLimit = (charNum * cols);
    int dispMatrix[ROWS][columnLimit]; // make a 2D array of the enabled pixels

    // fill matrix buffer
    for(int k = 0; k < text.length(); k++){ // iterate over characters
      // get the matrix representing the current character
      uint8_t** currentChar = getPixelMatrix(text.charAt(k));

      uint8_t charBuffer[ROWS][cols];
      for(int i = 0; i < cols; i++){
        for(int j = 0; j < ROWS; j++){
          if(settings.vertical){
            if(i < 5)
              charBuffer[i][j] = currentChar[j][i];
            else
             charBuffer[i][j] = 0;
          } else {
            if(i < 5)
              charBuffer[j][i] = currentChar[j][i];
            else
             charBuffer[j][i] = 0;
          }
        }
      }

      for(int i = 0; i < cols; i++){
        for(int j = 0; j < ROWS; j++){
          if(i >= cols - settings.gap)
            dispMatrix[j][k * cols + i] = 0;
          else
            dispMatrix[j][k * cols + i] = charBuffer[j][i];
        }
      }
      for(int i = 0; i < ROWS; i++){
        delete[] currentChar[i]; // delete dynamic array
      }
      delete[] currentChar; // delete dynamic array
    }

    // Flip the matrix if settings.flipped is not 0
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
      if(updateIndex <= 0)
        updateIndex = columnLimit;
      else
        updateIndex--;
    } else {
      if(updateIndex >= columnLimit)
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

void hsl_to_rgb(float h, float s, float l, int *r, int *g, int *b) {
  float c, x, m;
  float tmp_r, tmp_g, tmp_b;

  if (s == 0) {
    *r = *g = *b = (int)(l * 255);
    return;
  }

  c = (1 - fabs(2 * l - 1)) * s;
  x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
  m = l - c / 2.0;

  if (h < 60) {
    tmp_r = c;
    tmp_g = x;
    tmp_b = 0;
  } else if (h < 120) {
    tmp_r = x;
    tmp_g = c;
    tmp_b = 0;
  } else if (h < 180) {
    tmp_r = 0;
    tmp_g = c;
    tmp_b = x;
  } else if (h < 240) {
    tmp_r = 0;
    tmp_g = x;
    tmp_b = c;
  } else if (h < 300) {
    tmp_r = x;
    tmp_g = 0;
    tmp_b = c;
  } else {
    tmp_r = c;
    tmp_g = 0;
    tmp_b = x;
  }

  *r = (int)((tmp_r + m) * 255);
  *g = (int)((tmp_g + m) * 255);
  *b = (int)((tmp_b + m) * 255);
}

// Extract the passed parameters from the HTTP POST request
void extractParameters(String request) {
  int post = request.indexOf("POST");
  if(post == -1)
    return;

  // Extract the value for effect
  int textPosition = request.indexOf("effect="); // Search for the index of the parameter
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition); // Parameters are separated by an ampersand, so read until one appears
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 7, nextParameterPosition);  // Apply the value to a global parameter
    if (!inputValue.isEmpty()) {                                                      // If the parameter is null, continue to next
      settings.effect = inputValue;
    }
  }

  // Extract the values for effect parameters
  textPosition = request.indexOf("effB=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 5, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      int val = inputValue.toInt();
      if(val > 100)
        val = 100;
      if(val < 0)
        val = 0;
      settings.effectBrightness = val;
    }
  }
  textPosition = request.indexOf("effS=");
  if (textPosition != -1){
    int nextParameterPosition = request.indexOf("&", textPosition);
    if (nextParameterPosition == -1) {
      nextParameterPosition = request.length();
    }
    String inputValue = request.substring(textPosition + 5, nextParameterPosition);
    if (!inputValue.isEmpty()) {
      int val = inputValue.toInt();
      if(val > 100)
        val = 100;
      if(val < 0)
        val = 0;
      settings.effectSpeed = val;
    }
  }

  // Extract the value for "inputText"
  textPosition = request.indexOf("inputText="); // Search for the index of the parameter
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
      int val = inputValue.toInt();
      if(val < 0)
        val = 0;
      settings.gap = val;
    }
  }

  // extract value for text flip enable
  textPosition = request.indexOf("flip=");
  if (textPosition != -1){
    settings.flipped = 1;
  } else {
    settings.flipped = 0;
  }

  textPosition = request.indexOf("vert=");
  if (textPosition != -1){
    settings.vertical = 1;
  } else {
    settings.vertical = 0;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.textR = val;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.textG = val;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.textB = val;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.bgR = val;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.bgG = val;
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
      int val = inputValue.toInt();
      if(val > 255)
        val = 255;
      if(val < 0)
        val = 0;
      settings.bgB = val;
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
          settings.passwd = inputValue; // Change password on success
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

void savePrefs(settings_t& set){
  prefs.begin("configs", false); // Open preferences: settings namespace, false for RW mode

  prefs.putString("effect", set.effect);

  prefs.putUInt("effB", set.effectBrightness);
  prefs.putUInt("effS", set.effectSpeed);

  prefs.putString("submittedText", set.submittedText);
  prefs.putUInt("scrollDelay", set.scrollDelay);
  prefs.putUInt("gap", set.gap);
  prefs.putUInt("flipped", set.flipped);
  prefs.putUInt("vertical", set.vertical);

  prefs.putUInt("textR", set.textR);
  prefs.putUInt("textG", set.textG);
  prefs.putUInt("textB", set.textB);

  prefs.putUInt("bgR", set.bgR);
  prefs.putUInt("bgG", set.bgG);
  prefs.putUInt("bgB", set.bgB);

  prefs.putString("ssid", set.ssid);
  prefs.putString("passwd", set.passwd);

  prefs.end();
}

void initPrefs(settings_t& set){
  prefs.begin("configs", false); // Open preferences: settings namespace, false for RW mode

  // WIFI
  if(not prefs.isKey("ssid")) {
    set.ssid = "ESP-32-HaalariLED";
    prefs.putString("ssid", set.ssid);
  } else {
    set.ssid = prefs.getString("ssid");
  }

  if(not prefs.isKey("passwd")) {
    set.passwd = "12345678";
    prefs.putString("passwd", set.passwd);
  } else {
    set.passwd = prefs.getString("passwd");
  }

  // Effect
  if(not prefs.isKey("effect")) {
    set.effect = "text";
    prefs.putString("effect", set.effect);
  } else {
    set.effect = prefs.getString("effect");
  }

  if(not prefs.isKey("effB")) {
    set.effectBrightness = 5;
    prefs.putUInt("effB", set.effectBrightness);
  } else {
    set.effectBrightness = prefs.getUInt("effB");
  }

  if(not prefs.isKey("effS")) {
    set.effectSpeed = 70;
    prefs.putUInt("effS", set.effectSpeed);
  } else {
    set.effectSpeed = prefs.getUInt("effS");
  }

  // TEXT
  if(not prefs.isKey("submittedText")) {
    set.submittedText = "<SOURCE>   ";
    prefs.putString("submittedText", set.submittedText);
  } else {
    set.submittedText = prefs.getString("submittedText");
  }

  if(not prefs.isKey("scrollDelay")) {
    set.scrollDelay = 150;
    prefs.putUInt("scrollDelay", set.scrollDelay);
  } else {
    set.scrollDelay = prefs.getUInt("scrollDelay");
  }

  if(not prefs.isKey("gap")) {
    set.gap = 1;
    prefs.putUInt("gap", set.gap);
  } else {
    set.gap = (uint8_t)prefs.getUInt("gap");
  }

  // Text orientation
  if(not prefs.isKey("flipped")) {
    set.flipped = 0;
    prefs.putUInt("flipped", set.flipped);
  } else {
    set.flipped = (uint8_t)prefs.getUInt("flipped");
  }
  if(not prefs.isKey("vertical")) {
    set.vertical = 0;
    prefs.putUInt("vertical", set.vertical);
  } else {
    set.vertical = (uint8_t)prefs.getUInt("vertical");
  }

  // TEXT COLOR
  if(not prefs.isKey("textR")){
    set.textR = 0;
    prefs.putUInt("textR", set.textR);
  } else {
    set.textR = (uint8_t)prefs.getUInt("textR");
  }

  if(not prefs.isKey("textG")){
    set.textG = 0;
    prefs.putUInt("textG", set.textG);
  } else {
    set.textG = (uint8_t)prefs.getUInt("textG");
  }

  if(not prefs.isKey("textB")){
    set.textB = 250;
    prefs.putUInt("textB", set.textB);
  } else {
    set.textB = (uint8_t)prefs.getUInt("textB");
  }

  // BACKGROUND COLOR
  if(not prefs.isKey("bgR")){
    set.bgR = 0;
    prefs.putUInt("bgR", set.bgR);
  } else {
    set.bgR = (uint8_t)prefs.getUInt("bgR");
  }

  if(not prefs.isKey("bgG")){
    set.bgG = 0;
    prefs.putUInt("bgG", set.bgG);
  } else {
    set.bgG = (uint8_t)prefs.getUInt("bgG");
  }

  if(not prefs.isKey("bgB")){
    set.bgB = 0;
    prefs.putUInt("bgB", set.bgB);
  } else {
    set.bgB = (uint8_t)prefs.getUInt("bgB");
  }

  prefs.end(); // Close preferences
}