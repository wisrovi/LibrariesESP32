#define useWifi
#define useEth
#define useAP


#ifdef useWifi
bool thereConectionWifi = false;
#define EspacioMemoriaWifi  15000	//8500
#endif


#ifdef useEth
bool thereConectionEthernet = false;
#define EspacioMemoriaEthernet  1500
#endif




#ifdef useAP
bool thereConectionAP = false;
char passwordAP_char[25];
char nombreAP_char[25];
#define EspacioMemoriaWifiAP  15000 //3100
#define useCaptivePortal
#endif





/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                 WIFI TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

#ifdef useWifi
#define DEVICE_NAME "SENINT"
#define WIFI_TIMEOUT 20000 // 20 seconds



#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>

#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server_OTA(80);

#include <ArduinoJson.h>
bool IniciarServidorWifi = false;
bool servidorWifi_iniciado = false;
bool usarWifiManager = true;
DynamicJsonDocument rutasUrl(1024);
typedef std::function<void(AsyncWebServerRequest *request)> THandlerFunction;
bool RegistrarNuevoServicioGet(String url, THandlerFunction fn){
	bool servicioCreado = false;
	
	bool estaActivoUsoWii = false;
	#ifdef useAP
		estaActivoUsoWii = thereConectionAP;
	#endif
		
	if(estaActivoUsoWii || thereConectionWifi){
		char url_char[url.length()+1];
		url.toCharArray(url_char, url.length()+1);
		bool existeRuta = rutasUrl[url];;
		if(existeRuta==false){
			server_OTA.on(url_char, HTTP_GET, fn);
			rutasUrl[url] = true;
			servicioCreado = true;
		}else{
			Serial.print("[WIFI]: El servicio '");	
			Serial.print(url);	
			Serial.println("' no se puede crear, ya existe o esta reservado.");	
		}		
		/*String allStationsConected;
		serializeJson(rutasUrl, allStationsConected);
		Serial.print("[AP]: ");
		Serial.println(allStationsConected);*/
	}
	return servicioCreado;
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}



TaskHandle_t nucleo1WifiKeepAlive_Handle = NULL;
void keepWiFiAlive( void *pvParameters );

bool useCredentialsConfigured = false;
bool actived_use_wifi = false;
#define tiempoIntentoConexionMaxima	20 //segundos

bool setIniciarServidorWifi(){
	if(thereConectionWifi){
		IniciarServidorWifi = true;
	}
	return IniciarServidorWifi;
}

#ifndef useAP
	char nombreAP_char[25];
	char passwordAP_char[25];
	bool thereConectionAP = false;
#endif


bool ConfigRedWifiConection(String nombreAP, String passwordAP){
	bool is_AP = false;
	#ifdef useAP
		is_AP = thereConectionAP;
	#endif
	
	if(is_AP == false){
		nombreAP.toCharArray(nombreAP_char, 25);
		passwordAP.toCharArray(passwordAP_char, 25);
		useCredentialsConfigured = true;
	}	
	return useCredentialsConfigured;
}

void ConfigWifiKeepAlive() {
  //Esta tarea se ejecuta en el nucleo 1, y se encargar de mantener la conexión wifi activa,
  //si pierde conexión, la reestablece nuevamente con las credenciales guardadas
  //siempre que haya conexion, esta libreria monta su actualización por OTA
  //cuando se pierde la conexión, la libreria reinicia los servicios asociados a la libreria para evitar que un servicio no vaya a funcionar

  xTaskCreatePinnedToCore(
    keepWiFiAlive,      // Function that should be called
    "keepWiFiAlive",    // Name of the task (for debugging)
    EspacioMemoriaWifi,         // Stack size (bytes),                             EL USADO Y FUNCIONAL: 8000
    NULL,         // Parameter to pass
    1 ,           // Task priority
    &nucleo1WifiKeepAlive_Handle,         // Task handle
    1             // Core you want to run the task on (0 or 1)
  );
}


void deshabilitarWifimanager(){
	usarWifiManager = false;
}

