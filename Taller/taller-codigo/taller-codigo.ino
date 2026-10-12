#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

const char* WIFI_SSID = "Alejo";
const char* WIFI_PASSWORD = "pupusa123";


// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================


const char* MQTT_SERVER = "192.168.43.108";
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";




// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// Definir los pines y objetos necesarios para los sensores.
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 32

DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {
//1. Obtener la temperatura.
  float temp = dht.readTemperature();
  
// 2. Obtener la humedad.
  float hum = dht.readHumidity();
// 3. Obtener la lectura del sensor de iluminación.
  int luz = analogRead(LDRPIN)/100;

//4. Preparar los valores para poder publicarlos mediante MQTT.
  char tempStr[8];
  char humStr[8];
  char luzStr[8];

  dtostrf(temp, 1, 2, tempStr);
  dtostrf(hum, 1, 2, humStr);
  snprintf(luzStr, sizeof(luzStr), "%d", luz);

//5. Publicar cada dato en su topic correspondiente:
  mqttClient.publish(TOPIC_TEMPERATURA, tempStr);
  mqttClient.publish(TOPIC_HUMEDAD, humStr);
  mqttClient.publish(TOPIC_LUZ, luzStr);

// 6. Mostrar las mediciones en el Monitor Serial.
  Serial.print("Temperatura: ");
  Serial.print(tempStr);
  Serial.print("° C | Humedad: ");
  Serial.print(humStr);
  Serial.print(" % ");
  Serial.println(luzStr);

}

void setup() {

  Serial.begin(115200);

  // Inicializar los sensores y configurar los pines
  // necesarios.
  dht.begin();
  pinMode(LDRPIN, INPUT);

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();


  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}