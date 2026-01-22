#include <Adafruit_SSD1306.h>
#include <vector>
#include <LittleFS.h>

const int RED = 14;
const int GREEN = 13;
const int BLUE = 12;
const int BT1 = 7;
const int BT2 = 6;
const int BT3 = 5;
const int BT4 = 4;
const int BT5 = 3;
const int BT6 = 2;
const int LDR = 1;
const int BUZZER = 17;

String mensagem;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64);

struct Cor {
  int r, g, b;
};

Cor nota2cor(int nota);
void enviarRGB(Cor cor);
void enviarRGB(int red, int green, int blue);
int getRecorde();
void escreveRecorde(int pontos);

const int DO = 523;
const int RE = 587;
const int MI = 659;
const int FA = 698;
const int SOL = 784;
const int LA = 880;
/*
const int DO = 131;
const int RE = 147;
const int MI = 165;
const int FA = 175;
const int SOL = 196;
const int LA = 220;*/

const int NOTAS[] = {DO, RE, MI, FA};

std::vector<int> notas = {};
int pontos;
bool perdeu;

int recorde = 0;

void setup() {
  Serial.begin(115200);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BT1, INPUT_PULLUP);
  pinMode(BT2, INPUT_PULLUP);
  pinMode(BT3, INPUT_PULLUP);
  pinMode(BT4, INPUT_PULLUP);
  pinMode(BT5, INPUT_PULLUP);
  pinMode(BT6, INPUT_PULLUP);

  analogReadResolution(12);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE, BLACK);
  display.display();
  display.clearDisplay();

  if (!LittleFS.begin(true)) {
    Serial.println("Erro ao montar LittleFS");
    return;
  }
  Serial.println("Sucesso ao montar LittleFS");

  getRecorde();
}

void loop() {
  novoJogo();
  while (!perdeu) {
    mostra("Pontos: " + String(pontos));
    escreve("Recorde: " + String(recorde));
    adicionarNota();
    tocarMelodia(notas);
    perguntaJogador();
    delay(1000);
  }
}

void novoJogo() {
  int seed = ((micros() ^ analogRead(LDR)) * 37);
  randomSeed(seed);
  notas.clear();
  pontos = 0;
  perdeu = false;
  Serial.println("NOVO JOGO: " + String(notas.size()));
}

int escolherNota() {
  return NOTAS[random(4)];
}

void adicionarNota() {
  notas.push_back(escolherNota());
  Serial.println("nova nota: " + String(notas.size()) + ", " + String(notas.back()));
}

void tocarMelodia(std::vector<int> notas) {
  Serial.println("melodia: " + String(notas.size()));
  for (int nota : notas) {
    tocarNota(nota, 500);
  }
}

void tocarNota(int nota, int tempo) {
  enviarRGB(nota2cor(nota));
  tone(BUZZER, nota, tempo);
  delay(tempo * 1.3);
  noTone(BUZZER);
  enviarRGB(0, 0, 0);
}

int getNota() {
  int nota = 0;
  do {
    if (digitalRead(BT1) == LOW) {
      nota = DO;
    }
    else if (digitalRead(BT2) == LOW) {
      nota = RE;
    }
    else if (digitalRead(BT3) == LOW) {
      nota = MI;
    }
    else if (digitalRead(BT4) == LOW) {
      nota = FA;
    }
    else if (digitalRead(BT5) == LOW) {
      nota = SOL;
    }
    else if (digitalRead(BT6) == LOW) {
      nota = LA;
    }
  } while (nota == 0);
  Serial.println("nota usuario: " + String(nota));
  return nota;
}

void perguntaJogador() {
  int quantidade = 0;
  while (quantidade < notas.size()) {
    Serial.println("quantidade: " + String(quantidade) + ", " + String(notas.size()));
    int nota = getNota();

    tocarNota(nota, 200);

    Serial.println("diff : " + String(nota) + ", " + String(notas[quantidade]));
    if (nota != notas[quantidade]) {
      perde();
      return;
    }
    quantidade++;
  }
  ganha();
  pontos++;
}

void perde() {
  escreve("Voce perdeu!\n Tente outra vez");
  delay(500);
  tocarNota(LA, 1000);
  Serial.println("perdeu");
  perdeu = true;

  if (pontos > recorde) {
    escreveRecorde(pontos);
  }
}

void ganha() {
  escreve("Muito bem!");
  delay(500);
  tocarNota(SOL, 100);
  tocarNota(SOL, 100);
  tocarNota(SOL, 100);
  Serial.println("ganhou");
}

Cor nota2cor(int nota) {
  switch (nota) {
    case DO:
      return {255, 0, 0};
    case RE:
      return {0, 200, 0};
    case MI:
      return {0, 0, 255};
    case FA:
      return {155, 50, 0};
    default:
      return {100, 50, 100};
  }
}

void enviarRGB(Cor cor) {
  enviarRGB(cor.r, cor.g, cor.b);
}

void enviarRGB(int red, int green, int blue) {
  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);
}

int getRecorde() {
  File f = LittleFS.open("/recordes.txt", "r");
  if (!f) {
    recorde = 0;
    Serial.println("erro ao ler: " + String(recorde));
  }
  else {
    Serial.println("getRecorde linha: " + String(recorde));
    while (f.available()) {
      String linha = f.readStringUntil('\n');
      recorde = linha.toInt();
      Serial.println("getRecorde linha2: " + String(recorde));
    }
  }
  f.close();
  return recorde;
}

void escreveRecorde(int pontos) {
  recorde = pontos;
  File f = LittleFS.open("/recordes.txt", "w");
  if (!f) {
    Serial.println("erro ao escrever: " + String(recorde));
    return;
  }
  f.println(String(recorde));
  f.close();
  Serial.println("escreveu recorde: " + String(recorde));
}

void mostra(String mensagem) {
  display.clearDisplay();
  display.setCursor(0, 0);
  escreve(mensagem);
}
void escreve(String mensagem) {
  display.println(mensagem);
  display.display();
}
