//  Defniçao dos pinos do sensor de linha
#define LINE_F_FUNDO A1
#define LINE_F_CENTRO A2
#define LINE_F_FRENTE A3
#define LINE_F_ESQUERDA A4
#define LINE_F_DIREITA A5

//  Definiçao dos pinos dos encoders opticos das rodas
#define ENCODER_DIREITA 21
#define ENCODER_ESQUERDA 2

//  
#define ACELERADOR_DIREITA 5
#define ACELERADOR_ESQUERDA 6

//  Definição das entradas para os motores de cada lado, observem que alguns robôs precisaram ter os pinos trocados devido aos módulos de comunicação que utilizam.
//  Vejam os 4 fios unidos da ponte H que são conectados no Arduino para saberem o padrão do robô que vocês tem em mãos.
#define DIRECAO_DIREITA_1 8
#define DIRECAO_DIREITA_2 7

#define DIRECAO_ESQUERDA_1 10
#define DIRECAO_ESQUERDA_2 9

#define IR_PARA_FRENTE_DIREITA() do { digitalWrite(DIRECAO_DIREITA_1, HIGH); digitalWrite(DIRECAO_DIREITA_2, LOW); } while(false)
#define IR_PARA_FRENTE_ESQUERDA() do { digitalWrite(DIRECAO_ESQUERDA_1, HIGH); digitalWrite(DIRECAO_ESQUERDA_2, LOW); } while(false)
#define IR_PARA_FRENTE() do { IR_PARA_FRENTE_DIREITA(); IR_PARA_FRENTE_ESQUERDA(); } while(false)

#define IR_PARA_TRAS_DIREITA() do { digitalWrite(DIRECAO_DIREITA_1, LOW); digitalWrite(DIRECAO_DIREITA_2, HIGH); } while(false)
#define IR_PARA_TRAS_ESQUERDA() do { digitalWrite(DIRECAO_ESQUERDA_1, LOW); digitalWrite(DIRECAO_ESQUERDA_2, HIGH); } while(false)
#define IR_PARA_TRAS() do { IR_PARA_TRAS_DIREITA(); IR_PARA_TRAS_ESQUERDA(); } while(false)

#define ACELERA_DIREITA(VELOCIDADE) do { pwmDireita = VELOCIDADE; analogWrite(ACELERADOR_DIREITA, VELOCIDADE); } while(false)
#define ACELERA_ESQUERDA(VELOCIDADE) do { pwmEsquerda = VELOCIDADE; analogWrite(ACELERADOR_ESQUERDA, VELOCIDADE); } while(false)
#define ACELERA(VELOCIDADE) do { ACELERA_DIREITA(VELOCIDADE); ACELERA_ESQUERDA(VELOCIDADE); } while(false)

#define FREIO_DIREITA() { ACELERA_DIREITA(0); digitalWrite(DIRECAO_DIREITA_1, LOW); digitalWrite(DIRECAO_DIREITA_2, LOW); } while(false)
#define FREIO_ESQUERDA() { ACELERA_ESQUERDA(0); digitalWrite(DIRECAO_ESQUERDA_1, LOW); digitalWrite(DIRECAO_ESQUERDA_2, LOW); } while(false)
#define FREIO() do { FREIO_DIREITA(); FREIO_ESQUERDA(); } while(false)

// Velocidade armazenada PWM
volatile int pwmDireita = 0;
volatile int pwmEsquerda = 0;

// Numero de passos do Encoder Ótico
volatile int contador_direita = 0;
volatile int contador_esquerda = 0;

// Definir velocidades
int velocidadeDireita  = 180;
int velocidadeEsquerda = 180;

// Define variaveis que monitoram os sendores do seguidor de linha
boolean segLinDireita;
boolean segLinEsquerda;
boolean segLinFrente;
boolean segLinFundo;
boolean segLinCentro;

// Define valores que irao tracar o limiar entre linha e resto do ambiente
int preto = 800;
int pretoCentro = 300;

boolean semReferencia = true;