void keepWiFiAlive(void * parameter) {
  Serial.print("[WIFI]: [Core 1]: "); Serial.println(xPortGetCoreID());
  
  rutasUrl["/"] = true;
  rutasUrl["/update"] = true;

  int retardoConexionEstablecida = 0;
  bool debugWifiManager = true;
  bool haveIpOutFor = false;
  thereConectionWifi = false;
  
  WiFiManager wifiManager;  
  
  int conteoIntentos = 0;
  bool heObtenidoIpConCredenciales = false;
  while(useCredentialsConfigured==true && heObtenidoIpConCredenciales==false){
	  Serial.println("\n[WIFI]: Usando credenciales para buscar ip");
	  WiFi.begin(nombreAP_char, passwordAP_char);
	  byte contadorTiempoConexion = 0;	  
	  while (WiFi.status() != WL_CONNECTED) {
		  vTaskDelay(500 / portTICK_PERIOD_MS);
		  //Serial.print(".");
		  contadorTiempoConexion++;
		  if(contadorTiempoConexion>=(tiempoIntentoConexionMaxima*2)){
			  contadorTiempoConexion = 0;
			  break;
		  }
	  }	  
	  
	  if (WiFi.status() == WL_CONNECTED) {
		  heObtenidoIpConCredenciales = true;		  
	  }else{
		  conteoIntentos++;
		  if(conteoIntentos>=3){
			  useCredentialsConfigured = false;
		  }
	  }	  
  }

  if(useCredentialsConfigured==false && usarWifiManager==true){	
	  Serial.println("[WIFI]: Las credenciales dadas no son validas, se ha activado un AP para que desde el movil pueda configurar la Wifi, favor ingresar a: 192.168.4.1 en un navegador.");
	  //wifiManager.setTimeout(300);
	  wifiManager.setDebugOutput(debugWifiManager);
	  wifiManager.autoConnect(DEVICE_NAME);
	  Serial.println("\n[WIFI]: Las nuevas credenciales han sido almacenadas.");
  }
  

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("[WIFI]: Iniciando AP para configuración...");
  }

  //Serial.println(wifiManager.getSSID());
  //Serial.println(wifiManager.getPassword());
  
  
  if (WiFi.status() == WL_CONNECTED) {
    haveIpOutFor = true;
	thereConectionWifi = true;	
  }else{
	if(usarWifiManager==false){
	  Serial.println("\n[WIFI]: Cerrando la tarea pues con las credenciales dadas no es posible conectarse a la red y se ha desactivado el WifiManager.");
	  vTaskDelete(NULL); //borrar esta tarea
	}
  }
  String ssid = WiFi.SSID();
  String PSK = WiFi.psk();  

  int conteoIntentosConexion = 0;
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && haveIpOutFor == false) {
      bool thereConection = true;
      while (thereConection && servidorWifi_iniciado) {
        retardoConexionEstablecida++;
        if (retardoConexionEstablecida > 1000) {
          retardoConexionEstablecida = 0;
          thereConection = false;
        }
        vTaskDelay(15 / portTICK_PERIOD_MS);
        AsyncElegantOTA.loop();
      }      
      conteoIntentosConexion = 0;
      thereConectionWifi = true;
	  if(servidorWifi_iniciado==true){
		  Serial.print("[WIFI]: [Core 1]: "); Serial.println(xPortGetCoreID());
		  continue;
	  }      
    } else {
      thereConectionWifi = false;
	  servidorWifi_iniciado = false;
    }

	if(thereConectionWifi==false){
		Serial.println("[WIFI] Connecting...");
		if (conteoIntentosConexion <= 2) {
		  char ssid_char[ssid.length() + 1];
		  ssid.toCharArray(ssid_char, ssid.length() + 1);

		  char psk_char[PSK.length() + 1];
		  PSK.toCharArray(psk_char, PSK.length() + 1);

		  WiFi.mode(WIFI_STA);
		  WiFi.setHostname(DEVICE_NAME);
		  WiFi.begin(ssid_char, psk_char);
		} else {
			while(useCredentialsConfigured && WiFi.status() != WL_CONNECTED){
				WiFi.begin(nombreAP_char, passwordAP_char);
				byte contadorTiempoConexion = 0;
				Serial.println("Usando credenciales para buscar ip 2");
				while (WiFi.status() != WL_CONNECTED) {
					vTaskDelay(500 / portTICK_PERIOD_MS);
					//Serial.print(".");
					contadorTiempoConexion++;
					if(contadorTiempoConexion>=(tiempoIntentoConexionMaxima*2)){
						contadorTiempoConexion = 0;
						break;
					}
				}	  

				if (WiFi.status() == WL_CONNECTED) {
				  break;
				}else{
					conteoIntentos++;
					if(conteoIntentos>=3){
						useCredentialsConfigured = false;
					}
				}	  
			}
			
			if(useCredentialsConfigured==false && usarWifiManager==true){	 
				wifiManager.setTimeout(300);
				wifiManager.setDebugOutput(debugWifiManager);
				//wifiManager.autoConnect(DEVICE_NAME);
				wifiManager.startConfigPortal(DEVICE_NAME);
			}     
		}
	}

    unsigned long startAttemptTime = millis();

    // Keep looping while we're not connected and haven't reached the timeout
	while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {}   
	
    // If we couldn't connect within the timeout period, retry in 30 seconds.
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI]: FAILED");
      conteoIntentosConexion++;
	  servidorWifi_iniciado = false;
      vTaskDelay(30000 / portTICK_PERIOD_MS);
      continue;
    } else {
      conteoIntentosConexion = 0;
	  thereConectionWifi = true;
    }
	
	if(IniciarServidorWifi==true){
		Serial.print("[WIFI]: Connected: ");
		Serial.println(WiFi.localIP());
		thereConectionWifi = true;
		
		server_OTA.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
		  request->send(200, "text/plain", "Version Libreria Internet: 1.0.");
		});

		AsyncElegantOTA.begin(&server_OTA);    // Start ElegantOTA
		server_OTA.onNotFound(notFound);
		server_OTA.begin();
		Serial.println("[WIFI]: Activando OTA.");
		servidorWifi_iniciado = true;		
	} 
	
    haveIpOutFor = false;   
  }
}

