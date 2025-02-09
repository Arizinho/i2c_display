# Display de LED

Este projeto implementa um sistema de controle de LEDs RGB, matriz de LEDs WS2812 e um display de LED utilizando a Raspberry Pi Pico. O sistema permite o acionamento dos LEDs por meio de botões, exibição de caracteres no display e representação de números na matriz de LEDs.

---

## **Configuração dos Pinos GPIO**

Os pinos GPIO da Raspberry Pi Pico estão configurados conforme a tabela abaixo:

| GPIO   | Componente     | Função                                                   |
| ------ | -------------- | -------------------------------------------------------- |
| **5**  | Botão A        | Entrada para alternar o estado do LED verde              |
| **6**  | Botão B        | Entrada ara alternar o estado do LED azul                |
| **7**  | Matriz de LEDs | Saída para controle da matriz WS2812                     |
| **11** | LED Verde      | Saída digital                                            |
| **12** | LED Azul       | Saída digital                                            |
| **14** | I2C SDA        | Linha de dados para comunicação I2C com o display de LED |
| **15** | I2C SCL        | Linha de clock para comunicação I2C com o display de LED |

---

## **Funcionamento do Código**

1. **Inicialização:**

   - Configura os pinos GPIO para os LEDs e botões.
   - Inicializa a comunicação I2C para o display OLED.
   - Configura a matriz de LEDs WS2812 via PIO.

2. **Controle dos LEDs:**

   - O **Botão A** alterna o estado do LED verde.
   - O **Botão B** alterna o estado do LED azul.
   - O estado dos LEDs é atualizado no display OLED e exibido via comunicação serial.

3. **Exibição no Display de LED:**

   - Caracteres digitados são exibidos no display.
   - Números digitados são exibidos também na matriz de LEDs WS2812.

4. **Tratamento de Interrupção:**

   - Os botões são configurados para gerar interrupção ao serem pressionados.
   - Debounce de 200 ms.

---

## **Link do Vídeo**

https://youtu.be/4Ubj6IM94Bw

