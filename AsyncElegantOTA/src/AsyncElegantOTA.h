#ifndef AsyncElegantOTA_h
#define AsyncElegantOTA_h

#include "Arduino.h"
#include "stdlib_noniso.h"

#if defined(ESP8266)
    #include "ESP8266WiFi.h"
    #include <ESPAsyncTCP.h>
#elif defined(ESP32)
    #include "WiFi.h"
    #include <AsyncTCP.h>
    #include <Update.h>
    #include <esp_int_wdt.h>
    #include <esp_task_wdt.h>
#endif

#include <ESPAsyncWebServer.h>

#include "elegantWebpage.h"

/*
   Login page
*/

const char* loginIndex_start =
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\"/>"
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('";

const char* loginIndex_end =
  "')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

String html = "";


size_t content_len;

class AsyncElegantOtaClass{
    public:

        void begin(AsyncWebServer *server){
            _server = server;
			
			String chipID_string = String((uint32_t)ESP.getEfuseMac());
			//Serial.println("/");
			//Serial.println(chipID_string);

			String html_start(loginIndex_start);
			String html_end(loginIndex_end);

			html = "";
			html.concat(html_start);
			html.concat("/");
			html.concat(chipID_string);
			html.concat(html_end);
			_server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request){
				char loginIndex[html.length() + 1];
				html.toCharArray(loginIndex, html.length() + 1);
				request->send(200, "text/html", loginIndex);
			});			
			
			String rutaUpdate = "/" + chipID_string;
			byte sizeChipID = rutaUpdate.length();
			char chipID_char[sizeChipID + 1];
			rutaUpdate.toCharArray(chipID_char, sizeChipID + 1);
            _server->on(chipID_char, HTTP_GET, [&](AsyncWebServerRequest *request){
				AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", ELEGANT_HTML, ELEGANT_HTML_SIZE);
				response->addHeader("Content-Encoding", "gzip");				
                request->send(response);
            });

            _server->on(chipID_char, HTTP_POST, [&](AsyncWebServerRequest *request) {
                // the request handler is triggered after the upload has finished... 
                // create the response, add header, and send response
                AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
                response->addHeader("Connection", "close");
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
                restartRequired = true;
            }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                //Upload handler chunks in data
                if (!index) {
                
                    content_len = request->contentLength();
                    #if defined(ESP8266)
                        int cmd = (filename.indexOf("spiffs") > -1) ? U_FS : U_FLASH;
                        Update.runAsync(true);
                        if (!Update.begin(content_len, cmd)){ // Start with max available size
                    #elif defined(ESP32)
                        int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
                        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
                    #endif
                            Update.printError(Serial);   
                        }
                
                }

                // Write chunked data to the free sketch space
                if (Update.write(data, len) != len) {
                    Update.printError(Serial); 
                }
                    
                if (final) { // if the final flag is set then this is the last frame of data
                    if (Update.end(true)) { //true to set the size to the current progress

                    }
                }
            });
        }

        void loop(){
            if(restartRequired){
                yield();
                delay(1000);
                yield();
                #if defined(ESP8266)
                    ESP.restart();
                #elif defined(ESP32)
                    esp_task_wdt_init(1,true);
                    esp_task_wdt_add(NULL);
                    while(true);
                #endif
            }
        }

    private:
        AsyncWebServer *_server;
        bool restartRequired = false;

};

AsyncElegantOtaClass AsyncElegantOTA;
#endif