bool getStatusConectionwifi(){
	if(thereConectionAP==true){
		//Serial.println("[WIFI]: no AP activo.");
		return true;
	}else{
		return thereConectionWifi;
	}		
}

bool isActiveUseWifi(){
	return actived_use_wifi;
}

#endif









/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                ETHERNET TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

#ifdef useEth
#define ETH_TIMEOUT 20000 // 20 seconds

#ifndef useAP
	#ifndef useWifi
		bool thereConectionAP = false;
	#endif
#endif

#include <WiFi.h>   //para obtener la MAC wifi
//#include <esp_wifi.h>    //aveces se requiere para obtener el ChipID
#include <SPI.h>
#include <Ethernet.h>

#define RESET_P  26        // Tie the Wiz820io/W5500 reset pin to ESP32 GPIO26 pin.

bool actived_use_eth = false;

TaskHandle_t nucleo1EthKeepAlive_Handle = NULL;
void keepEthAlive( void *pvParameters );

int ConvertChar(char c) {
  switch (c) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
      return 10;
    case 'B':
      return 11;
    case 'C':
      return 12;
    case 'D':
      return 13;
    case 'E':
      return 14;
    case 'F':
      return 15;
  }
  return -1;
}

uint8_t ConvertChartoMac(char h1, char h0) {
  h1 = ConvertChar(h1);
  h0 = ConvertChar(h0);
  return int(h1) * 16 + h0;
}

void ConfigEthernetKeepAlive() {
  xTaskCreatePinnedToCore(
    keepEthAlive,      // Function that should be called
    "keepEthAlive",    // Name of the task (for debugging)
    EspacioMemoriaEthernet,         // Stack size (bytes)
    NULL,         // Parameter to pass
    1 ,           // Task priority
    &nucleo1EthKeepAlive_Handle,         // Task handle
    1             // Core you want to run the task on (0 or 1)
  );
}

