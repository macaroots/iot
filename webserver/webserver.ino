#include "WiFi.h"
#include <WebServer.h>
#include "DHT.h"
#include <Adafruit_SSD1306.h>
#include <env.h>

// Configurações da rede Wi-Fi
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

const int LDR_PIN = 1;
const int LED_PIN = 14;
const int DHT_PIN = 15;
const int LIMIAR_LUZ = 5000;
int ldr;
int temperatura;
int umidade;
float estadoLuz = LOW;
IPAddress ip;
bool automatico = true;

void mudar_luz(float estado);

WebServer server(80);
WiFiClient wifi;

DHT dht(DHT_PIN, DHT11);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);

  dht.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE, BLACK);
  display.display();
  display.clearDisplay();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Aguarde conectar-se à rede...");
    display.println("Aguarde conectar-se à rede...");
    display.display();
    delay(500);
  }

  Serial.println("Conectado com sucesso!");
  Serial.print("IP: ");
  ip = WiFi.localIP();
  Serial.println(ip);

  server.on("/", handle_index);
  server.on("/ligar", handle_ligar);
  server.on("/desligar", handle_desligar);
  server.on("/automatico", handle_automatico);
  server.onNotFound(handle_not_found);
  
  server.begin();
  
  Serial.println("Web Server Ativo");
}

void loop() {
  server.handleClient();

  int temp_temperatura = dht.readTemperature();
  int temp_umidade = dht.readHumidity();
  if (!isnan(temp_temperatura)) {
    temperatura = temp_temperatura;
  }
  if (!isnan(temp_umidade)) {
    umidade = temp_umidade;
  }
  
  ldr = analogRead(LDR_PIN);
  if (automatico) {
    if (ldr < LIMIAR_LUZ) {
      int luz = 255 - map(ldr, 1000, LIMIAR_LUZ, 0, 255);
      mudar_luz(luz);
    }
    else {
      mudar_luz(LOW);
    }
  }

  mostra_tela();

  delay(5);
}

void mostra_tela() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("IP:" + ip.toString());
  display.println("Luminosidade:" + String(ldr));
  display.println("Temperatura:" + String(temperatura));
  display.println("Umidade:" + String(umidade));
  display.display();
}

String getPage() {
  String estado = (estadoLuz > 0) ? "Ligado (" + String(estadoLuz) + ")" : "Desligado";
  String html = "<html><meta charset=\"utf-8\" /><body>";
  html += "<div><span>Estado da luz:</span> " + estado + "</div>";
  html += "<div><span>Luminosidade:</span> " + String(ldr) + "</div>";
  html += "<div><span>Temperatura:</span> " + String(temperatura) + "</div>";
  html += "<div><span>Umidade:</span> " + String(umidade) + "</div>";
  html += "<div><a href=\"/ligar\">Ligar</a></div>";
  html += "<div><a href=\"/desligar\">Desligar</a></div>";
  html += "<div><a href=\"/automatico\">Automático</a></div>";
  html += "</body></html>";
  return html;
}

void handle_index() {
  server.send(200, "text/html", getPage());
}

void mudar_luz(float estado) {
  estadoLuz = estado;
  analogWrite(LED_PIN, estadoLuz);
}

void handle_ligar() {
  automatico = false;
  mudar_luz(255);
  server.send(200, "text/html", getPage());
}

void handle_desligar() {
  automatico = false;
  mudar_luz(LOW);
  server.send(200, "text/html", getPage());
}

void handle_automatico() {
  automatico = true;
  server.send(200, "text/html", getPage());
}

void handle_not_found() {
  server.send(404, "text/plain", "Página não encontrada");
}