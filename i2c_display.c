#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/timer.h"

//macros LEDs RGB e botões
#define BUTTON_A 5 
#define BUTTON_B 6
#define BLUE_LED 12
#define GREE_LED 11

//macros matriz de LEDs
#define WS2812_PIN 7
#define NUM_PIXELS 25

//macros display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

//flags para indicar se LEDs estão acesos
volatile bool led_verde_on = 0;
volatile bool led_azul_on = 0;

//flag para indicar se um número está sendo mostrado na matriz de LEDS
bool numero_on = 0;

//variável para guardar tempo da ultima chamada efetiva de interrupção (para debounce)
volatile uint32_t last_time = 0;

ssd1306_t ssd; // Inicializa a estrutura do display

//buffer para desenhar números na matriz de LEDs
bool buffer_leds[10][25] = {
  {0, 1, 1, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0},

  {0, 0, 0, 1, 0,
  0, 1, 0, 0, 0,
  0, 0, 0, 1, 0,
  0, 1, 0, 0, 0,
  0, 0, 0, 1, 0},

  {0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 1, 1, 1, 0,
  0, 0, 0, 1, 0,
  0, 1, 1, 1, 0},

  {0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 1, 1, 1, 0},

  {0, 1, 0, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 0, 0, 1, 0},

  {0, 1, 1, 1, 0,
  0, 0, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 1, 1, 1, 0},

  {0, 1, 1, 1, 0,
  0, 0, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0},

  {0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 0, 0, 1, 0,
  0, 1, 0, 0, 0,
  0, 0, 0, 1, 0},

  {0, 1, 1, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0},

  {0, 1, 1, 1, 0,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 0, 0,
  0, 1, 1, 1, 0}
};

//protótipos das funções
void gpio_irq_handler(uint gpio, uint32_t events);
void desenha_numero(double r, double g, double b, uint8_t numero);
uint32_t matrix_rgb(double b, double r, double g);

int main()
{
  //variável para guardar tecla pressionada
  char tecla;

  //inicializa comunicação serial
  stdio_init_all();

  //define PIO e state machine utilizados
  PIO pio = pio0;
  int sm = 0;

  //coloca a frequência de clock para 128 MHz
  set_sys_clock_khz(128000, false);

  //configura PIO
  uint offset = pio_add_program(pio, &ws2812_program);
  ws2812_program_init(pio, sm, offset, WS2812_PIN);
    
  //inicializa com matriz de LEDs apagada
  desenha_numero(0.00, 0.00, 0.00, 0);

  //inicializa botões A e B
  gpio_init(BUTTON_A);
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_A);
  gpio_pull_up(BUTTON_B);

  //inicializa leds verde e azul
  gpio_init(GREE_LED);
  gpio_init(BLUE_LED);
  gpio_set_dir(GREE_LED, GPIO_OUT);
  gpio_set_dir(BLUE_LED, GPIO_OUT);
  gpio_put(GREE_LED, 0);
  gpio_put(BLUE_LED, 0);

  //habilita interrupções por borda de descida nos botões
  gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  while (true)
  {
    //ler tecla pressionada
    tecla = getchar();

    //caso tecla pressionada seja uma letra
    if ((tecla >= 'A' && tecla <= 'Z') || (tecla >= 'a' && tecla <= 'z')){
      // Atualiza o conteúdo do display com a tecla pressionada
      ssd1306_fill(&ssd, false); // Limpa o display
      ssd1306_draw_char(&ssd, tecla, 8, 10); // Desenha caractere
      ssd1306_send_data(&ssd); // Atualiza o display

      //apaga número da matriz se houver
      if (numero_on){
        numero_on = 0;
        desenha_numero(0.00, 0.00, 0.00, 0);
      }
    }
    //caso tecla pressionada seja um número
    else if (tecla >= '0' && tecla <= '9') {
      //desenha número pressionada na matriz de LEDs
      desenha_numero(0.01, 0.01, 0.01, (tecla - '0'));
      
      //indica que um número é mostrado na matriz
      numero_on = 1;

      // Atualiza o conteúdo do display com a tecla pressionada
      ssd1306_fill(&ssd, false); // Limpa o display
      ssd1306_draw_char(&ssd, tecla, 8, 10); // Desenha caractere
      ssd1306_send_data(&ssd); // Atualiza o display
    }
  }
}

//função para tratar interrupção
void gpio_irq_handler(uint gpio, uint32_t events){

  //lê tempo absoluto atual
  uint32_t current_time = to_ms_since_boot(get_absolute_time());

  //compara diferença entre tempo atual e última interrupção efetiva (debounce por delay de 0,2 s) 
  if(current_time-last_time > 200){
      //guarda tempo atual
      last_time = current_time;

      //interrupção por botão A -> comuta estado do led verde
      if (gpio == BUTTON_A){
        led_verde_on = !led_verde_on;
        gpio_put(GREE_LED, led_verde_on);

        //imprime novo estado do LED no display 
        //estado: ON
        if (led_verde_on){
          ssd1306_fill(&ssd, false); // Limpa o display
          ssd1306_draw_string(&ssd, "LED verde ON", 8, 10); // Desenha uma string
          ssd1306_send_data(&ssd); // Atualiza o display
          //exibe mensagem da operação via comunicação serial no computador
          printf("Botão A foi pressionado. O estado do LED verde foi comutado de OFF para ON.\n");
        }
        //estado: OFF
        else {
          ssd1306_fill(&ssd, false); // Limpa o display
          ssd1306_draw_string(&ssd, "LED verde OFF", 8, 10); // Desenha uma string
          ssd1306_send_data(&ssd); // Atualiza o display
          //exibe mensagem da operação via comunicação serial no computador
          printf("Botão A foi pressionado. O estado do LED verde foi comutado de ON para OFF.\n");
        }
        
        
      }
      //interrupção por botão B -> comuta estado do led azul
      else if (gpio == BUTTON_B){
        led_azul_on = !led_azul_on;
        gpio_put(BLUE_LED, led_azul_on);

        //imprime novo estado do LED no display 
        //estado: ON
        if (led_azul_on){
          ssd1306_fill(&ssd, false); // Limpa o display
          ssd1306_draw_string(&ssd, "LED azul ON", 8, 10); // Desenha uma string
          ssd1306_send_data(&ssd); // Atualiza o display
          //exibe mensagem da operação via comunicação serial no computador
          printf("Botão B foi pressionado. O estado do LED azul foi comutado de OFF para ON.\n");
        }
        //estado: OFF
        else {
          ssd1306_fill(&ssd, false); // Limpa o display
          ssd1306_draw_string(&ssd, "LED azul OFF", 8, 10); // Desenha uma string
          ssd1306_send_data(&ssd); // Atualiza o display
          //exibe mensagem da operação via comunicação serial no computador
          printf("Botão B foi pressionado. O estado do LED azul foi comutado de ON para OFF.\n");
        }
      }
  }
  
}

//função para transformar intensidade de cores de 0.0~1.0 (blue, red, green) em binários para envio para a ws2812
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

//função para desenhar números do buffer a partir das intensidades RGB em double (0.0~1.0) e indicação do número a ser desenhado em uint8_t (0 a 9)
void desenha_numero(double r, double g, double b, uint8_t numero){
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        //se 1 no buffer liga LED com intensidade especificada
        if (buffer_leds[numero][24-i] == 1){
            pio_sm_put_blocking(pio0, 0, matrix_rgb(b, r, g));
        }
        //se 0 no buffer não liga LED
        else{
            pio_sm_put_blocking(pio0, 0, matrix_rgb(0, 0, 0));
        }
    }
}

