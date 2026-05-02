# Monitor de Conforto Térmico

## Descrição
Projeto de monitoramento de temperatura e umidade usando ESP32, com display OLED e integração com Home Assistant via MQTT.

## Componentes Utilizados
- **Microcontrolador**: ESP32
- **Sensor**: DHT11 (temperatura e umidade)
- **Display**: OLED SSD1306 (128x64 pixels)
- **Botão**: GPIO 14 (para alternar telas)

## Funcionalidades
- ✅ Leitura de temperatura e umidade a cada 5 segundos
- ✅ Exibição em display OLED com duas telas alternadas:
  - **Tela 1**: Dados de conforto térmico (temperatura e umidade)
  - **Tela 2**: Status da rede (WiFi e IP)
- ✅ Envio de dados para Home Assistant via MQTT
- ✅ Conexão WiFi automática com detecção de falhas
- ✅ Reconexão MQTT automática

## Pinagem
| Componente | Pino GPIO |
|-----------|-----------|
| DHT11     | GPIO 4    |
| Botão     | GPIO 14   |
| Display SSD1306 | I2C (SCL: GPIO 22, SDA: GPIO 21) |

## Configuração

### WiFi
```cpp
const char* ssid = "Li&Jandinho";
const char* password = "w26j21_12032022";
```

### MQTT
- **Servidor**: 192.168.2.30
- **Porta**: 1883
- **Usuário**: homeassistant
- **Senha**: @terra123
- **Tópicos**:
  - `home/quarto/temp` - Temperatura
  - `home/quarto/umid` - Umidade

## Dependências
```
- adafruit/DHT sensor library
- adafruit/Adafruit Unified Sensor
- adafruit/Adafruit SSD1306
- adafruit/Adafruit GFX Library
- knolleary/PubSubClient @ ^2.8
```

## Como Usar

### Compilar e Enviar
```bash
pio run --target upload
```

### Monitorar Serial
```bash
pio device monitor
```

### Configuração no Home Assistant
Adicione os sensores ao `configuration.yaml`:
```yaml
mqtt:
  sensor:
    - name: "Temperatura Quarto"
      state_topic: "home/quarto/temp"
      unit_of_measurement: "°C"
    
    - name: "Umidade Quarto"
      state_topic: "home/quarto/umid"
      unit_of_measurement: "%"
```

## Notas
- A leitura dos sensores é feita a cada 5 segundos
- O botão possui debounce simples de 200ms
- Em caso de falha na conexão WiFi, o sistema tenta reconectar automaticamente
- O display possui endereço I2C padrão: 0x3C

## Autor
Eng. Anderson Pedrosa

## Data
Maio de 2026
