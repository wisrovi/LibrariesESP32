#include "Beacon.h"

TaskHandle_t nucleo0_servidor = NULL;
void Nucleo0_IniciarServidorBLE(void *pvParameters);
bool startWithBLE = true;

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    Nucleo0_IniciarServidorBLE,
    "Nucleo0_IniciarServidorBLE" // A name just for humans
    ,
    5000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    &nucleo0_servidor, 0);
}

void loop() {
  delay(1000);
}

void Nucleo0_IniciarServidorBLE(void *pvParameters)
{ // This is a task.
  (void)pvParameters;
  Serial.print("Nucleo: ");
  Serial.println(xPortGetCoreID());
  configBLEscan();
  bool detenerEscaneoAlEncontrarBeaconCorrecto = true;

  unsigned long startAttemptTime = millis();
  for (;;)
  { // A Task shall never return or exit.
    if (millis() - startAttemptTime > 1000)
    {
      startAttemptTime = millis();
      Serial.println("Comprobando BLE");
      
      scanBeacon();
      if (BeaconEncontrado) {
        BeaconEncontrado = false;
        Serial.println("BLE encontrado");
        //Proceso del BLE que se queda esperando 
        //hasta tener la proximidad de un movil con la app instalada, 
        //cuando encuentra un BLE correcto: 
        //-se sale de este modo
        //-activa el wifi con todos sus perifericos

        if(detenerEscaneoAlEncontrarBeaconCorrecto){
          startWithBLE = false;
        }
      }
    }

    if (startWithBLE == false)
    {
      vTaskDelete(NULL); //borrar esta tarea
      Serial.println("Deteniendo busqueda de BLE");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // one tick delay (15ms) in between reads for stability
  }
}
