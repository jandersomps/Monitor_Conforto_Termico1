#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURAÇÕES ---
const char* ssid = "Li&Jandinho";
const char* password = "w26j21_12032022";
const char* mqtt_server = "192.168.2.30"; // Ex: 192.168.1.100

#define DHTPIN 4
#define DHTTYPE DHT11
#define BUTTON_PIN 14
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient espClient;
PubSubClient client(espClient);

int telaAtual = 0;
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.print("Conectando em: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tentativas++;
    if (tentativas > 20) { // Se passar de 10 segundos
       Serial.println("\nFalha ao conectar. Verificando sinal...");
       tentativas = 0;
    }
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
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
  
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  
  dht.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Alternar tela com o botão
  if (digitalRead(BUTTON_PIN) == LOW) {
    telaAtual = !telaAtual;
    delay(200); // Debounce simples
  }

  unsigned long now = millis();
  if (now - lastMsg > 5000) { // Envia a cada 5 segundos
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      // Publicando dados para o HA
      client.publish("home/quarto/temp", String(t).c_str());
      client.publish("home/quarto/umid", String(h).c_str());
      
      // Atualiza o Display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      
      if(telaAtual == 0) {
        display.println("CONFORTO TERMICO");
        display.printf("\nTemp: %.1f C", t);
        display.printf("\nUmid: %.1f %%", h);
      } else {
        display.println("STATUS REDE");
        display.printf("\nWiFi: Conectado");
        display.printf("\nIP: %s", WiFi.localIP().toString().c_str());
      }
      display.display();
    }
  }
}