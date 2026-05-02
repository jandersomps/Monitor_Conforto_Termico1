# Monitor de Conforto Térmico

## Descrição
Projeto de monitoramento de temperatura e umidade usando ESP32, com display OLED, relógio sincronizado via NTP e integração com Home Assistant via MQTT. Inclui sistema de configuração via portal captivo (WiFiManager) para fácil configuração sem necessidade de reprogramação.

## Componentes Utilizados
- **Microcontrolador**: ESP32
- **Sensor**: DHT11 (temperatura e umidade)
- **Display**: OLED SSD1306 (128x64 pixels, I2C)
- **Botão 1**: GPIO 14 (alternar telas)
- **Botão 2**: GPIO 12 (resetar configurações)

## Funcionalidades
- ✅ **Portal Captivo (WiFiManager)**: Configuração de WiFi e servidor MQTT sem reprogramar
- ✅ **Relógio em Tempo Real**: Sincronização automática via NTP (fuso horário Brasília UTC-3)
- ✅ Leitura de temperatura e umidade a cada 5 segundos
- ✅ Display atualizado a cada 500ms (hora em tempo real sem delay)
- ✅ **Duas telas alternadas no display**:
  - **Tela 1**: Data completa, dia da semana, hora, temperatura e umidade
  - **Tela 2**: Status detalhado da rede (SSID, IP, sinal WiFi, status MQTT)
- ✅ Envio de dados para Home Assistant via MQTT
- ✅ Reconexão automática WiFi e MQTT
- ✅ Configurações salvas na memória flash (persistem após reiniciar)
- ✅ Sistema de reset para reconfiguração fácil

## Pinagem
| Componente | Pino GPIO | Função |
|-----------|-----------|--------|
| DHT11     | GPIO 4    | Sensor de temperatura/umidade |
| Botão Tela | GPIO 14  | Alternar entre telas |
| Botão Reset | GPIO 12 | Resetar configurações (segurar 3s) |
| Display SSD1306 | I2C | SCL: GPIO 22, SDA: GPIO 21 |

## Configuração Inicial

### Primeira Configuração (ou após reset)
1. **Ligue o ESP32** - O display mostrará "Iniciando..."
2. **O ESP32 cria um ponto de acesso**:
   - **Nome da rede**: `ESP32_ConfortoAP`
   - **Senha**: `12345678`
3. **Conecte seu celular ou computador** nesta rede WiFi
4. **O portal de configuração abre automaticamente** (ou acesse: `192.168.4.1`)
5. **Configure os parâmetros**:
   - Selecione sua rede WiFi
   - Digite a senha do WiFi
   - Digite o IP do servidor MQTT (Home Assistant)
6. **Clique em "Save"** e aguarde
7. O ESP32 reinicia e conecta automaticamente!

### Reconfigurar WiFi/MQTT
Para alterar as configurações sem reprogramar:
1. **Segure o botão de RESET** (GPIO 12) por **3 segundos**
2. O display mostra "RESETANDO..."
3. Todas as configurações são apagadas
4. O ESP32 reinicia em modo configuração
5. Repita o processo de configuração inicial

### Usar os Botões
- **Botão GPIO 14**: Pressione rapidamente para alternar entre as telas
- **Botão GPIO 12**: Segure por 3 segundos para resetar todas as configurações

## Layout das Telas

### Tela 1 - Principal
```
Seg 02/05/2026
10:30:45        <- Hora grande
---------------
Temp: 25.5°C
Umid: 65.0%
```

### Tela 2 - Status da Rede
```
STATUS REDE
---------------
SSID: MinhaRede
IP: 192.168.1.100
Sinal: -45 dBm
MQTT: OK
```

## MQTT - Integração Home Assistant

### Configuração Padrão
- **Porta**: 1883
- **Usuário**: homeassistant
- **Senha**: @terra123 (altere no código se necessário)
- **Tópicos publicados**:
  - `home/quarto/temp` - Temperatura em °C
  - `home/quarto/umid` - Umidade em %

### Configuração no Home Assistant
Adicione os sensores ao `configuration.yaml`:
```yaml
mqtt:
  sensor:
    - name: "Temperatura Quarto"
      state_topic: "home/quarto/temp"
      unit_of_measurement: "°C"
      device_class: "temperature"
    
    - name: "Umidade Quarto"
      state_topic: "home/quarto/umid"
      unit_of_measurement: "%"
      device_class: "humidity"
```