void keepEthAlive(void * parameter) {
  //Serial.print("[ETHERNET]: [Core 1]: "); Serial.println(xPortGetCoreID());

  thereConectionEthernet = false;

  Ethernet.init(5);           // GPIO5 on the ESP32.   // Use Ethernet.init(pin) to configure the CS pin.

  { //WizReset
    /**
        Reset Ethernet: Wiz W5500 reset function.
    **/
    Serial.print("[ETHERNET]: Resetting Wiz W5500 Ethernet Board...  ");
    pinMode(RESET_P, OUTPUT);
    digitalWrite(RESET_P, HIGH);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    digitalWrite(RESET_P, LOW);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    digitalWrite(RESET_P, HIGH);
    vTaskDelay(350 / portTICK_PERIOD_MS);
    //Serial.println("Done.");
  }
  /****************************************/

  uint8_t eth_MAC[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  { //Asignar MAC para el modulo Ethernet
    bool GenerarMacAutomatica = true;
    /*
      Para asignar la MAC que se usará en el modulo Ethernet, tomo los primeros 3 hexetos de la mac wifi,
      estos tres primeros hexetos me definen el fabricante del producto, por lo cual uso el mismo fabricante del ESP para generar la MAC Ethernet

      Los siguientes tres hexetos finales los defino usando el chipID del ESP, este ID es un codigo unico donde está el serial del dispositivo, contiene entre 7 u 8 indices
      para lo cual divido en dos caracteres iniciando por las unidades, es decir,
      decenas y unidades             corresponden a el hexeto 6
      milesimas y centenas           corresponden a el hexeto 5
      centenar_de_mil y diezmilesima corresponden a el hexeto 4

      por ejemplo:
        MAC    = AA:BB:CC:DD:EE:FF
        ChipID = 76543210

        Quedaría:
        MAC_Ethernet = AA:BB:CC:54:32:10
    */
    if (GenerarMacAutomatica) {
      String mac_string = WiFi.macAddress();

      //String chipID_string = String(ESP.getChipId());
      String chipID_string = String((uint32_t)ESP.getEfuseMac());
      byte sizeChipID = chipID_string.length();
      char chipID_char[sizeChipID + 1];
      chipID_string.toCharArray(chipID_char, sizeChipID + 1);

      char mac_char[mac_string.length() + 1];
      mac_string.toCharArray(mac_char, mac_string.length() + 1);

      eth_MAC[0] = ConvertChartoMac(mac_char[0], mac_char[1]);
      eth_MAC[1] = ConvertChartoMac(mac_char[3], mac_char[4]);
      eth_MAC[2] = ConvertChartoMac(mac_char[6], mac_char[7]);
      //eth_MAC[3] = ConvertChartoMac(mac_char[9], mac_char[10]);
      //eth_MAC[4] = ConvertChartoMac(mac_char[12], mac_char[13]);
      //eth_MAC[5] = ConvertChartoMac(mac_char[15], mac_char[16]);

      eth_MAC[3] = ConvertChartoMac(chipID_char[sizeChipID - 5], chipID_char[sizeChipID - 4]);
      eth_MAC[4] = ConvertChartoMac(chipID_char[sizeChipID - 3], chipID_char[sizeChipID - 2]);
      eth_MAC[5] = ConvertChartoMac(chipID_char[sizeChipID - 1], chipID_char[sizeChipID]);
    }
  }

  { //Imprimir MAC
    Serial.print("[ETHERNET]: MAC: ");
    for (byte i = 0; i < 6; i++) {
      Serial.print(eth_MAC[i], HEX);
      if (i < 5) {
        Serial.print(":");
      }
    }
    Serial.println();
  }

  bool thereConection = false;
  { //Buscar IP
    while (!thereConection) {
      Serial.println("[ETHERNET]: Connecting...");
      if (Ethernet.begin(eth_MAC) == 0) {
        //Serial.println("[ETHERNET]: Failed to configure Ethernet using DHCP");
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          //Serial.println("[ETHERNET]: Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          //Serial.println("[ETHERNET]: Ethernet cable is not connected.");
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS);
      } else {
        thereConection = true;
        thereConectionEthernet = true;
      }
    }
  }
  Serial.print("[ETHERNET]: DHCP assigned IP: ");
  Serial.println(Ethernet.localIP());

  unsigned long startAttemptTime = millis();
  for (;;) {
    if (millis() - startAttemptTime > ETH_TIMEOUT) {
      startAttemptTime = millis();
      Serial.print("[ETHERNET]: [Core 1]: "); Serial.println(xPortGetCoreID());
    }

    { //Buscar fallos en el ethernet relacionados con el cambio de ip por dhcp para corregirlo
      switch (Ethernet.maintain()) {
        case 1:
          //renewed fail
          //Serial.println("[ETHERNET]: Error: renewed fail");
          thereConectionEthernet = false;
          break;
        case 2:
          //renewed success
          //Serial.println("[ETHERNET]: Renewed success");
          //print your local IP address:
          /*Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());*/
          break;
        case 3:
          //rebind fail
          //Serial.println("[ETHERNET]: Error: rebind fail");
          thereConectionEthernet = false;
          break;
        case 4:
          //rebind success
          //Serial.println("[ETHERNET]: Rebind success");
          //print your local IP address:
          /*Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());*/
          break;
        default:
          //nothing happened
          break;
      }
    }

    vTaskDelay(15 / portTICK_PERIOD_MS);
  }
}

bool getStatusConectionEthernet(){
	if(thereConectionAP){
		return true;
	}else{
		return thereConectionEthernet;		
	}	
}

bool isActiveUseEthernet(){
	return actived_use_eth;
}
#endif





/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                AP TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/


#ifdef useAP

#define email_portal_cautivo	"wisrovi.rodriguez@gmail.com"
#define website_portal_cautivo	"https://wisrovirodriguez.wixsite.com/wisrovi"
#define linkedin_portal_cautivo	"https://www.linkedin.com/in/wisrovi-rodriguez"
//http://conectivitycheck.gstatic.com/


#include <WiFi.h>
#include <WiFiAP.h>
#include <ArduinoJson.h>
DynamicJsonDocument clientsStations(1024);
#include "esp_wifi.h"

