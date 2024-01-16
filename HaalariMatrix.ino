// Author: Remi / RxmTaco
// https://github.com/RxmTaco

// Dependencies
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

// Custom headers
#include "secrets.h"
#include "Pix_Ascii.h"

// LED options
#define LED_PIN         3
#define LED_ROWS        8
#define LED_COLS        32
#define LED_NUM         (LED_ROWS*LED_COLS)

// WIFI options
const uint8_t PORT =    80;
int keyIndex = 0;             // Needed for WEP

/*
// Settings structure
typedef struct {
  String  submittedText = "<SOURCE>    "; // The initial text to be shown
  uint    scrollDelay = 150;              // Time between column change while scrolling in milliseconds
  uint8_t textR = 0;                     // Text color RGB values
  uint8_t textG = 0;
  uint8_t textB = 255;
  uint8_t bgR = 0;                        // Background color RGB values
  uint8_t bgG = 0;
  uint8_t bgB = 0;
} settings;
*/

Preferences prefs;

//
// Declare SECRETS_SSID and SECRETS_PASSWD in secrets.h for WIFI SSID and password!!!
//

WiFiServer server(PORT);

Adafruit_NeoPixel pixels(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.clear();

  // Set all leds to 20,20,20
  for(int i = 0; i < LED_NUM; i ++){
    pixels.setPixelColor(i, pixels.Color(1, 1, 1));
    pixels.show();
    delay(1);
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(SECRETS_SSID, SECRETS_PASSWD);

  server.begin();
}

String  submittedText = "<SOURCE>    "; // The initial text to be shown
uint    scrollDelay = 150;              // Time between column change while scrolling in milliseconds
uint8_t textR = 0;                     // Text color RGB values
uint8_t textG = 0;
uint8_t textB = 255;

uint8_t bgR = 0;                        // Background color RGB values
uint8_t bgG = 0;
uint8_t bgB = 0;

void loop() {
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
        client.printf("Text: <input type='text' name='inputText' placeholder='%s'>", submittedText); client.println("<br>");
        client.println("<br>");
        client.printf("Scroll Delay: <input type='text' name='scrollDelay' placeholder='%i'>", scrollDelay); client.println("<br>");
        client.println("<br>");
        client.println("Text properties:");
        client.println("<br>");
        client.printf("Text R: <input type='text' name='colorR' placeholder='%i'>", textR); client.println("<br>");
        client.printf("Text G: <input type='text' name='colorG' placeholder='%i'>", textG); client.println("<br>");
        client.printf("Text B: <input type='text' name='colorB' placeholder='%i'>", textB); client.println("<br>");
        client.println("<br>");
        client.println("Background:"); 
        client.println("<br>");
        client.printf("Background R: <input type='text' name='bgR' placeholder='%i'>", bgR); client.println("<br>");
        client.printf("Background G: <input type='text' name='bgG' placeholder='%i'>", bgG); client.println("<br>");
        client.printf("Background B: <input type='text' name='bgB' placeholder='%i'>", bgB); client.println("<br>");
        client.println("<input type='submit' value='Submit'>");
        client.println("</form>");
        client.println("</div>");
        //client.println("<br>");
        //client.println(request);  // Display request for debugging
        //client.println("<br>");
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
  displayText(submittedText);
}

uint8_t updateIndex = 0;
unsigned long currentMillis = 0;

void displayText(String text) {
  if(millis() - currentMillis > scrollDelay){
    pixels.clear();

    int charNum = text.length();
    uint8_t cols = 6; // 5 for character and 1 for a blank space between characters

    // create display matrix buffer
    int columnLimit = charNum + charNum * 5;
    int dispMatrix[ROWS][columnLimit]; // make a 2D array of the enabled pixels

    // fill matrix buffer
    for(int k = 0; k < text.length(); k++){ // iterate over characters
      // get the matrix representing the current character
      const uint8_t (&currentChar)[ROWS][5] = getPixelMatrix(text.charAt(k));
      //int colIndex = k * cols; // shift index for each character

      for(int i = 0; i < cols; i++){
        for(int j = 0; j < ROWS; j++){
          if(i == cols - 1)
            dispMatrix[j][k * cols + i] = 0;
          else
          dispMatrix[j][k * cols + i] = currentChar[j][i];
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
            (enabled)? textR : bgR, 
            (enabled)? textG : bgG,
            (enabled)? textB : bgB));
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
            (enabled)? textR : bgR, 
            (enabled)? textG : bgG,
            (enabled)? textB : bgB));
        }
      }
    }

    pixels.show();

    if(updateIndex == columnLimit)
      updateIndex = 0;
    else
      updateIndex++;

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
      submittedText = urldecode(inputValue);
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
      scrollDelay = inputValue.toInt();
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
      textR = inputValue.toInt();
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
      textG = inputValue.toInt();
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
      textB = inputValue.toInt();
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
      bgR = inputValue.toInt();
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
      bgG = inputValue.toInt();
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
      bgB = inputValue.toInt();
    }
  }
}