void setup() {
  // Configuração da Comunicação Serial
  Serial.begin(9600);
  
  //Configuraçao dos pinos do sensor de linha
  pinMode(LINE_F_FUNDO,INPUT);
  pinMode(LINE_F_CENTRO,INPUT);
  pinMode(LINE_F_FRENTE,INPUT);
  pinMode(LINE_F_ESQUERDA,INPUT);
  pinMode(LINE_F_DIREITA,INPUT);
  
  // Configuração dos pinos da Ponte H
  pinMode(DIRECAO_DIREITA_1, OUTPUT);
  pinMode(DIRECAO_DIREITA_2, OUTPUT);
  pinMode(DIRECAO_ESQUERDA_1, OUTPUT);
  pinMode(DIRECAO_ESQUERDA_2, OUTPUT);

  // Configuração dos pinos do Encoder Ótico
  pinMode(ENCODER_DIREITA, INPUT_PULLUP);
  pinMode(ENCODER_ESQUERDA, INPUT_PULLUP);
  
  // Funções de Interrupção de cada um dos Encoders
  attachInterrupt(ENCODER_DIREITA, contadorDireita, CHANGE);
  attachInterrupt(ENCODER_ESQUERDA, contadorEsquerda, CHANGE);
  
  calibraVelocidadeRodas();
}

void loop() {
  ACELERA_DIREITA(velocidadeDireita);
  ACELERA_ESQUERDA(velocidadeEsquerda);
  
  /*IR_PARA_FRENTE();
  
  delay(5000);
  
  FREIO();
  delay(1000);*/
  segLinDireita = (analogRead(LINE_F_DIREITA) > preto);
  segLinEsquerda = (analogRead(LINE_F_ESQUERDA) > preto);
  segLinFrente = (analogRead(LINE_F_FRENTE) > preto);
  segLinFundo = (analogRead(LINE_F_FUNDO) > preto);
  segLinCentro = (analogRead(LINE_F_CENTRO) > pretoCentro);
  
  segueLinha1();
  
  /*followLine();
  delay(200);
  FREIO();
  delay(200);
  Serial.print("Direita: ");
  Serial.println(segLinDireita);
  Serial.println(analogRead(LINE_F_DIREITA));
  Serial.print("Esquerda: ");
  Serial.println(segLinEsquerda);
  Serial.println(analogRead(LINE_F_ESQUERDA));
  Serial.print("Frente: ");
  Serial.println(segLinFrente);
  Serial.println(analogRead(LINE_F_FRENTE));
  Serial.print("Dundo: ");
  Serial.println(segLinFundo);
  Serial.println(analogRead(LINE_F_FUNDO));
  Serial.println("-----------");
  delay(1000);*/
}

void segueLinha1(){
  // Caso 1 - esta em cima da linha:
  // Eh preciso fazer de tudo para continuar na linha
  if(segLinCentro){
    if(segLinFrente && segLinFundo){
      IR_PARA_FRENTE();
    }
    else if(!segLinFrente || !segLinFundo){
      if(segLinEsquerda){
        giraDireita();
      }
      else if(segLinDireita){
        giraEsquerda();
      }
      else{
        if(verificaEsquerda()){
          IR_PARA_FRENTE();
        }
        else if(verificaDireita()){
          IR_PARA_FRENTE();
        }
        else if(giraPraEncontrar()){
          IR_PARA_FRENTE();
        }
        else{
          // Eh um pedaço de linha pequeno. Fugir do pedaço e andar reto ateh encontrar algo.
        }
      }
    }
  }
  else {
    IR_PARA_TRAS();
  }
}

void giraDireita(){
  IR_PARA_TRAS_ESQUERDA();
  IR_PARA_FRENTE_DIREITA();
}

void giraEsquerda(){
  IR_PARA_TRAS_DIREITA();
  IR_PARA_FRENTE_ESQUERDA();
}

boolean verificaDireita(){
  // Girar para a direita ateh encontrar a linha
  for(int i = 0; i < 5; i++){
    giraDireita();
    delay(100);
    FREIO():
    if(segLinFrente){
      return true;
    }
  }
  else {
    retornaEsquerda();
  }
}

boolean verificaEsquerda(){
  //TODO: Girar para a esquerda ateh encontrar a linha
}

boolean giraPraEncontrar(){
  //TODO: Girar para um lado soh, por um determinado tempo, ateh encontar a linha;
}


void calibraVelocidadeRodas(){
  //TODO
}

void contadorDireita(){
  contador_direita++;
}

void contadorEsquerda(){
  contador_esquerda++;
}
