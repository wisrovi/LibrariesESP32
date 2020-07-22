#include <Internet.h>

void prueba(AsyncWebServerRequest *request){
  //String ipCliente = request->client()->remoteIP().toString().c_str();
  ProcesarClienteRequest(request);    //only for AP
  const char* VALOR_VARIABLE_BUSCAR = "msg";
  if (request->hasParam(VALOR_VARIABLE_BUSCAR)) {
    String valor_variable = request->getParam(VALOR_VARIABLE_BUSCAR)->value();
  }    
  request->send(200, "text/plain", "Hello, world");
}

void prueba2(AsyncWebServerRequest *request){
  request->send(200, "text/plain", "Hola mundo");
}

TaskHandle_t nucleo0Handle = NULL;
void Nucleo0( void *pvParameters );

void setup() {
  Serial.begin(115200);
  bool usarWifi = false;
  bool usarEth = false;
  bool isAP = true;  
  ConfigInternet(usarWifi, usarEth, isAP);
  
  if(isActiveUseAP()){
    if(ConfigWifiAP("queso", "1234567890")==false){
      Serial.println("Por favor active el modo AP desde la opción de 'ConfigInternet(*,*,true);' para poder usar el modo AP.");
    }
    while (!( getAPStart()   )) {
      //esperando a que inicie el AP o se conecte a una red internet
      delay(100);
      Serial.print(".");
    }
  }  
  
  RegistrarNuevoServicioGet("/prueba", prueba);
  RegistrarNuevoServicioGet("/hola", prueba2);
  setIniciarServidorAP();

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

  for (;;) { // A Task shall never return or exit.
    if (millis() - startAttemptTime > 5000) {
      startAttemptTime = millis();
      Serial.print("[Blink]: [Core 0]: "); Serial.println(xPortGetCoreID());
      timeGetResponse = millis();
    }

    digitalWrite(2, !digitalRead(2));   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(500 / portTICK_PERIOD_MS); // one tick delay (15ms) in between reads for stability
  }
  //vTaskDelete(NULL); //borrar esta tarea
}