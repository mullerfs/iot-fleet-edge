# esp32-bme280

Projeto ESP32 com BME280, GPS e envio MQTT. Inclui scanner I2C automatico para detectar o endereco do BME280 e um padrao de blink do LED indicando o estado de conexao.

## Funcionalidades
- Leitura de temperatura, umidade, pressao e altitude estimada (BME280)
- Leitura de dados do GPS (TinyGPS++)
- Publicacao MQTT com payload JSON
- Scanner I2C no boot para detectar o BME280
- LED de status:
  - Wi-Fi conectado: 1 piscada por segundo
  - Wi-Fi + MQTT: 2 piscadas, pausa 1s, repete
  - Wi-Fi + MQTT + GPS fix: 3 piscadas, pausa 1s, repete

## Requisitos
- ESP32 (board: esp32dev)
- BME280 via I2C
- Modulo GPS (UART)

## Configuracao rapida
1. Ajuste pinos e parametros em `include/config.h`.
2. Configure suas credenciais em `include/secrets.h`.
3. Compile e envie pelo PlatformIO.

### Pinos padrao
Veja `include/config.h`:
- I2C: SDA 19, SCL 21
- GPS: RX 16, TX 17
- LED: GPIO 2

## Monitor Serial
Use 115200. No boot, o scanner I2C imprime os enderecos encontrados e o endereco usado pelo BME280.

## Licenca
Veja o arquivo `LICENSE`.
