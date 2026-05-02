#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <Preferences.h>

// --- CONFIGURAÇÕES ---
// Servidor MQTT (será configurado via portal)
char mqtt_server[40] = "192.168.2.30"; // Valor padrão

#define DHTPIN 4
#define DHTTYPE DHT11
#define BUTTON_PIN 14        // Botão para alternar telas
#define RESET_BUTTON_PIN 12  // Botão para resetar configurações
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient espClient;
PubSubClient client(espClient);

// Configuração NTP (UTC -3 para Brasília)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);

const char* diasSemana[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

int telaAtual = 0;
unsigned long lastMsg = 0;
unsigned long lastDisplayUpdate = 0;
float ultimaTemp = 0;
float ultimaUmid = 0;
unsigned long resetButtonPressTime = 0;
bool resetConfigMode = false;

Preferences preferences;

void setup_wifi() {
  WiFiManager wm;
  
  // Carrega o servidor MQTT salvo
  preferences.begin("mqtt-config", false);
  String savedServer = preferences.getString("server", "192.168.2.30");
  savedServer.toCharArray(mqtt_server, 40);
  preferences.end();
  
  // Cria campo customizado para o servidor MQTT
  WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", mqtt_server, 40);
  wm.addParameter(&custom_mqtt_server);
  
  // Callback quando salvar configurações
  wm.setSaveParamsCallback([]() {
    Serial.println("Salvando configurações...");
  });
  
  // Define timeout do portal (3 minutos)
  wm.setConfigPortalTimeout(180);
  
  // Mostra no display que está configurando
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("CONFIGURACAO WiFi");
  display.println("\nConecte em:");
  display.println("ESP32_ConfortoAP");
  display.println("\nAcesse:");
  display.println("192.168.4.1");
  display.display();
  
  // Tenta conectar, se falhar abre o portal
  if (!wm.autoConnect("ESP32_ConfortoAP", "12345678")) {
    Serial.println("Falha ao conectar e timeout atingido");
    delay(3000);
    ESP.restart();
  }
  
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Salva o servidor MQTT
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  preferences.begin("mqtt-config", false);
  preferences.putString("server", mqtt_server);
  preferences.end();
  
  Serial.print("Servidor MQTT configurado: ");
  Serial.println(mqtt_server);
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Tenta conectar
    if (client.connect("ESP32_Conforto_Termico", "homeassistant", "@terra123")) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Dá 2 segundos para o monitor serial estabilizar
  Serial.println("\n--- ESP32 INICIALIZADO ---");
  
  // Inicializa o display ANTES de tudo
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao inicializar display!");
    for(;;); // Loop infinito se falhar
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,20);
  display.println("  Iniciando...");
  display.display();
  delay(1000);
  
  // Agora configura WiFi (que usa o display)
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  
  // Inicializa NTP
  timeClient.begin();
  timeClient.update();
  Serial.println("NTP sincronizado!");
  
  dht.begin();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  
  // Atualiza hora periodicamente
  timeClient.update();

  // Botão para alternar telas (pressão simples)
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    telaAtual = !telaAtual;
    delay(200); // Debounce
  }
  lastButtonState = currentButtonState;
  
  // Botão de RESET (segurar por 3 segundos)
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (resetButtonPressTime == 0) {
      resetButtonPressTime = millis();
      // Mostra aviso no display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,20);
      display.println("  Segure para");
      display.println("  resetar...");
      display.display();
    }
    // Segurando por 3 segundos reseta as configurações
    if (millis() - resetButtonPressTime > 3000 && !resetConfigMode) {
      resetConfigMode = true;
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,20);
      display.println("  RESETANDO...");
      display.println("\n  Configuracoes");
      display.println("  apagadas!");
      display.display();
      delay(2000);
      
      WiFiManager wm;
      wm.resetSettings();
      preferences.begin("mqtt-config", false);
      preferences.clear();
      preferences.end();
      
      ESP.restart();
    }
  } else {
    // Botão de reset solto
    resetButtonPressTime = 0;
  }

  unsigned long now = millis();
  
  // Lê sensor e publica MQTT a cada 5 segundos
  if (now - lastMsg > 5000) {
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      ultimaTemp = t;
      ultimaUmid = h;
      // Publicando dados para o HA
      client.publish("home/quarto/temp", String(t).c_str());
      client.publish("home/quarto/umid", String(h).c_str());
    }
  }
  
  // Atualiza display a cada 500ms para hora em tempo real
  if (now - lastDisplayUpdate > 500) {
    lastDisplayUpdate = now;
    
    // Obtém data e hora
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime(&epochTime);
    int diaSemana = timeClient.getDay();
    int dia = ptm->tm_mday;
    int mes = ptm->tm_mon + 1;
    int ano = ptm->tm_year + 1900;
    String horaFormatada = timeClient.getFormattedTime();
    
    // Atualiza o Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    if(telaAtual == 0) {
      // Tela principal com data/hora e dados
      display.printf("%s %02d/%02d/%d", diasSemana[diaSemana], dia, mes, ano);
      display.setCursor(0, 12);
      display.setTextSize(2);
      display.printf("%s", horaFormatada.c_str());
      display.setTextSize(1);
      display.setCursor(0, 32);
      display.println("---------------");
      display.setCursor(0, 42);
      display.printf("Temp: %.1fC", ultimaTemp);
      display.setCursor(0, 54);
      display.printf("Umid: %.1f%%", ultimaUmid);
    } else {
      // Tela de status da rede
      display.println("STATUS REDE");
      display.println("---------------");
      display.printf("SSID: %s\n", WiFi.SSID().c_str());
      display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
      display.printf("Sinal: %d dBm\n", WiFi.RSSI());
      display.printf("\nMQTT: %s", client.connected() ? "OK" : "Erro");
    }
    display.display();
  }
}