## Dependências
As seguintes bibliotecas são instaladas automaticamente pelo PlatformIO:
```ini
- adafruit/DHT sensor library         # Sensor DHT11/DHT22
- adafruit/Adafruit Unified Sensor    # Dependência dos sensores Adafruit
- adafruit/Adafruit SSD1306           # Display OLED
- adafruit/Adafruit GFX Library       # Gráficos para display
- knolleary/PubSubClient @ ^2.8       # Cliente MQTT
- arduino-libraries/NTPClient @ ^3.2.1 # Sincronização de hora via NTP
- tzapu/WiFiManager @ ^2.0.16-rc.2    # Portal captivo para configuração WiFi
```

## Como Usar

### 1. Compilar e Enviar pela primeira vez
```bash
# Compilar o projeto
pio run

# Compilar e enviar para o ESP32
pio run --target upload

# Monitorar a saída serial
pio device monitor
```

### 2. Primeiro Boot
- O display acende mostrando "Iniciando..."
- Aparece a tela de configuração WiFi
- Conecte-se ao AP `ESP32_ConfortoAP` (senha: `12345678`)
- Configure WiFi e servidor MQTT no portal
- Salve e aguarde a conexão

### 3. Operação Normal
- O display alterna automaticamente entre duas telas
- Pressione o botão GPIO 14 para trocar manualmente
- A hora sincroniza automaticamente via internet
- Dados são enviados para o Home Assistant a cada 5 segundos

### 4. Troubleshooting
- **Display não acende**: Verifique conexão I2C e endereço (0x3C)
- **Não conecta no WiFi**: Segure botão de reset por 3s e reconfigure
- **MQTT com erro**: Verifique IP do servidor na configuração
- **Hora incorreta**: Verifique conexão com internet (necessária para NTP)

## Especificações Técnicas

### Hardware
- **Microcontrolador**: ESP32 (dual-core, WiFi integrado)
- **Tensão de operação**: 3.3V (alimentação via USB 5V)
- **Sensor**: DHT11 (0-50°C, 20-80% umidade)
- **Display**: OLED monocromático 0.96" (protocolo I2C)

### Software
- **Framework**: Arduino (via PlatformIO)
- **Taxa de atualização**: Display 500ms | Sensor 5s | MQTT 5s
- **Fuso horário**: UTC-3 (Horário de Brasília)
- **Servidor NTP**: pool.ntp.org
- **Porta serial**: 115200 baud

## Notas Importantes
- ⚠️ **Primeira vez**: É necessário configurar WiFi e MQTT via portal captivo
- 📡 **Conexão internet**: Necessária para sincronização de hora (NTP)
- 💾 **Configurações persistentes**: Salvas na memória flash do ESP32
- 🔄 **Reset de fábrica**: Segure botão GPIO 12 por 3 segundos
- 📶 **Qualidade de sinal**: Exibida em dBm (-30 excelente, -90 fraco)
- 🕐 **Relógio**: Atualiza a cada 500ms (segundos em tempo real)
- 🌡️ **Sensor DHT11**: Leitura a cada 5s (recomendação do fabricante)
- 🔌 **Display I2C**: Endereço padrão 0x3C
- 📤 **Timeout portal**: 3 minutos sem configurar = reinicia

## Melhorias Futuras
- [ ] Adicionar sensor de luminosidade
- [ ] Criar terceira tela com histórico de temperatura
- [ ] Implementar modo sleep para economia de energia
- [ ] Adicionar notificações via Home Assistant
- [ ] Calibração dos sensores via interface web
- [ ] OTA (atualização via WiFi)

## Licença
Este projeto é de código aberto e pode ser usado livremente para fins educacionais e pessoais.

## Autor
**Eng. Anderson Pedrosa**  
📧 eng-jandersonpedrosa

## Histórico de Versões

### v2.0 - Maio 2026 (Versão Atual)
- ✨ Adicionado WiFiManager (portal captivo)
- ✨ Relógio com data, hora e dia da semana via NTP
- ✨ Display atualizado a cada 500ms (tempo real)
- ✨ Segundo botão dedicado para reset
- ✨ Configurações salvas em memória flash
- ✨ Tela de status melhorada (SSID, IP, sinal, MQTT)
- 🐛 Corrigido delay nos segundos do relógio
- 🐛 Corrigido ordem de inicialização do display

### v1.0 - Maio 2026
- 🎉 Versão inicial
- DHT11 + Display OLED + MQTT
- Duas telas alternadas
- Conexão WiFi básica

---

**Última atualização**: Maio de 2026
