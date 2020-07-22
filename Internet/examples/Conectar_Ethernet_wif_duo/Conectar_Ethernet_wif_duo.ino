#include <Internet.h>
#include "AP_personalizado.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);

void ConfigNewServer() {
  Serial.println("Servidor iniciado.");

  const char* rutaUrl = "/prueba";
  server.on(rutaUrl, HTTP_GET, [](AsyncWebServerRequest * request) {
    String ipCliente = request->client()->remoteIP().toString().c_str();
    ProcessClient(ipCliente);

    const char* VALOR_VARIABLE_BUSCAR = "msg";
    if (request->hasParam(VALOR_VARIABLE_BUSCAR)) {
      String valor_variable = request->getParam(VALOR_VARIABLE_BUSCAR)->value();
    }

    String respuestaPeticion = "Hola mundo";
    request->send(200, "text/plain", respuestaPeticion);
  });

  server.begin();
}









TaskHandle_t nucleo0Handle = NULL;
void Nucleo0( void *pvParameters );

void setup() {
  Serial.begin(115200);
  bool usarWifi = true;
  bool usarEth = true;
  bool isAP = false;  
  ConfigInternet(usarWifi, usarEth, isAP);

  if(isActiveUseWifi() || isActiveUseEthernet()){    
    while (!( thereInternet() )) {
      //esperando a que se conecte a internet
      delay(100);
      Serial.print(".");
    }
  }
  
  ConfigNewServer();

  xTaskCreatePinnedToCore(
    Nucleo0
    ,  "Nucleo0"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &nucleo0Handle
    ,  0);
}

void loop() {

}


void Nucleo0(void *pvParameters) { // This is a task.
  (void) pvParameters;
  Serial.print("Nucleo: ");
  Serial.println(xPortGetCoreID());

  pinMode(2, OUTPUT);

  unsigned long startAttemptTime = millis();
  unsigned long timeGetResponse = millis();

  bool ejecutarAccion = false;
  bool esperarRespuesta = false;
  for (;;) { // A Task shall never return or exit.
    if (millis() - startAttemptTime > 5000) {
      startAttemptTime = millis();
      Serial.print("[Blink]: [Core 0]: "); Serial.println(xPortGetCoreID());
      timeGetResponse = millis();
      //ejecutarAccion = true;
    }



    if (ejecutarAccion) {
      SendGet("172.16.66.84", 1986, "/esp32?Datos=holaMundoEstoEsUnaPruebaDeWISROVI");
      esperarRespuesta = true;
    }

    if (esperarRespuesta) {
      if (millis() - timeGetResponse > 2000) {
        esperarRespuesta = false;
        Serial.print("RTA Internet: ");
        Serial.println(getResponse());
      }
    }

    digitalWrite(2, !digitalRead(2));   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(500 / portTICK_PERIOD_MS); // one tick delay (15ms) in between reads for stability
  }
  //vTaskDelete(NULL); //borrar esta tarea
}