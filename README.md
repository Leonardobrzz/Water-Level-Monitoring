# Water Level Monitoring

**Monitoramento de Níveis de Água com Sensor VL53L0X e Bitdoglab**  
*Base para futura expansão com app, dashboards e múltiplos sensores*

Este repositório contém a base de um sistema embarcado em C para a placa Bitdoglab (Raspberry Pi Pico) que:

- Lê distância em milímetros usando um **sensor VL53L0X** (I2C0 em GP8/GP9)  
- Converte distância em nível de água (N = H – distância)  
- Exibe medições em um **display OLED SSD1306** (I2C1 em GP14/GP15)  
- Aciona um **LED RGB** (GP13, GP11, GP12) e um **buzzer** (PWM em GP21) conforme faixas (ideal, alerta, crítico)  
- Executa leituras em loop a cada 500 ms  

> **Importante**: esta é a versão inicial. Nas próximas etapas integraremos comunicação com app, geração de dashboards e gráficos, além de suporte a novos sensores.

---

## 📋 Funcionalidades Atuais

- **Medição em tempo real** do nível de água  
- **Indicação visual e sonora** por LEDs e buzzer  
- **Código-fonte em C** organizado por módulos (sensor, display, LEDs, buzzer)  
- **Build com CMake** e Raspberry Pi Pico SDK  

---

## 🛠️ Hardware

| Componente            | Pino na Bitdoglab | Interface    |
|-----------------------|-------------------|--------------|
| VL53L0X (SDA, SCL)    | GP8, GP9          | I2C0         |
| OLED SSD1306 (SDA,SCL)| GP14, GP15        | I2C1         |
| LED Vermelho          | GP13              | GPIO         |
| LED Verde             | GP11              | GPIO         |
| LED Azul              | GP12              | GPIO         |
| Buzzer (sinal PWM)    | GP21              | PWM          |

---

## 🚀 Como Compilar e Enviar

1. **Pré-requisitos**  
   - Raspberry Pi Pico SDK  
   - Toolchain ARM GNU, CMake, Make  

2. **Clonar e navegar**  
   ```bash
   git clone https://github.com/Leonardobrzz/Water-Level-Monitoring.git
   cd Water-Level-Monitoring