TaskHandle_t nucleo1WifiAP_Handle = NULL;
void WifiAP( void *pvParameters );

bool AP_started = false;
bool actived_use_AP = false;


#include <DNSServer.h>
#include <AsyncTCP.h>
#ifndef useWifi		
	DynamicJsonDocument rutasUrl(1024);
	#include <Hash.h>
	#include <AsyncTCP.h>
	#include <ESPAsyncWebServer.h>
	#include <AsyncElegantOTA.h>
	AsyncWebServer server_OTA(80);
	typedef std::function<void(AsyncWebServerRequest *request)> THandlerFunction;
	bool RegistrarNuevoServicioGet(String url, THandlerFunction fn){
		bool servicioCreado = false;
		
		
		bool estaActivoUsoWii = false;
		#ifdef useWifi
			estaActivoUsoWii = thereConectionWifi;
		#endif
			
		if(thereConectionAP || estaActivoUsoWii){
			char url_char[url.length()+1];
			url.toCharArray(url_char, url.length()+1);
			bool existeRuta = rutasUrl[url];;
			if(existeRuta==false){
				server_OTA.on(url_char, HTTP_GET, fn);
				rutasUrl[url] = true;
				servicioCreado = true;
			}else{
				Serial.print("[AP]: El servicio '");	
				Serial.print(url);	
				Serial.println("' no se puede crear, ya existe o esta reservado.");	
			}		
			/*String allStationsConected;
			serializeJson(rutasUrl, allStationsConected);
			Serial.print("[AP]: ");
			Serial.println(allStationsConected);*/
		}
		return servicioCreado;
	}

	void notFound(AsyncWebServerRequest *request) {
	  request->send(404, "text/plain", "Not found");
	}
#endif

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

void linkedin(AsyncWebServerRequest *request){
  request->send(200, "text/plain", linkedin_portal_cautivo);
}

void Website(AsyncWebServerRequest *request){
  request->send(200, "text/plain", website_portal_cautivo);
}

void email(AsyncWebServerRequest *request){
  request->send(200, "text/plain", email_portal_cautivo);
}

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //request->addInterestingHeader("ANY");
      // redirect if not in wifi client mode (through filter)
      // and request for different host (due to DNS * response)
      if (request->host() != WiFi.softAPIP().toString()){
        return true;
      }else{
        return false;
      }      
    }

    void handleRequest(AsyncWebServerRequest *request) {      
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->print("<!DOCTYPE html>");
      response->print("<html>");
        response->print("<head>");
          //response->print("<meta http-equiv='refresh' content='5'/>");
          response->print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\"/>");
          response->print("<title>Captive Portal</title>");
          response->print("<style> body {  background-color: rgb(43, 42, 42);  text-align: center;  color: white; }</style>");        
        response->print("</head>");
        response->print("<body>");
          response->print("<p>&nbsp;</p>");
          response->print("<h1 style=\"text-align: center;\">Portal Cautivo</h1>");
          response->print("<h3 style=\"text-align: center;\">P&aacute;gina de Informaci&oacute;n</h3>");
          response->print("<p style=\"text-align: center; \"><strong>AVISO:  </strong><em><span style=\"color: #ff0000; font-size: large;\">Para garantizar que el m&oacute;vil no se desconecte del AP, se recomienda:</span>");
          response->print("<br/><br/>  <input type=\"checkbox\" value=\"1\" />&nbsp;&nbsp;<span style=\"color: #ff0000; font-size: large;\">Apagar los datos m&oacute;viles  </span></input> ");       
     	  response->print("<br/><br/><input type=\"checkbox\" value=\"3\" />&nbsp;&nbsp;<span style=\"color: #ff0000; font-size: large;\">Apagar VPN (si tiene alguna)</span></input> ");    
     	  response->print("<br/><br/>  <input type=\"checkbox\" value=\"2\" />&nbsp;&nbsp;<span style=\"color: #ff0000; font-size: large;\">Quitar el proxy (si tiene alguno)</span></input> </em></p><p>&nbsp;</p>"); 
		  //response->print("");    		  
					
          response->print("<p style=\"text-align: center;\"><em>Para poder realizar las diferentes solicitudes al AP se recomienda usar la app proporcionada por el fabricante.</em></p>");
          response->print("<p>&nbsp;</p>");
          
          response->print("<p style=\"text-align: center;\">Creado por: Wisrovi Rodriguez.</p>");
		  response->print("<p style=\"text-align: center; \">");
          response->printf("&nbsp;      &nbsp;<a title=\"Email author\" href=\"http://%s%s\" target=\"_blank\" rel=\"noopener\">email</a>", request->host().c_str(), "/Website");        
          response->printf("&nbsp;      &nbsp;<a title=\"Linkedin autor\" href=\"http://%s%s\" target=\"_blank\" rel=\"noopener\">linkedin</a>", request->host().c_str(), "/Website");       
          response->printf("&nbsp;      &nbsp;<a title=\"Curriculum vitae autor\" href=\"http://%s%s\" target=\"_blank\" rel=\"noopener\">Website</a>", request->host().c_str(), "/Website");       
		  //response->printf("<p style=\"text-align: center;color: greenyellow !important;\"><a title=\"Curriculum vitae autor\" href=\"http://%s%s\" target=\"_blank\" rel=\"noopener\">Website</a></p>", request->host().c_str(), "/Website");       
        response->print("</body>");
      response->print("</html>");      
      request->send(response);
    }
};

