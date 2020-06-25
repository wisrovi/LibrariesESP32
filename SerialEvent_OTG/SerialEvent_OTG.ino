String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

unsigned long startAttemptTime;

void setup() {
  Serial.begin(115200);
  inputString.reserve(200);
  Serial.println("Sistema iniciado");
  pinMode(2, OUTPUT);

  startAttemptTime = millis();
}


void loop() {
  if (stringComplete) {
    if (inputString.equals("DISP")) {
      String dataSend = "{\"code\":\"DISP\",\"listModulos\":[{\"key\":\"tempAmb\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":24,\"setPoint\":-1,\"unidadMedida\":\"C\",\"id\":10}},{\"key\":\"tempCabin\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":24,\"setPoint\":-1,\"unidadMedida\":\"C\",\"id\":11}},{\"key\":\"GIROS\",\"value\":{\"X\":32,\"Y\":25,\"Z\":10,\"unidadMedida\":\"g\",\"id\":38}},{\"key\":\"PRESION\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":14.2,\"setPoint\":-1,\"unidadMedida\":\"PSI\",\"id\":19}},{\"key\":\"GASOLINA\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":75,\"setPoint\":-1,\"unidadMedida\":\"%\",\"id\":30}},{\"key\":\"TORQUE\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":3.8,\"setPoint\":-1,\"unidadMedida\":\"u\",\"id\":34}},{\"key\":\"POTEN\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":22,\"setPoint\":-1,\"unidadMedida\":\"Hp\",\"id\":35}},{\"key\":\"RPM\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":5000,\"setPoint\":-1,\"unidadMedida\":\"rpm\",\"id\":36}},{\"key\":\"SHIFT\",\"value\":{\"shift\":true,\"id\":49}},{\"key\":\"BRUJULA\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":30,\"setPoint\":-1,\"unidadMedida\":\"g\",\"id\":37}},{\"key\":\"MSNM\",\"value\":{\"msnm\":1775,\"unidadMedida\":\"m\",\"id\":39}},{\"key\":\"VEL\",\"value\":{\"valorMax\":-1,\"valorMin\":-1,\"valorActual\":127,\"setPoint\":-1,\"unidadMedida\":\"Kmh\",\"id\":50}}]}";
      Serial.println(dataSend);
    } else {      
      Serial.print("[esp32]");
      Serial.print(inputString);
      Serial.println("[/esp32]");
    }
    inputString = "";
    stringComplete = false;
    digitalWrite(2, !digitalRead(2));
  }
  if (millis() - startAttemptTime > 2000)
  {
    startAttemptTime = millis();
    Serial.println("Proceso automatico");
  }

  serialEvent(); //manually calling "void serialEvent()"
}

void serialEvent() {
  if (Serial.available()) {
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar == '\n') {
        stringComplete = true;
      } else {
        inputString += inChar;
      }
    }
  }
}
