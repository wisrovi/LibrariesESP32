#include <Internet.h>

void prueba(AsyncWebServerRequest *request) {
  //String ipCliente = request->client()->remoteIP().toString().c_str();
  const char* VALOR_VARIABLE_BUSCAR = "msg";
  if (request->hasParam(VALOR_VARIABLE_BUSCAR)) {
    String valor_variable = request->getParam(VALOR_VARIABLE_BUSCAR)->value();
  }
  request->send(200, "text/plain", "Hello, world");
}





TaskHandle_t nucleo0Handle = NULL;
void Nucleo0( void *pvParameters );

void setup() {
  Serial.begin(115200);
  bool usarWifi = true;
  bool usarEth = false;
  bool isAP = false;

  //deshabilitarWifimanager(); //se usa esta linea para evitar que el sistema en caso de que las credenciales de red no sean correctas, el mismo sistema active el wifi manager
  if (ConfigRedWifiConection("CTEx", "C2Pr0du.") == false) {
    //se utiliza antes de configurar el internet para que la wifi se conecte a estas credenciales
    //esta instrucción no funciona si el modo AP esta activo
    Serial.println("No se puede configurar una conexion a una red wifi mientras este activo un modo AP.");
  }
  ConfigInternet(usarWifi, usarEth, isAP);

  if (isActiveUseWifi()) {
    while (!getStatusConectionwifi()) {
      //esperando a que se conecte a una red wifi
      delay(100);
      Serial.print(".");
    }
  }

  if (getStatusConectionwifi() && isActiveUseWifi()) {
    RegistrarNuevoServicioGet("/prueba", prueba);
    setIniciarServidorWifi();
  }


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
  for (;;) { // A Task shall never return or exit.
    if (millis() - startAttemptTime > 5000) {
      startAttemptTime = millis();
      Serial.print("[Blink]: [Core 0]: "); Serial.println(xPortGetCoreID());
    }

    digitalWrite(2, !digitalRead(2));   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(500 / portTICK_PERIOD_MS); // one tick delay (15ms) in between reads for stability
  }
  //vTaskDelete(NULL); //borrar esta tarea
}