bool ConfigWifiAP(String nombreAP, String passwordAP) {
	if(thereConectionAP){
	  nombreAP.toCharArray(nombreAP_char, 25);
	  passwordAP.toCharArray(passwordAP_char, 25);

	  xTaskCreatePinnedToCore(
		WifiAP
		,  "WifiAP"   // A name just for humans
		,  EspacioMemoriaWifiAP  // // Stack size (bytes),
		,  NULL  // Parameter to pass
		,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  &nucleo1WifiAP_Handle
		,  1);
		return true;
	}else{		
		return false;
	}  
}

bool IniciarServidorAP = false;
bool servidorAP_iniciado = false;

bool setIniciarServidorAP(){
	if(thereConectionAP){
		IniciarServidorAP = true;
	}
	return IniciarServidorAP;
}

void ProcessClient(String ipCliente) {  
	if(thereConectionAP){
		for (byte i = 0; i < clientsStations.size(); i++) {
			DynamicJsonDocument doc(128);
			doc[String(i)] = clientsStations[String(i)];
			String ip_save = doc[String(i)]["ip"];
			if (ipCliente.equals(ip_save)) {
			  doc[String(i)]["active"] = true;
			  clientsStations[String(i)] = doc[String(i)];
			  break;
			}
		}
		String allStationsConected;
		serializeJson(clientsStations, allStationsConected);
		Serial.print("[AP]: ");
		Serial.println(allStationsConected);
	}  
}

void ProcesarClienteRequest(AsyncWebServerRequest *request){
	String ipCliente = request->client()->remoteIP().toString().c_str();
	ProcessClient(ipCliente);
}

