#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h> // Baixe do repositório: https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA
#include <arduinoFFT.h> // Baixe do repositório: https://github.com/kosme/arduinoFFT

// ======== CONFIGURAÇÕES PRINCIPAIS ========
// Tamanho da matriz de LEDs
#define LARGURA_MATRIZ 64
#define ALTURA_MATRIZ  32
#define NUM_BANDAS     32
#define AMOSTRAS       1024 
#define FREQ_AMOSTRAGEM 40000
#define PINO_AUDIO      34

#define ESCALA        5000 // Funciona como um paramentro de sensibilidade, ajuste conforme testes

// HUB75 PINOUT
#define A    15
#define B    16
#define C    17
#define D    18
#define CLK  19
#define LAT  32
#define OE   33
#define R1   21
#define G1   22
#define B1   23
#define R2   27
#define G2   25
#define B2   26
#define PANEL_CHAIN 1

// ======== VARIÁVEIS GLOBAIS ========
double vReal[AMOSTRAS];
double vImag[AMOSTRAS];
int energiaBanda[NUM_BANDAS];
float topoSuave[NUM_BANDAS];
int alturaAnterior[NUM_BANDAS];

unsigned int tempo_entre_amostras;
MatrixPanel_I2S_DMA *matrix = nullptr;
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, AMOSTRAS, FREQ_AMOSTRAGEM);

// ======== UTILITÁRIOS DE COR ========
uint16_t corBanda(int banda, int altura) {
  uint16_t h = banda * (65535 / NUM_BANDAS);
  uint8_t s = 255;
  uint8_t v = 255;
  // Conversão HSV para RGB565
  float hf = h / 65535.0 * 6.0;
  int i = int(hf);
  float f = hf - i;
  float p = v * (1.0 - s / 255.0);
  float q = v * (1.0 - f * s / 255.0);
  float t = v * (1.0 - (1.0 - f) * s / 255.0);
  float r, g, b;
  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
  return matrix->color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

// ======== DESENHO ========
void desenhaBarra(int banda, int altura, uint16_t cor) {
  int larguraBarra = LARGURA_MATRIZ / NUM_BANDAS;
  int xInicio = larguraBarra * banda;

  // Apaga toda a coluna da banda antes de desenhar
  for (int x = xInicio; x < xInicio + larguraBarra; x++) {
    for (int y = 0; y < ALTURA_MATRIZ; y++) {
      matrix->drawPixel(x, y, 0);
    }
  }

  for (int x = xInicio; x < xInicio + larguraBarra; x++) {
    for (int y = ALTURA_MATRIZ - 1; y > ALTURA_MATRIZ - 1 - altura; y--) {
      if (x >= 0 && x < LARGURA_MATRIZ && y >= 0 && y < ALTURA_MATRIZ) {
        matrix->drawPixel(x, y, cor);
      }
    }
  }
}

void desenhaPico(int banda, int altura, uint16_t corPico) {
  static int picoAnterior[NUM_BANDAS] = {0};
  int larguraBarra = LARGURA_MATRIZ / NUM_BANDAS;
  int xInicio = larguraBarra * banda;


  int yPicoAnt = ALTURA_MATRIZ - 1 - picoAnterior[banda];
  if (yPicoAnt >= 0 && yPicoAnt < ALTURA_MATRIZ) {
    for (int x = xInicio; x < xInicio + larguraBarra; x++) {
      matrix->drawPixel(x, yPicoAnt, 0); 
    }
  }

 
  int yPico = ALTURA_MATRIZ - 1 - altura;
  if (yPico >= 0 && yPico < ALTURA_MATRIZ) {
    for (int x = xInicio; x < xInicio + larguraBarra; x++) {
      matrix->drawPixel(x, yPico, corPico);
    }
  }

  picoAnterior[banda] = altura;
}

// ======== SETUP ========
void setup() {
  HUB75_I2S_CFG mxconfig(
    LARGURA_MATRIZ,
    ALTURA_MATRIZ,
    PANEL_CHAIN
  );
  mxconfig.gpio.a   = A;
  mxconfig.gpio.b   = B;
  mxconfig.gpio.c   = C;
  mxconfig.gpio.d   = D;
  mxconfig.gpio.clk = CLK;
  mxconfig.gpio.lat = LAT;
  mxconfig.gpio.oe  = OE;
  mxconfig.gpio.r1  = R1;
  mxconfig.gpio.g1  = G1;
  mxconfig.gpio.b1  = B1;
  mxconfig.gpio.r2  = R2;
  mxconfig.gpio.g2  = G2;
  mxconfig.gpio.b2  = B2;

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(18);
  matrix->clearScreen();

  tempo_entre_amostras = round(1000000 * (1.0 / FREQ_AMOSTRAGEM));
  memset(topoSuave, 0, sizeof(topoSuave));
  memset(alturaAnterior, 0, sizeof(alturaAnterior));
}

// ======== LOOP PRINCIPAL ========
void loop() {
  memset(energiaBanda, 0, sizeof(energiaBanda));

  // Coleta de áudio
  for (int i = 0; i < AMOSTRAS; i++) {
    vReal[i] = analogRead(PINO_AUDIO);
    vImag[i] = 0;
    delayMicroseconds(tempo_entre_amostras);
  }

  // FFT
  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  
  int binIni = 9;
  int binFim = AMOSTRAS / 2;
  float logMin = log10(binIni);
  float logMax = log10(binFim);
  int limites[NUM_BANDAS + 1];
  for (int b = 0; b <= NUM_BANDAS; b++) {
    float frac = (float)b / NUM_BANDAS;
    int bin = round(pow(10, logMin + frac * (logMax - logMin)));
    if (bin < binIni) bin = binIni;
    if (bin > binFim) bin = binFim;
    limites[b] = bin;
  }

  for (int banda = 0; banda < NUM_BANDAS; banda++) {
    for (int i = limites[banda]; i < limites[banda + 1]; i++) { // Opcional, aumenta  os graves e atenua os agudos
      energiaBanda[banda] += (int)vReal[i];
    }
    if (banda < 6) energiaBanda[banda] *= 2;
    if (banda >= NUM_BANDAS - 6) energiaBanda[banda] /= 2;
  }


  for (int banda = 0; banda < NUM_BANDAS; banda++) {
    int altura = energiaBanda[banda] / ESCALA;
    if (altura > ALTURA_MATRIZ) altura = ALTURA_MATRIZ;
    altura = (alturaAnterior[banda] + altura) / 2;

  
    topoSuave[banda] = max(topoSuave[banda] * 0.92f, (float)altura);

    desenhaBarra(banda, altura, corBanda(banda, altura));
    desenhaPico(banda, (int)topoSuave[banda], matrix->color565(255, 255, 255));

    
    alturaAnterior[banda] = altura;
  }

  delay(5);
}
