/*********
  PWMWebServer.cpp
*********/

#include <Arduino.h>
// Load Wi-Fi library
#include <WiFi.h>

#include "PWMWebServer.h"

#include "PWMCommander.h"


const char* ssid     = "RTAX999";
const char* password = "LiDoDa#959285$";
//const char* ssid     = "HUAWEI P30";
//const char* password = "6381bf07b666";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";
String powerState = "off";
String strDirection = "forward";

bool directionState = true;

#define MAX_DUTY_CYCLE 99


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

int dutyCycle;
/* Setting PWM Properties */


void SetupPWMWebServer() {
  Serial.begin(115200);
  
  dutyCycle = 0;
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

   /*
     * The IR library setup. That's all!
     */
   
}

void EchoDutyCycle()
{
    Serial.print(F("PWM duty cycle "));
    Serial.println(dutyCycle);
}

void LoopPWMWebServer()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // 
            if (header.indexOf("GET /Speed+1/on") >= 0) {
              if (dutyCycle <= MAX_DUTY_CYCLE - 1)
                dutyCycle += 1;
              SetPWMCommand(dutyCycle);
              EchoDutyCycle ();
              
            } else if (header.indexOf("GET /Speed-1/on") >= 0) {
               if (dutyCycle >= 1)
                dutyCycle -= 1;
               SetPWMCommand(dutyCycle);
               EchoDutyCycle ();
                           
            } else if (header.indexOf("GET /Speed+5/on") >= 0) {
              if (dutyCycle <= MAX_DUTY_CYCLE - 5)
                dutyCycle += 5;
              SetPWMCommand(dutyCycle);
              EchoDutyCycle ();
              
            } else if (header.indexOf("GET /Speed-5/on") >= 0) {
               if (dutyCycle >= 5)
                dutyCycle -= 5;
               SetPWMCommand(dutyCycle);
               EchoDutyCycle ();
                           
            } else if (header.indexOf("GET /Speed+20/on") >= 0) {
              if (dutyCycle <= MAX_DUTY_CYCLE - 20)
                dutyCycle += 20;
              SetPWMCommand(dutyCycle);
              EchoDutyCycle ();
              
            } else if (header.indexOf("GET /Speed-20/on") >= 0) {
               if (dutyCycle >= 20)
                dutyCycle -= 20;
               SetPWMCommand(dutyCycle);
               EchoDutyCycle ();
                           
            } else if (header.indexOf("GET /Power/on") >= 0) {
               dutyCycle = 0;
               powerState="on";
               SetPWMCommand(dutyCycle);
               EchoDutyCycle ();
               Serial.println("Power on");
                           
            } else if (header.indexOf("GET /Power/off") >= 0) {
               dutyCycle = 0;
               powerState="off";
               SetPWMCommand(dutyCycle);
               EchoDutyCycle ();
               Serial.println("Power off");
            } else if (header.indexOf("GET /direction/v") >= 0) {
              dutyCycle = 0;
              powerState="off";
              directionState = !directionState;
               if (directionState) {
                  Serial.println("Direction forward");
                  strDirection = "forward";
                  SetPWMCommand(false, dutyCycle);

               } else {
                  Serial.println("Direction backward");
                  strDirection = "backward";

                  SetPWMCommand(true, dutyCycle);
               }
                           
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 10px 30px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".buttonGray {background-color: #555555;}");
            client.println(".buttonRed {background-color: #AF3535;}");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 PWM Server</h1>");              

            client.println("<p>Power State " + powerState + "</p>");
            if (powerState=="off") {
              client.println("<p><a href=\"/Power/on\"><button class=\"button\">ON</button></a>");
            } else {
              client.println("<p><a href=\"/direction/v\"><button class=\"button buttonRed\">OFF</button></a></p>");
            }

            char bufferDutyCycle[7];         //the ASCII of the integer will be stored in this char array
            itoa(dutyCycle,bufferDutyCycle,10);
            String strDutyCycle (bufferDutyCycle);
            client.println("<p>DutyCycle " + strDutyCycle + "</p>");
            client.println("<p>Direction " + strDirection + "</p>");

            client.println("<p><a href=\"/Speed+1/on\"><button class=\"button\">Speed+1</button></a>");
            client.println("<a href=\"/Speed-1/on\"><button class=\"button\">Speed-1</button></a></p>");
            
            client.println("<p><a href=\"/Speed+5/on\"><button class=\"button\">Speed+5</button></a>");
            client.println("<a href=\"/Speed-5/on\"><button class=\"button\">Speed-5</button></a></p>");
            
            client.println("<p><a href=\"/Speed+20/on\"><button class=\"button\">Speed+20</button></a>");
            client.println("<a href=\"/Speed-20/on\"><button class=\"button\">Speed-20</button></a></p>");

            client.println("<input type=\"range\" value=\"0\" max=\"100\" href=\"/slider-1/value\" name=\"slider-1\" id=\"slider-1\" oninput=\"numSliderValue.value = this.value\">");
            client.println("<output href=\"/numSliderValue/value\" id=\"numSliderValue\">0</output>");

            client.println("<form oninput=\"x.value=parseInt(a.value)+parseInt(b.value)\">");
            client.println("<input type=\"range\" id=\"a\" value=\"50\">");
            client.println("+<input type=\"number\" id=\"b\" value=\"25\">");
            client.println("=<output name=\"x\" for=\"a b\"></output>");
            
            client.println("<input type=\"submit\" value=\"Send Request\">");
            
            client.println("</form>");
            client.println("<form oninput=\"d.value=parseInt(c.value)\">");
            client.println("<a href=\"/direction/v\"><button class=\"button\">direction</button></a></p>");

           
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