void WifiAP(void *pvParameters) {
  (void*) pvParameters;
  Serial.print("Nucleo: ");
  Serial.println(xPortGetCoreID());
  
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(nombreAP_char, passwordAP_char);
    
  #ifdef useCaptivePortal
  //dnsServer.setTTL(60*5);  // default is 60 seconds, modify TTL associated  with the domain name (in seconds)
  //dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  #endif
  
  server_OTA.on("/email", HTTP_GET, email);
  server_OTA.on("/linkedin", HTTP_GET, linkedin);
  server_OTA.on("/Website", HTTP_GET, Website);
  rutasUrl["/email"] = true;
  rutasUrl["/linkedin"] = true;
  rutasUrl["/Website"] = true;
  server_OTA.onNotFound(notFound);
  
  #ifdef useCaptivePortal
  server_OTA.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  #endif
	
  Serial.println();
  Serial.print("[AP]: SSID: ");
  Serial.println(nombreAP_char);
  Serial.print("[AP]: PWD: ");
  Serial.println(passwordAP_char);
  Serial.print("[AP]: Gate: ");
  Serial.println(WiFi.softAPIP());

  vTaskDelay(2500 / portTICK_PERIOD_MS); 
  
  AP_started = true;

  byte numStationsConected = 0;
  unsigned long timeGetCheckNumberStationsConected = millis();
  for (;;) {
	if(IniciarServidorAP==true && servidorAP_iniciado==false){
		servidorAP_iniciado = true;
		
		AsyncElegantOTA.begin(&server_OTA);    // Start ElegantOTA
		server_OTA.begin();
		Serial.println("[AP]: Iniciando Servidor");
	}
	
	if(servidorAP_iniciado){
		#ifdef useCaptivePortal
		dnsServer.processNextRequest();
		#endif
		AsyncElegantOTA.loop();		
	}else{
		//Serial.println("No se ha iniado el servidor.");
	}
		
    if (millis() - timeGetCheckNumberStationsConected > 1000) {
      timeGetCheckNumberStationsConected = millis();
      byte cantidadEstacionesConectadas = WiFi.softAPgetStationNum(); //https://esp8266-arduino-spanish.readthedocs.io/es/latest/esp8266wifi/soft-access-point-class.html
      if (numStationsConected != cantidadEstacionesConectadas) {
        if (numStationsConected > cantidadEstacionesConectadas) {
          Serial.println("[AP]: cliente desconectado");
        } else {
          Serial.println("[AP]: Nuevo cliente detectado");

          tcpip_adapter_sta_list_t adapter_sta_list;
          { //extraigo la lista de estaciones
            wifi_sta_list_t wifi_sta_list;
            memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
            memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
            esp_wifi_ap_get_sta_list(&wifi_sta_list);
            tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
          }

		  byte conteoErrores = 0;
          for (int idStation = 0; idStation < adapter_sta_list.num; idStation++) {
            tcpip_adapter_sta_info_t station = adapter_sta_list.sta[idStation];
            String mac = "";
            String ip = "";
            { //leo la MAC y la IP
              for (int i = 0; i < 6; i++) {
                mac.concat(String(station.mac[i], HEX));
                if (i < 5) {
                  mac.concat(":");
                }
              }
              ip = ip4addr_ntoa(&(station.ip));
            }

            bool mac_and_ip_ok = false;
            bool ya_esta_creada = false;
            if (!mac.equals("0:0:0:0:0:0") || !ip.equals("0.0.0.0")) {  //confirmo que los datos son datos_validos y que no se encuentren creados
              mac_and_ip_ok = true;
              { //buscar y si encuentro una coincidencia, en este caso de la MAC
                for (byte i = 0; i < clientsStations.size(); i++) {
                  DynamicJsonDocument doc(128);
                  doc[String(i)] = clientsStations[String(i)];
                  String mac_save = doc[String(i)]["mac"];
                  if (mac.equals(mac_save)) {
                    ya_esta_creada = true;
                    //doc[String(i)]["active"] = true;
                    clientsStations[String(i)] = doc[String(i)];
                    break;
                  }
                }
              }
            }else{
				Serial.print("[AP]: Error detectar mac o ip en cliente -> ");
				Serial.print("[ MAC: ");
				Serial.print(mac);
				Serial.print(" - IP: ");
				Serial.print(ip);
				Serial.println(" ]");
				conteoErrores++;
				if(conteoErrores<adapter_sta_list.num){
					idStation--;
				}
			}

            if (mac_and_ip_ok && !ya_esta_creada) {  //inserto en el array los datos de mac e ip, siempre que no existan
              if (clientsStations.size() == 0) {
                //Serial.println("[AP]: Creando Json estaciones conectadas.");
              }
              DynamicJsonDocument thisStation(128);              
			  if (!mac.equals("0:0:0:0:0:0")){
				  thisStation["mac"] = mac;
			  }else{
				  Serial.print("[AP]: Error detectar mac, la ip es: ");Serial.println(ip);
			  }
              if (!ip.equals("0.0.0.0")){
				  thisStation["ip"] = ip;
			  }else{
				  Serial.print("[AP]: Error detectar ip, la mac es: ");Serial.println(mac);
			  }
              thisStation["id"] = idStation;
              thisStation["active"] = false;
              clientsStations[String(idStation)] = thisStation;
            }
          }          
          //Serial.println("********************************************************");
        }
        numStationsConected = cantidadEstacionesConectadas;
      }
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    //vTaskDelete( NULL );*/
  }
}

String getClientes(){
  String output;
  serializeJson(clientsStations, output);
  return output;
}

bool getAPStart() {
	if(thereConectionAP){
		return AP_started;
	}else{
		return true;
	}  
}

bool isActiveUseAP(){
	return actived_use_AP;
}

#endif











/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                       SEND AND RESPONSE TASK
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/

String _ip;
int _port;
String _urlSolicitud;
bool _enviar_get = false;

#include <RestClient.h>
RestClient client = RestClient("192.168.1.5");
String respuestaGet = "";

#include <HTTPClient.h>

TaskHandle_t nucleo1InternetResponse_Handle = NULL;
void internetResponse( void *pvParameters );
#define EspacioMemoriaInternetResponse  1800

bool ConfigInternet(bool usarWifi, bool usarEth, bool isAP) {
  bool existConfigRed = false;
  
  if(isAP){	
	#ifdef useAP	
		actived_use_AP = isAP;
		existConfigRed = true;	
		thereConectionAP = true;
	#endif
  }else{
	#ifdef useWifi
	  actived_use_wifi = usarWifi;
	  if (usarWifi) {
		existConfigRed = true;	   
		thereConectionAP = false;
		ConfigWifiKeepAlive();
	  }
	#endif

	#ifdef useEth
	  actived_use_eth = usarEth;
	  if (usarEth) {
		thereConectionEthernet = true;
		existConfigRed = true;
		ConfigEthernetKeepAlive();

		client.setHeader("Host: www.wisrovi.com");
		client.setHeader("Connection: close");
	  }
	#endif

	  if (existConfigRed) {
		xTaskCreatePinnedToCore(
		  internetResponse,      // Function that should be called
		  "internetResponse",    // Name of the task (for debugging)
		  EspacioMemoriaInternetResponse,         // Stack size (bytes)
		  NULL,         // Parameter to pass
		  1 ,           // Task priority
		  &nucleo1InternetResponse_Handle,         // Task handle
		  1             // Core you want to run the task on (0 or 1)
		);
	  }
  } 
  return existConfigRed;
}

bool thereInternet(){
	bool hayUsoEthernet = false;
	bool hayUsoWifi = false;
	#ifdef useEth
		hayUsoEthernet = thereConectionEthernet;
	#endif
	#ifdef useWifi
		hayUsoWifi = thereConectionWifi;
	#endif
	return hayUsoEthernet || hayUsoWifi;
}

void internetResponse(void * parameter) {
  respuestaGet = "";
  _enviar_get = false;

  for(;;){
    vTaskDelay(15 / portTICK_PERIOD_MS);
    if(_enviar_get){
      _enviar_get = false;

      bool thereInternet = false;
      respuestaGet = "";

    #ifdef useEth  
      if (thereConectionEthernet) {    
        bool checkHardwareEthernet = true;
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          //Serial.println("[ETHERNET]: Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          checkHardwareEthernet = false;
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          //Serial.println("[ETHERNET]: Ethernet cable is not connected.");
          checkHardwareEthernet = false;
        }
    
        if (checkHardwareEthernet) {
          thereInternet = true;   
          char ip_char[_ip.length() + 1];
          _ip.toCharArray(ip_char, _ip.length() + 1);
    
          char urlSolicitud_char[_urlSolicitud.length() + 1];
          _urlSolicitud.toCharArray(urlSolicitud_char, _urlSolicitud.length() + 1);
    
          int statusCode = client.get(ip_char, _port, urlSolicitud_char, &respuestaGet);
          if (statusCode == 200) {
            //Serial.print("[ETHERNET]: Rta get: ");
            //Serial.println(respuestaGet);
          } else {
            Serial.print("[ETHERNET]: Status get: ");
            Serial.println(statusCode);
          }
        }
      }
    #endif

    #ifdef useWifi
      if (!thereInternet) {
        if (thereConectionWifi) {
          thereInternet = true;
    
          String urlSend = "http://";
          urlSend.concat(_ip);
          urlSend.concat(":");
          urlSend.concat(String(_port));
          urlSend.concat(_urlSolicitud);
          //Serial.println(urlSend);
    
          HTTPClient http;
          http.begin(urlSend); // configure traged server and url
          int httpCode = http.GET(); // start connection and send HTTP header
          if (httpCode > 0  ) {  // httpCode will be negative on error
            // HTTP header has been send and Server response header has been handled
            if (httpCode == HTTP_CODE_OK) {  // file found at server
              respuestaGet = http.getString();
              //Serial.print("[WIFI]: Rta get: ");
              //Serial.println(respuestaGet);
            }
          } else {
            Serial.print("[WIFI]: Error get: ");
            Serial.println(http.errorToString(httpCode).c_str());
          }
          http.end();
        }
      }
    #endif

      if (!thereInternet) {
        Serial.println("No hay internet, no se puede enviar el mensaje.");
      }
    }
  }  
  //vTaskDelete(NULL); //borrar esta tarea
}

void SendGet(String ip, int port, String urlSolicitud) {
  _ip = ip;
  _port = port;
  _urlSolicitud = urlSolicitud;
  _enviar_get = true;
}

String getResponse(){
  return respuestaGet;
}




























/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *                                                                          *
                                  EXAMPLE
 *                                                                          *
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 *****************************************************************************/



/**
Configurar que deseo activar de la libreria: ethernet, wifi o ambos
esto va en el setup


bool usarWifi = true;
  bool usarEth = true;
  ConfigInternet(usarWifi, usarEth);

*/



/***


    Para enviar un mensaje se debe:
    SendGet("ip", puerto, "rutaGet");

*/



/**

    Para recibir la respuesta se debe esperar al menos 2 segundos, luego se captura la respuesta con:
    getResponse()

*/
