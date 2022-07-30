/*********************************************************************** 
 | Sistema de Irrigação Automática com Banco de Dados e Comunicação GSM|
 | Versão 0.29                                                         |
 | Johann Herrero Cavadas                                              |
 | Marcos Vinícius Silva de Souza                                      |
 | Matheus Rodrigues Salomão                                           |
 | Rodrigo Gernohovski                                                 |
 | Vinícius Pereira                                                    |
 |                                                                     |
 |                                                                     |
 | Ambiente de Desenvolvimento Integrado(IDE): Arduino IDE V1.8.5      |
 | Plataforma: Arduino Mega - Microcontrolador ATMega 2560             |
 **********************************************************************/

/*-------------------------Instruções--------------------------*/
/* Pinagem utilizada (Númeração do arduino)
 * pino 2 = Botão de reset 
 * pino 3 = Sinal do circuito de chaveamento
 * pino 4 = Atuador central
 * pino 5 = Reset do RTC
 * pino 6 = Data do RTC
 * pino 7 = Clock do RTC
 * pino 8 = Atuador do módulo de expansão 1
 * pino 9 = Atuador do módulo de expansão 2
 * pino 10 = Atuador do módulo de expansão 3
 * pino 11 = Atuador do módulo de expansão 4
 * pino 12 = Atuador do módulo de expansão 5
 * pino 13 = Atuador do módulo de expansão 6
 * pino 16 = RX Sim 800l (TX Arduino)
 * pino 17 = TX Sim 800l(RX Arduino)
 * pino 20 = SDA LCD I2C
 * pino 21 = SCL LCD I2C
 * pino A0 = Divisor de tensão da bateria (Medidor de porcentagem)
 * pino A1 = Botões direcionais e enter
 * pino A2 = Sensor de umidade do ar e temperatura (DHT11)
 * pino A3 = Sensor de umidade do solo do módulo central (Higrômetro)
 * pino A4 = Sensor de umidade do solo do módulo 1 (Higrômetro)
 * pino A5 = Sensor de umidade do solo do módulo 2 (Higrômetro)
 * pino A6 = Sensor de umidade do solo do módulo 3 (Higrômetro)
 * pino A7 = Sensor de umidade do solo do módulo 4 (Higrômetro)
 * pino A8 = Sensor de umidade do solo do módulo 5 (Higrômetro)
 * pino A9 = Sensor de umidade do solo do módulo 6 (Higrômetro)
 */
 
/*-------------------Inclusão de bibliotecas-------------------*/
#include <LiquidCrystal_I2C.h> //Display
#include <Wire.h>//Protocolo I2C
#include <EEPROM.h>//Manipulação da EEPROM
#include <DHT.h>//DHT11
#include <Adafruit_Sensor.h>//Sensor de umidade do solo
#include <stdio.h>//Inclusão de instruções da linguagem C
#include <DS1302.h>//RTC
/*----------------Configuração dos componentes-----------------*/
//Para alterações na pinagem dos componentes e de parâmetros do programa, altere os dados marcados como XX no exemplo abaixo:
//#define exemplo XX

/*-----Definição da pinagem:-----*/

//Pinos analógicos:
#define bateria A0 //Pino que receberá a tensão da bateria, para verificar a porcentagem
#define botao A1 //Pino dos botões
#define DHT_Modulocentral A2 //Pino do sensor de umidade e temperatura do ar (DHT11)
 

//pinos digitais:
#define resetar 2 //Pino do botão de reset
#define controle_bateria 3//Pino que verificará se ocorreu a queda de energia
#define Clock_RTC 7//Pino de clock do RTC
#define Data_RTC 6//Pino de data do RTC
#define Rst_RTC 5//Pino de reset do RTC


/*-----Definição de parâmetros:-----*/
#define LCD_endereco 0x27//Endereço do LCD I2C
#define modlimite 6 //altere o valor para mudar o limite de módulos
#define DHTTYPE DHT11
#define Energia_Media 5//Tensão da bateria, considerada média
#define Energia_Baixa 4//Tensão da bateria, considerada baixa
#define Energia_Critica 3//Tensão da bateria, considerada crítica
#define Qplantas 10//Quantidade de hortaliças que terão num canteiro. // No caso de policultura, considera-se uma divisão igualitária.
#define EficienciaRega 0.75//Para irrigação por aspersão, o valor é de 75¢, ou 0,75
#define vazaoDaBomba 4 //a bomba bombeia 4 litros por minuto

/*-----Criação de objetos do programa:-----*/

LiquidCrystal_I2C lcd(LCD_endereco,20,4);
DHT dht(DHT_Modulocentral, DHTTYPE);
DS1302 rtc(Rst_RTC, Data_RTC, Clock_RTC);

/*----------------Variáveis gerais-----------------*/
bool resetou; //Variável de reset, para retornar à configuração
bool modulo_ativado[modlimite]; // Variável que verifica se o módulo foi conectado
bool mono, poli;// Variáveis de controle para monocultura e policultura;
bool Nivel_Critico = 0; // Variável que controla se a energia está em nível crítico
String telefone;// Valor do telefone


struct dados
{
bool cult;// tipo de cultura
byte prant1;// índice da primeira planta
byte prant2;// índice da segunda planta
byte Etapa;// índice da etapa de crescimento da planta
String Hr;// Horas
String Min;//Minutos
short Qagua_planta1;//Quantidade de água que a hortaliça1 precisa.
short Qagua_planta2;//Quantidade de água que a hortaliça2(se houver) precisa. Se for monocultura, será 0;
short Qagua_canteiro;//Quantidade de água que o canteiro necessitará.(Qagua_planta*Qplantas ou Qagua_planta*Qplantas/2 + Qagua_planta*Qplantas/2)
//short aguamin; 
//short aguamax;
};

struct dados modulo[modlimite];//Inicialização de um array de structs do tipo dados(com as variáveis declaradas acima).


struct banco_de_dados
{
 //String nome_planta; // é o nome da planta. O uso no programa é algo meio opcional
 short Qagua_Brotinho; //Quantidade de água que a planta consome no estágio broto
 short Qagua_Medio; //Quantidade de água que a planta consome no estágio mediano
 short Qagua_Crescida; //Quantidade de água que a planta consome no estágio final
 };
 
struct banco_de_dados alface;
struct banco_de_dados brocolis;
struct banco_de_dados tomate;
//struct banco_de_dados planta n;
// copie o código acima, substituindo " planta n" por uma planta


/*----------------Variáveis: Pinos-----------------*/
byte modulo_pino[modlimite];// Array que definirá os pinos dos módulos. Módulo_pino também é o pino do atuador.
char Sensor_Solo[modlimite];// Array que definirá os pinos sensores de umidade dos módulos.

/*----------------Variáveis: Configuração-----------------*/

bool mod[modlimite];//Array que receberá o estado dos pinos dos atuadores. Usado em algumas etapas de configuração.
bool cultura = 0; //Tipo de cultura por canteiro. pode ser monocultura(uma planta) ou policultura(duas plantas).
bool configuracao; //Variável para verificar se a configuração está ou não ativada.
bool reutilizar = 0;//Se a pessoa quiser reutilizar os dados existentes.
bool pisca_pisca = 0; //Variável para configurar a piscada do cursor numa única vez. Não é a melhor maneira, mas fazer o quê ¯\_(?)_/¯

byte guia = 0; // Variável usada  para controle da matriz de dígitos
byte statuses = 0; //Variável para controle do estado da configuração
byte modular;//Variável para controle dos módulos
byte maxhora; //Variável para controle do número máximo a ser digitado na hora. parâmetro da função butaun
byte d[11]; // Digitos. Usado na etapa de reutilização para armazenar etapas e dados temporários.
byte planta1 = 0; // Índice da primeira planta
byte planta2 = 0; // Índice da segunda planta
byte Etapa_planta = 0; //Índice da etapa de crescimento da planta

String Hora; // Valor das horas
String Minutos; //Valor dos minutos

/*----------------Variáveis: Reutilizar dados-----------------*/
//As variáveis abaixo indicam se determinado dado está sendo reutilizado.
//0(ou false): Não será reutilizado
//1(ou true): Será reutilizado
bool reutiliza_telefone;
bool reutiliza_cultura[modlimite];
bool reutiliza_planta1[modlimite];
bool reutiliza_planta2[modlimite];
bool reutiliza_Estagio[modlimite];
bool reutiliza_Hora[modlimite];
bool reutiliza_Minuto[modlimite];
/*----------------Variáveis: Botões-----------------*/
bool controleBotao = true; // Trava do botão
bool retorno = 0; // Botão de retorno
bool avanco = 0; // Botão de avanço
bool adicionar = 0; // Botão de incremento
bool subtrair = 0; // Botão de decremento
bool enter = 0; // Botão de confirmar
bool reset = false; //variável que governa a ação de "resetar". Também é o botão de cancelar
bool confirmar = false; //Confirmação dupla

//String Serial_Comando; // teste

/*----------------Variáveis: GSM-----------------*/
String SerialGSM; //Armazena a resposta do módulo GSM.
byte porcentagem; //Armazenará a porcentagem da bateria, utilizada na notificação de bateria baixa. 
byte Estado_SMS = 0;// Variável que controla qual SMS deve ser enviado ao telefone.
bool trava_SMS[8]; //trava para só enviar o SMS uma vez a cada evento.
/* Observação: O array acima é para facilitar a codificação, pois cada evento terá uma trava única.
 * Assim, por exemplo, caso a trava seja aplicada no evento que enviará a mensagem quando ocorrer 
 * a irrigação do módulo 1 e o módulo 2 tiver começado a irrigar um pouco depois do módulo 1, a
 * trava para evitar que o SMS de irrigação do módulo 2 seja enviado mais de uma vez não influenciará
 * na trava para evitar que o SMS de conclusão de irrigação do módulo 1 seja enviado mais de uma vez, 
 * e vice versa.
 */

/*----------------Variáveis: RTC-----------------*/
//Time t = rtc.time();
String minuto_master;
String hora_master;

/*----------------Variáveis: Sensores-----------------*/
bool perigo1; //Varíavel que indicará se a temperatura estará alta
bool perigo2[modlimite]; //Variável que determinará se o solo já está úmido. Para cada módulo
bool perigoTotal; //Varíavel que acionará se algum dos casos acima ocorrer

float umid;
float temp;
short Umidade_Limiarmin = 400;// Seria o Umimin
short Umidade_Limiarmax = 800;// Seria o Umimax


/*----------------Variáveis: Irrigar-----------------*/
bool irriga[modlimite]; //É a variável que verifica se está irrigando.
float Leitura_Solo[modlimite]; // Recebe a leitura dos sensores de umidade do solo

bool primavera, verao, outono, inverno; //estações do ano

float Tmed; //temperatura média de cada mês
byte NDP; //número de dias no mês
float Qo; //irradiação solar extraterrestre de acordo com o mês
float ETP; //parte da fórmula da quantidade de água
byte Qb; //quantidade de brotos
float tempo_irrigando1, tempo_irrigando2; //tempo que a bomba de irrigação fica ligada

bool alfaceM, brocolisM, tomateM;//variáveis que determinarão qual planta foi selecionada (policultura) (para fins de fórmula de água)
bool brotoM, crescendoM, crescidaM;//variáveis que determinarão o estágio de crescimento da planta (para fins de fórmula de água)

bool alfaceP, brocolisP, tomateP;//variáveis que determinarão qual planta foi selecionada (policultura) (para fins de fórmula de água)
bool brotoP, crescendoP, crescidaP;//variáveis que determinarão o estágio de crescimento da planta (para fins de fórmula de água)


//Abaixo para teste
String Tele = "11991061932";
String Hr_Teste = "21";
String Min_Teste = "33";
bool cultur = 1;
byte p1 = 1;
byte p2 = 2;














 /*--------------------Execução das funções---------------------*/
void setup()
{  
 Serial.begin(9600); //Monitor serial, para fins de teste.
 Serial2.begin(9600);//Serial do módulo GSM
 dht.begin();//Inicialização do DHT11.
 lcd.init();//Inicialização do LCD.
 lcd.backlight();//Habilitar a luz de fundo do LCD.
 
lcd.setCursor(0,0);
 lcd.print("Iniciando Interface");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");
 
 /* Configuração da interface. */
 pinMode(botao, INPUT);//Botões direcionais e enter.
 pinMode(resetar, INPUT_PULLUP);//Botão de reset.
 pinMode(controle_bateria, INPUT);//Pino de sinal do chaveamento.

 /* Fim da configuração da interface */

 delay(500);
 lcd.setCursor(0,1);
 lcd.print("Iniciando GSM");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");
 
 /* Configuração do sistema GSM. */
 Serial2.write("AT+CMGF=1\r\n");//Configurar o GSM para mensagens de texto.
 //Serial.println("Seleção do modo de mensagens de texto:");
 Serial2.write("AT+CNMI=2,2,0,0,0");//Verificar as mensagens
 //Serial.println("Verificação de mensagens:");
 //pinMode(DTR, OUTPUT);//Conferir isso depois 
 trava_SMS[0] = 0;
 trava_SMS[1] = 0;
 trava_SMS[2] = 0;
 trava_SMS[3] = 0;
 trava_SMS[4] = 0;
 trava_SMS[5] = 0;
 trava_SMS[6] = 0;
 trava_SMS[7] = 0;
 
 /* Fim da configuração do sistema GSM */
 
 delay(500);
 lcd.setCursor(0,2);
 lcd.print("Iniciando RTC");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");
 
/* Configuração do RTC. */
  rtc.writeProtect(false);
  rtc.halt(false);

 //Time t(2021, 12, 03, 11, 26, 00, Time::kTuesday);
 //rtc.time(t);  
 
 //Sugestão de aprimoramento: Permitir alterar o horário via IHM
 /* Fim da configuração do RTC */

 delay(500);
 lcd.setCursor(0,3);
 
 lcd.print("Iniciando Sensores");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");
 
  /* Configuração dos sensores. */
  //Sugestão de Aprimoramento: Otimizar a atribuição da pinagem
  Sensor_Solo[0] = 'A3';
  Sensor_Solo[1] = 'A4';
  Sensor_Solo[2] = 'A5';
  Sensor_Solo[3] = 'A6';
  Sensor_Solo[4] = 'A7';
  Sensor_Solo[5] = 'A8';
  Sensor_Solo[6] = 'A9';
  pinMode(bateria, INPUT);
  pinMode(DHT_Modulocentral, INPUT);
  pinMode(Sensor_Solo[0], INPUT);

/* Fim da configuração dos sensores */

 delay(500);
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Iniciando Banco de");
 lcd.setCursor(0,1);
 lcd.print("dados");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");

 delay(500);
 lcd.clear();
 lcd.setCursor(0,2);
 lcd.print("Iniciando Modulos");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");

/* Configuração dos módulos extras. */

 //Sugestão de Aprimoramento: Otimizar a atribuição da pinagem
  modulo_pino[0] = 4;  // pino do primeiro módulo (Módulo central)
  modulo_pino[1] = 8; //pino do segundo módulo   
  modulo_pino[2] = 9; //pino do segundo módulo   
  modulo_pino[3] = 10; //pino do segundo módulo   
  modulo_pino[4] = 11; //pino do segundo módulo   
  modulo_pino[5] = 12; //pino do segundo módulo   
  modulo_pino[6] = 13; //pino do segundo módulo 
  modulo_ativado[0] = 1;
  irriga[0] = 0;
  pinMode(modulo_pino[0], OUTPUT);
  
  for(modular = 1; modular<modlimite; modular++) 
              {  
               pinMode(modulo_pino[modular], INPUT_PULLUP); // Pino de conexão do módulo. também é o pino do atuador
               pinMode(Sensor_Solo[modular], INPUT);// Pino do sensor de umidade do solo.
               irriga[modular] = 0; //Variável que define quando pode irrigar.
               modulo_ativado[modular] = 0;//Variável que define quando o módulo está conectado.
               reutiliza_cultura[modular] = 0;
               reutiliza_planta1[modular] = 0;
               reutiliza_planta2[modular] = 0;
               reutiliza_Hora[modular] = 0;
               reutiliza_Minuto[modular] = 0;
              // Serial.print(modular);
              }
   modular = 0;

 /* Fim da configuração dos modulos extras */

 /* Outras configurações. */
   for(guia = 0; guia <= 10; guia++)
     {d[guia] = 0;}
     guia = 0;
     statuses = 0;

   // cultura = 0;
   // planta1 = 0;
   // planta2 = 0;
   Hora = "";
   Minutos = "";

  /* Fim das outras configurações */
 lcd.setCursor(0,1);
 lcd.print("Concluido!");
 delay(500);
 lcd.clear();

  lcd.setCursor(0,1);
  lcd.print("Modo: configuracao:");
  configuracao = 1;
  delay(1000);
  lcd.clear();
 
}

void loop()
{ 
lcd.clear();

  while(configuracao == 1)
 {
    if(reutilizar == 1)
    { 
     guia = d[0] + 1;  
     lcd.setCursor(0, 0);
     lcd.print("deseja reutilizar: ");
    
     switch(d[0])
     {

          case 0:
           lcd.setCursor(0, 1);
           lcd.print("Telefone?");
           lcd.setCursor(0, 2);
           //lcd.print(telefone);//Teste
           //Descomentar abaixo para uso da EEPROM
           //lcd.print("(");
           //lcd.print(EEPROM.read(1));
           //lcd.print(EEPROM.read(2));
           //lcd.print(")");
           //lcd.print(EEPROM.read(3));
           //lcd.print(EEPROM.read(4));
           //lcd.print(EEPROM.read(5));
           //lcd.print(EEPROM.read(6));
           //lcd.print(EEPROM.read(7));
           //lcd.print("-");
           //lcd.print(EEPROM.read(8));
           //lcd.print(EEPROM.read(9));
           //lcd.print(EEPROM.read(10));
           //lcd.print(EEPROM.read(11));
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados
        
          break;
     
          case 1:
           lcd.setCursor(0, 1);
           lcd.print("modulo ");
           lcd.print(modular);
           lcd.print("?  ");
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados

          break;
     
          case 2:
           lcd.setCursor(0, 1);
           lcd.print("Cultura?  ");
           lcd.setCursor(0, 2);
           if(/*EEPROM.read(12 +(5*modular)) == 0*/ cultur == 0)
           {lcd.print("Monocultura");}
           else
           {lcd.print("Policultura");}
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados
                 
          break;

          case 3:
           lcd.setCursor(0, 1);
           lcd.print("Hortaliça 1? ");
           lcd.setCursor(0, 2);
           switch(/*EEPROM.read(13 +(5*modular))*/ p1)
           {
            case 0:
            lcd.print("Alface Americana");
            break;

            case 1:
            lcd.print("Brocolis         ");
            break;

            case 2:
            lcd.print("Tomate          ");//mudar depois
            break;
           }
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados
          
          break;

          case 4:
           lcd.setCursor(0, 1);
           lcd.print("Hortaliça 2? ");
           lcd.setCursor(0, 2);
           switch(/*EEPROM.read(14+(5*modular))*/ p2)
           {
            case 0:
            lcd.print("Alface Americana");
            break;

            case 1:
            lcd.print("Brocolis        ");
            break;

            case 2:
            lcd.print("Tomate          ");
            break;
           }
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados
     
          break;
// inserir a recuperação da fase da planta
          case 5:
           lcd.setCursor(0, 1);
           lcd.print("Horas?       ");
           lcd.setCursor(0, 2);
           lcd.print(/*EEPROM.read(Num_Hr+(5*modular))*/Hr_Teste);
           
           butaun(0,6); // progresso 6 será a configuração de reutilização dos dados
     
          break;

          case 6:
           lcd.setCursor(0, 1);
           lcd.print("Minutos?");
           lcd.setCursor(0, 2);
           lcd.print(/*EEPROM.read(Num_Min+(5*modular))*/Min_Teste);
           
           butaun(2,6); // progresso 6 será a configuração de reutilização dos dados
     
          break;

          case 7:
           
           if(modular <= modlimite)
            {
              modular++; // Passa pro próximo módulo
              mod[modular] = digitalRead(modulo_pino[modular]); // Lê o próximo módulo (se está conectado)
              if(mod[modular] == 0 && modulo_ativado[modular] == 1)// Se estiver conectado e tiver sido ativado
                 {
                   d[0] = 2;
                 } else {
                    modular = modlimite+1; // apenas para pular para o else abaixo, e entrar no loop da configuração
                   }
             }else  {
              modular = 0;
              lcd.clear();
              for(guia = 0; guia <= 10; guia++)
                        {d[guia] = 0;}
                        guia = 0;
                  while(configuracao == 1)//Enquanto estiver no modo de configuração, o programa vai ficar nesse loop
                       { 
                         configurar();
                       } 
                     }
          break;
        }
      } else {
          while(configuracao == 1)//Enquanto estiver no modo de configuração, o programa vai ficar nesse loop
                { 
                  configurar();
                } 
      }
   }// fim do loop de configuração  
   
        irrigar();// Entra no modo de irrigação
 
}//fim do void loop

/*-------------------Funções personalizadas-------------------*/

void configurar()
{
    
    mod[modular] = digitalRead(modulo_pino[modular]);// só vai levar em conta o status de conexão do próximo módulo quando o atual for todo configurado
    switch(statuses)
  {

  case 0:// telefone;
if(reutiliza_telefone == 0)
{
  if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  {
  lcd.setCursor(0, 0);
  lcd.print("insira o telefone");
  lcd.setCursor(0,1);
  lcd.print("(");
  lcd.print(d[0]);
  lcd.print(d[1]);
  lcd.print(")");
  lcd.print(d[2]);
  lcd.print(d[3]);
  lcd.print(d[4]);
  lcd.print(d[5]);
  lcd.print(d[6]);
  lcd.print("-");
  lcd.print(d[7]);
  lcd.print(d[8]);
  lcd.print(d[9]);
  lcd.print(d[10]);
    if(guia == 0 || guia == 1)
           {
           lcd.setCursor(guia+1 ,1);
           } else if(guia > 1 && guia <= 6)
           {
           lcd.setCursor(guia+2 ,1);
           } else if(guia > 6 && guia <= 11)
           {
           lcd.setCursor(guia+3 ,1);
           }
           
  // Exibe (xx)xxxxx-xxxx no display, onde x são os dígitos
//  if(pisca_pisca == 0)
 // {
    lcd.cursor();
    lcd.blink();
  //  pisca_pisca = 1;
 // }
  }

  
  butaun(9, statuses);
} else
  {statuses++;     }
  break;
  

  case 1://Cultura;
  if(reutiliza_cultura[modular] == 0)
{
  if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  {
  lcd.setCursor(0, 0);
  lcd.print("tipo de cultura:");
  lcd.setCursor(0,1);
  
    if(cultura == 0)
      {lcd.print("Monocultura");}
      else
      {lcd.print("Policultura");}
  }
  butaun(0, statuses);// Colocar no código final (parada mudada e aparentemente, dá bom passar esses parâmetros)
  } else
  {statuses++;     }
  break;

case 2://Hortaliça 1;
  if(reutiliza_planta1[modular] == 0)
{
 if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  {
  lcd.setCursor(0, 0);
  lcd.print("Hortalica 1:");
  lcd.setCursor(0,1);
  
    switch(d[0])
   {
     case 0:
     lcd.print("Alface Americana");
     break;

     case 1:
     lcd.print("Brocolis        ");
     break;

     case 2:
     lcd.print("Tomate          ");
     break;
   } 
  }
  butaun(2,statuses); // se não der bom, coloca dentro do if(enter tralala)
  }else if(modulo[modular].cult == 0)
   {statuses = 4;}
   else
  {statuses++;     }
  break;

case 3://Hortaliça 2;
  if(reutiliza_planta2[modular] == 0)
{ 
 if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  {
  lcd.setCursor(0, 0);
  lcd.print("Hortalica 2:");
  lcd.setCursor(0,1);
  
    switch(d[0])
   {
     case 0:
     lcd.print("Alface Americana");
     break;

     case 1:
     lcd.print("Brocolis         ");
     break;

     case 2:
     lcd.print("Tomate           ");
     break;
   } 
  }
  butaun(2,statuses); // se não der bom, coloca dentro do if(enter tralala)
} else
  {statuses++;     }      
  break;
  
case 4:// Estágio de crescimento da planta
if(reutiliza_Estagio[modular] == 0)
{
  //EEPROM.update(17, t.mon);
  //EEPROM.update(18,t.day);
  //EEPROM.update(19, "fase de crescimento");
  if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  {
  lcd.setCursor(0, 0);
  lcd.print("Fase da planta:");
  lcd.setCursor(0,1);
  
   switch(d[0])
   {
   case 0:
   lcd.print("Broto    ");
   
   break;

   case 1:
   lcd.print("Crescendo");
   break;

   case 2:
   lcd.print("Crescida  ");//mudar depois
   break;
   }
  } 
  butaun(2, statuses);
  
}

else
  {
    statuses++;
  }

break;

case 5:// Horário;
if(reutiliza_Hora[modular] == 0 || reutiliza_Minuto[modular] == 0)
{ 
if(reutiliza_Hora[modular] == 1)
{
d[0] = (modulo[modular].Hr.toInt())/10;   
d[1] = (modulo[modular].Hr.toInt())%10; 
}
if(reutiliza_Minuto[modular] == 1)
{
d[2] = (modulo[modular].Min.toInt())/10;
d[3] = (modulo[modular].Min.toInt())%10;
}    
 if(enter == 0 && confirmar == 0) //Garantir que só vai ser executado enquanto não tiver a primeira confirmação
  { // Verificar posteriormente

  lcd.setCursor(0,0);
  lcd.print("insira o horario");
  lcd.setCursor(0,1);
  lcd.print(d[0]);
  lcd.print(d[1]);
  lcd.print("h");
  lcd.print(d[2]);
  lcd.print(d[3]);
  lcd.setCursor(0,1);
  
  if(pisca_pisca == 0)
  {
    lcd.cursor();
    lcd.blink();
    pisca_pisca = 1;
  }
  
   switch(guia)
   {
    case 0:
    maxhora = 2;
    lcd.setCursor(guia ,1);
    if(d[1] > 3 && d[0] == 2)
    {
      d[1] = 0;
    }
    break;

    case 1:
    if(d[0] == 2)
    {
    maxhora = 3;
    }
    else
    {
    maxhora = 9;      
    }
    lcd.setCursor(guia ,1);
   // deslocahora = 1;
    break;
    
    case 2:
    maxhora = 5;
    //deslocahora = 3;
    lcd.setCursor(guia+1 ,1);
    break;

    case 3:
    maxhora = 9;
    lcd.setCursor(guia+1 ,1);
    break;
   }
   
  }
    butaun(maxhora, statuses);
} else
  {statuses++;     }  
  break; 
  
case 6:// Módulos;
if(modular == 0)//Se estiver no módulo central
{ 
  digitalWrite(modulo_pino[modular], HIGH);//Liga o led(Isso é só pra teste)
  delay(1000);
  digitalWrite(modulo_pino[modular], LOW);//Liga o led(Isso é só pra teste)
  
  //Comandos abaixo para teste
     Serial.print("modular_central:");
     Serial.println(modular);
     Serial.print("cultura_central:");
     Serial.println(modulo[modular].cult);
     Serial.print("planta1_central:");
     Serial.println(modulo[modular].prant1);
     Serial.print("planta2_central:");
     Serial.println(modulo[modular].prant2); // se der qualquer valor doido, é monocultura esse módulo
     Serial.print("Hora_central:");
     Serial.println(modulo[modular].Hr);
     Serial.print("Minutos_central:");
     Serial.println(modulo[modular].Min);
//Acabou o teste

     d[0] = modulo[0].Hr.toInt();
     d[1] = modulo[0].Min.toInt();
     Serial.print("Hora:");
     Serial.println(d[0]);
     Serial.print("Minutos:");
     Serial.println(d[1]);
     
/* Descomentar só na etapa final do código, para não ficar desgastando a EEPROM
     EEPROM.update(12, modulo[0].cult);
     EEPROM.update(13, modulo[0].prant1);
     if(modulo[0].cult == 1)
     {
     EEPROM.update(14, modulo[0].prant2);
     }
     EEPROM.update(15, d[0]);//Guarda as Horas do módulo 0 numa posição só.
     EEPROM.update(16, d[1]);//Guarda os Minutos do módulo 0 numa posição só.

     */
    

     modular++;//Verificação do próximo módulo
     mod[modular] = digitalRead(modulo_pino[modular]);//leitura do módulo 1;

     if(mod[modular] == 0)// Vai verificar se o módulo 1 existe, pra poder depois passar para a próxima configuração
    {
    lcd.setCursor(0,0);
    lcd.print("Configuracao:");
    lcd.setCursor(0,1);
    lcd.print("Modulo ");
    lcd.print(modular);
    statuses = 1;
    delay(1000);
    lcd.clear();
    //Vai fazer a configuração do módulo 1 e só depois vai configurar o pinMode dele
     }
}
else if(mod[modular] == 0 && modular > 0 && modular <= modlimite)// Se o módulo estiver conectado
   {
     pinMode(modulo_pino[modular], OUTPUT);//Define o pino como saida
     digitalWrite(modulo_pino[modular], HIGH);//Liga o led(Isso é só pra teste)
     delay(1000);
     digitalWrite(modulo_pino[modular], LOW);//Liga o led(Isso é só pra teste)
     
     modulo_ativado[modular] = 1;

     
   //Comandos abaixo para teste
     Serial.print("modular_présoma:");
     Serial.println(modular);
 
     Serial.print("modular:");
     Serial.println(modular);
     Serial.print("cultura:");
     Serial.println(modulo[modular].cult);
     Serial.print("planta1:");
     Serial.println(modulo[modular].prant1);
     Serial.print("planta2:");
     Serial.println(modulo[modular].prant2);
     Serial.print("Hora:");
     Serial.println(modulo[modular].Hr);
     Serial.print("Minutos:");
     Serial.println(modulo[modular].Min);
    //Comandos acima para teste
     
     d[0] = modulo[0].Hr.toInt();
     d[1] = modulo[0].Min.toInt();
     Serial.print("Hora:");
     Serial.println(d[0]);
     Serial.print("Minutos:");
     Serial.println(d[1]);
     
     /* Descomentar só na etapa final do código, para não ficar desgastando a EEPROM
     EEPROM.update(12+(5*modular), modulo[modular].cult);
     EEPROM.update(13+(5*modular), modulo[modular].prant1);
     if(modulo[modular].cult == 1)
     {
     EEPROM.update(14+(5*modular), modulo[modular].prant2);
     }
     EEPROM.update(15+(5*modular), d[0]);//Guarda as Horas do módulo 0 numa posição só.
     EEPROM.update(16+(5*modular), d[1]);//Guarda os Minutos do módulo 0 numa posição só.

     */
     
     modular++;
     Serial.print("modular_póssoma:");
     Serial.println(modular);
     mod[modular] = digitalRead(modulo_pino[modular]);//leitura do módulo 1;
     //Serial.println(mod[modular]);
     if(mod[modular] == 0 && modular < modlimite){ //Verificação do mod[modular] em cima do novo valor de modular(modular+1)
     lcd.setCursor(0,0);
     lcd.print("Configuracao:");
     lcd.setCursor(0,1);
     lcd.print("Modulo ");
     lcd.print(modular);
     statuses = 1;
     delay(1000);
     lcd.clear();}
   
   //Executa nova configuração;
   
 } 
  if(mod[modular] || modular == modlimite) //Se estiver desconectado
 {
     pinMode(modulo_pino[modular], INPUT_PULLUP);//mantém do jeito que tá
     lcd.setCursor(0,0);
     lcd.print("Sem modulos extras");
     delay(500);
     lcd.setCursor(0,1);
     lcd.print("Fim da configuracao!");
     delay(1000);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Pressione o botão de");
     lcd.setCursor(0,1);
     lcd.print("reset p/ configurar");
     lcd.setCursor(0,2);
     lcd.print("novamente");
     
     delay(1000);
     configuracao = 0;
     statuses = 0;
     modular = 0;
     d[0] = 0;
     d[1] = 0;
     Umidade_Limiarmin = 400;// Para teste
     Umidade_Limiarmax = 800;//Para teste
     reutilizar = 1;
     lcd.clear();
    // Verificar o modo de alterar os valores da umidade.
 } 
  break;  
  
  }// fim do switch statuses  
}




void irrigar(){ 
  //lcd.clear();
  //Entrou na função irrigar
  while(configuracao == 0)
   {
   bool queda_energia = digitalRead(controle_bateria); 
   butaun(0,statuses); //Sem parâmetros
   if(queda_energia == 1 && trava_SMS[5] == 0)//Energia caiu
   {
    Estado_SMS = 6;
    SMS(Estado_SMS);
    trava_SMS[5] = 1;
    trava_SMS[7] = 0;

   }else if(queda_energia == 0 && trava_SMS[7] == 0)//Energia Retornou
   {
    Estado_SMS = 8;
    SMS(Estado_SMS);
    trava_SMS[5] = 0;
    trava_SMS[7] = 1;

   }

   if(porcentagem == Energia_Media || porcentagem == Energia_Baixa)
   {
   
    Estado_SMS = 7;
    SMS(Estado_SMS);

   } else if(porcentagem == Energia_Critica)
   {
   
    Estado_SMS = 7;
    SMS(Estado_SMS);
    Nivel_Critico = 1;
   } 
     //Comandos do processo de irrigação
     lcd.setCursor(0,0);
     lcd.print("Modo: Irrigacao     ");
     printTime();
    
     while(modular<=modlimite)// pensar melhor nesse mano
    {
    
     if(modulo_ativado[modular] == 1)
      {
        funcionamento(analogRead(Sensor_Solo[modular]));
      }
       printTime();//chamar o print time aqui dentro também(VERIFICAR DEPOIS)
       butaun(0,statuses); //Sem parâmetros
     Serial.println(modular);
     if(modulo[modular].Hr == hora_master && modulo[modular].Min == minuto_master && perigoTotal == 0 && modulo_ativado[modular] == 1)
     //O horário deve ser igual ao inserido nas configurações, deve estar apto para irrigar e o módulo deve estar conectado.
     {//pode irrigar.
      irriga[modular] = 1;
      if(trava_SMS[0] == 0)
      {
        Estado_SMS = 1;
        SMS(Estado_SMS);
        if(modular = modlimite)// vai garantir que só vai enviar uma vez a cada irrigação do módulo.
        {
        trava_SMS[0] = 1;
        }            
      }
      //Ver se rola de botar os comandos de irrigação aqui dentro mesmo;
     
     digitalWrite(modulo_pino[modular], HIGH);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Mod.");
     lcd.print(modular);
     lcd.print(": irrigando");
     delay(500);
     lcd.print(".");
     delay(500);
     lcd.print(".");
     delay(500);
     lcd.print(".");
     delay(500);
     //lcd.clear();
     } else if(modulo[modular].Hr == hora_master && modulo[modular].Min == minuto_master && perigoTotal == 1 && irriga[modular] == 0 && modulo_ativado[modular] == 1)//Não começou a irrigar
      {   digitalWrite(modulo_pino[modular], LOW);
          // lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Mod.");
          lcd.print(modular);
          lcd.print("nao irrigado!  ");
          lcd.setCursor(0,1);
          lcd.print("                     ");
          delay(500);
          irriga[modular] = 0;
     } 
     /*else if( perigoTotal == 1 && irriga[modular] == 1)//Parou no meio da irrigação
     
      {   digitalWrite(modulo_pino[modular], LOW);
          // lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("irrigacao do Modulo ");
          lcd.print(modular);
          lcd.setCursor(0,1);
          lcd.print(" interrompida! ");
          delay(500);
          irriga[modular] = 0;
          if(perigo1 == 1 && trava_SMS[2] == 0)//Se a temperatura está alta
          {
            Estado_SMS = 3;
            SMS(Estado_SMS);
            trava_SMS[2] = 1;            
          }else if(perigo2[modular] == 1 && trava_SMS[1] == 0)//Se a umidade não está legal para irrigar
          {
            Estado_SMS = 2;
            SMS(Estado_SMS);
            if(modular = modlimite)
            {
            trava_SMS[1] = 1;            
            }
          }
     }*/
     else if(modulo[modular].Min != minuto_master/*+quantidade de tempo necessária para irrigar talvez*/ && irriga[modular] == 1)//Se estava irrigando(concluída)
     {
          digitalWrite(modulo_pino[modular], LOW);
          // lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Mod.");
          lcd.print(modular);
          lcd.print("irrigado!      ");
          lcd.setCursor(0,1);
          lcd.print("                     ");
          delay(500);
          irriga[modular] = 0;
          if(trava_SMS[3] == 0)
           {
             Estado_SMS = 4;
             SMS(Estado_SMS);
             if(modular = modlimite)// vai garantir que só vai enviar uma vez a cada do módulo.
              {
                trava_SMS[3] = 1;
              }            
           }
           if(trava_SMS[4] == 0 && analogRead(Sensor_Solo[modular]) >= Umidade_Limiarmax && modulo_ativado[modular] == 1)
           {
              Estado_SMS = 5;
              SMS(Estado_SMS);
              
              if(modular = modlimite)// vai garantir que só vai enviar uma vez a cada módulo.
              {
                trava_SMS[4] = 1;
              }

           }
           
      
      
        }
        delay(250);
        modular++;
    }
    if(modular>modlimite)
    modular = 0;

          
}// Fim do while(configuração == 0);
}// Fim do void Irrigar

void printTime() // Exibição do tempo e outras configurações relacionadas à data e hora
{
  Time t = rtc.time();

  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",t.mon, t.date, t.hr, t.min, t.sec);


if(t.hr < 10)//Inserir o zero à esquerda, se a hora for menor que 10
{
 hora_master.replace(hora_master, "");
 hora_master.concat("0");
 hora_master.concat(t.hr);

} else if( t.hr >= 10)
{
 hora_master = t.hr;
}

if(t.min < 10)//Inserir o zero à esquerda, se os minutos forem menor que 10
{
 minuto_master.replace(minuto_master, "");
 minuto_master.concat("0");
 minuto_master.concat(t.min);
 
} else if( t.min >= 10)
{
 minuto_master = t.min;
}     
     lcd.setCursor(15,3);
     lcd.print(hora_master);
     lcd.print("h");
     lcd.print(minuto_master);
    // Serial.print("hora_master:");// teste
     //Serial.println(hora_master); // teste
     
    // Serial.print("minuto_master:");// teste
    // Serial.println(minuto_master);// teste
     delay(500); // teste

//Verificar de inserir as configurações das estações no switch abaixo
    if((t.mon >= 9 && t.day >= 23) && (t.mon <= 12 && t.day < 21)) //início estações
     primavera = true;

     else primavera = false;

    if((t.mon >= 12 && t.day >= 21) && (t.mon <= 3 && t.day < 21)) 
     verao = true;

     else verao = false;

     if((t.mon >= 3 && t.day >= 21) && (t.mon <= 6 && t.day < 21)) 
     outono = true;

     else outono = false;

     if((t.mon >= 6 && t.day >= 23) && (t.mon <= 9 && t.day < 24)) 
     inverno = true;

     else inverno = false; //fim estações

     //if(primavera || verao)
     //modo de irrigação verão
     
     //else
     //modo de irrigação inverno

     switch (t.mon)
     {
       case 1:
       {
         Tmed = 23.5;
         NDP = 31;
         Qo = 16.7;
       }

       case 2:
       {
         Tmed = 23.5;
         NDP = 28;
         Qo = 16;
       }

       case 3:
       {
         Tmed = 23.5;
         NDP = 31;
         Qo = 14.5;
       }

       case 4:
       {
         Tmed = 21.5;
         NDP = 30;
         Qo = 12.4;
       }

       case 5:
       {
         Tmed = 18.5;
         NDP = 31;
         Qo = 10.6;
       }

       case 6:
       {
         Tmed = 17.5;
         NDP = 30;
         Qo = 9.6;
       }

       case 7:
       {
         Tmed = 17;
         NDP = 31;
         Qo = 10;
       }

       case 8:
       {
         Tmed = 18;
         NDP = 31;
         Qo = 11.5;
       }

       case 9:
       {
         Tmed = 19;
         NDP = 30;
         Qo = 13.5;
       }

       case 10:
       {
         Tmed = 21;
         NDP = 31;
         Qo = 15.3;
       }

       case 11:
       {
         Tmed = 21.5;
         NDP = 30;
         Qo = 16.2;
       }

       case 12:
       {
         Tmed = 23;
         NDP = 31;
         Qo = 16.8;
       }
     }//fim switch mês

     ETP = 0,01 * Qo * Tmed * NDP; //cálculo do ETP local

//Qagua da planta = (ETP*Kc)/Eficiencia da rega   

  
alface.Qagua_Brotinho = (ETP*0.7)/EficienciaRega;// EficienciaRega = 0.75
alface.Qagua_Medio = (ETP*1)/EficienciaRega; 
alface.Qagua_Crescida = (ETP*0.95)/EficienciaRega;

brocolis.Qagua_Brotinho = (ETP*0.7)/EficienciaRega; 
brocolis.Qagua_Brotinho = (ETP*1.07)/EficienciaRega; 
brocolis.Qagua_Crescida = (ETP*0.95)/EficienciaRega; 

tomate.Qagua_Brotinho = (ETP*0.6)/EficienciaRega; 
tomate.Qagua_Medio = (ETP*1.15)/EficienciaRega; 
tomate.Qagua_Crescida = (ETP*0.7)/EficienciaRega;



  if (alfaceM)
  {
    if(brotoM)
   tempo_irrigando1 = alface.Qagua_Brotinho * Qb / vazaoDaBomba ;
   
    if(crescendoM)   
   tempo_irrigando1 = alface.Qagua_Medio * Qb / vazaoDaBomba ;
    
    if(crescidaM)
   tempo_irrigando1 = alface.Qagua_Crescida * Qb / vazaoDaBomba ;
  }

  if (brocolisM)
  {
    if(brotoM)
    tempo_irrigando1 = brocolis.Qagua_Brotinho * Qb / vazaoDaBomba ;

    if(crescendoM)
    tempo_irrigando1 = brocolis.Qagua_Medio * Qb / vazaoDaBomba ;

    if(crescidaM)
    tempo_irrigando1 = brocolis.Qagua_Crescida * Qb / vazaoDaBomba ;
  }  

  if (tomateM)
  {
    if(brotoM)
    tempo_irrigando1 = tomate.Qagua_Brotinho * Qb / vazaoDaBomba ;

    if(crescendoM)
    tempo_irrigando1 = tomate.Qagua_Medio * Qb / vazaoDaBomba ;

    if(crescidaM)
    tempo_irrigando1 = tomate.Qagua_Crescida * Qb / vazaoDaBomba ;
  }





  if (alfaceP)
  {
    if(brotoP)
   tempo_irrigando2 = alface.Qagua_Brotinho * Qb / vazaoDaBomba ;
   
    if(crescendoP)   
   tempo_irrigando2 = alface.Qagua_Medio * Qb / vazaoDaBomba ;
    
    if(crescidaP)
   tempo_irrigando2 = alface.Qagua_Crescida * Qb / vazaoDaBomba ;
  }

  if (brocolisP)
  {
    if(brotoP)
    tempo_irrigando2 = brocolis.Qagua_Brotinho * Qb / vazaoDaBomba ;

    else if(crescendoP)
    tempo_irrigando2 = brocolis.Qagua_Medio * Qb / vazaoDaBomba ;

    else if(crescidaP)
    tempo_irrigando2 = brocolis.Qagua_Crescida * Qb / vazaoDaBomba ;

    else
    tempo_irrigando2 = 0;
  }  

  if (tomateP)
  {
    if(brotoP)
    tempo_irrigando2 = tomate.Qagua_Brotinho * Qb / vazaoDaBomba ;

    if(crescendoP)
    tempo_irrigando2 = tomate.Qagua_Medio * Qb / vazaoDaBomba ;

    if(crescidaP)
    tempo_irrigando2 = tomate.Qagua_Crescida * Qb / vazaoDaBomba ;
  }



//arduino =  tempo_irrigando1 + tempo_irrigando2// TEMPO QUE A PORTA DA BOMBA FICARÁ EM NÍVEL ALTO
  
     //if(hora_master)
   //  {
     // lcd.noBacklight();
    // }
}// Fim do void printTime.

void funcionamento(float Umid_Solo)
{
  
umid = dht.readHumidity();
temp = dht.readTemperature();

sensorUmiTempAr(umid, temp);//Um sensor por sistema.
sensorUmiSolo(Umid_Solo ,Umidade_Limiarmin, Umidade_Limiarmax); // Um sensor por módulo

  if(perigo1 == 1 || perigo2[modular] == 1)
  {
    perigoTotal = 1;//Não rola de irrigar
  } else if(perigo1 == 0 || perigo2[modular] == 0)
  {
    perigoTotal = 0;//Irrigável
  }
}// Fim do void funcionamento.
 
void sensorUmiTempAr(float hum, float tem) //DHT
{
  lcd.setCursor(0,2);
  lcd.print("U:");
  lcd.print(hum);
  lcd.print("% T:");
  lcd.print(tem);
  lcd.print("C");

  if(tem >= 35)
  {
    perigo1 = true;//temperatura deveras alta para a planta
  }
  else
  {
    perigo1 = false;
  }
}//Fim do void sensorUmiTempAr

void sensorUmiSolo(short Sensor_umiSolo,short Umimin, short Umimax)//Umidade do solo
//Umimin = Valor minimo para ser considerado solo com umidade moderada
//Umimax = Valor máximo para ser considerado solo com umidade moderada
{
  lcd.setCursor(0,3);
  lcd.print("SoloMod:");
  lcd.print(modular);
 
  lcd.print(":");
  if(Sensor_umiSolo >= Umimax)
  {
    //lcd.setCursor(6,3);
    lcd.print("seco ");
    perigo2[modular] = false;
  }

  if(Sensor_umiSolo >= Umimin && Sensor_umiSolo < Umimax)
  {
    //lcd.setCursor(6,3);
    lcd.print("medio");
    perigo2[modular] = true;
  }

  if(Sensor_umiSolo >= 0 && Sensor_umiSolo < Umimin)
  {
    //lcd.setCursor(6,3);
    lcd.print("umido");
    perigo2[modular] = true;// não pode molhar, pois já está úmido
  }
 // delay(500);
}//Fim do void sensorUmiSolo


 //Tem que verificar como escrever o telefone sem problemas
void SMS(byte controleSMS)
{
 
porcentagem = analogRead(bateria)*(100.0/1023.0); // Verificação do status da bateria. Talvez colocar numa função personalizada
//precontroleSMS = controleSMS;

//if(trava_SMS == 0 && controleSMS = precontroleSMS)//Se o SMS ainda não tiver sido enviado
//{
switch (controleSMS)// Define as diretivas e modelos de mensagem que o módulo deve seguir de acordo com cada caso
{
case 1:// Plantação será irrigada
//dá pra implementar as variáveis de controle para resolver tudo dentro do switch
 
  //Serial2.write("AT+CMGF=1\r\n");
  delay(1000);
  Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
  Serial2.write("Ola! A plantacao do modulo ");
  Serial2.write(modular);
  Serial2.write("sera regada.");
  delay(1000);
  Serial2.write((char)26);
  delay(1000);






//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que a plantacao sera regada");
controleSMS = 0;//}
break;
/*
case 2:// Plantação não será irrigada pois o solo está umido

Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Ola. A plantacao do modulo ");
Serial2.write(modular);
Serial2.write(" nao sera regada pois o solo ja esta umido.");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{Serial.println("O numero cadastrado foi alertado que a plantacao nao sera regada pois o solo esta umido.");
controleSMS = 0;//}
break;

case 3:// Plantação não será irrigada pois a temperatura pode prejudicar as plantas

Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Ola. A(s) plantacao(coes) nao sera regada pois a temperatura pode prejudicar as hortaliças.");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que a plantacao nao sera regada pois a temperatura pode prejudicar as hortaliças.");
controleSMS = 0;//}
break;

case 4:// Plantação foi irrigada com sucesso

Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Atualizacoes! A plantacao do modulo ");
Serial2.write(modular);
Serial2.write(" foi irrigada com sucesso.");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que a plantacao foi regada com sucesso.");
controleSMS = 0;
break;

case 5:// Plantação foi irrigada com problema (sensores nao detectaram alteração significativa de umidade)

Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write(" Ola. A plantacao ");
Serial2.write(modular);
Serial2.write("nao foi irrigada com sucesso. Alguns motivos para isso são:\n");
Serial2.write("1. Pode haver obstrucao na tubulacao ou mangueira de distribuicao;\n");
Serial2.write("2. Pode haver vazamento na tubulacao ou mangueira de distribuicao;\n");
Serial2.write("3. O fornecimento de agua pode ter sido interrompido;\n");
Serial2.write("4. Houve problema nos atuadores ou nas bombas hidraulicas(se houver);\n");
Serial2.write("5. Houve problema no circuito do sensor de umidade;\n"); 
Serial2.write("Por favor, verifique o sistema ou contacte a assistencia tecnica o quanto antes.");  
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que ha possiveis avarias no sistema e que uma verificação deve ser realizada.");
controleSMS = 0;//}

//OBS: Para um conjunto aspersor+sensor
//OBS1: Caso o tempo de irrigação seja atingido e o sensor não tenha detectado variações significativas de umidade.


break;

case 6: //Aviso de queda de energia
Serial2.write("AT+CFUN=0"); //Gasto mínimo de energia
Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Olá! O sistema está sem energia da rede elétrica, e está sendo alimentado por baterias. Algumas funções podem não funcionar, caso essa condição se prolongue por muito tempo");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
  Serial.println("O numero cadastrado foi alertado que houve queda de energia e o sistema pode não funcionar corretamente");

controleSMS = 0;//}

break;

case 7: //Bateria fraca (30%, xx% e 18%)

Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Ola! A autonomia da bateria está em ");
Serial2.write(porcentagem);
Serial2.write("%");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que a bateria está com baixa autonomia");
controleSMS = 0;//}
if(Nivel_Critico){// O sistema desativará algumas funções para poupar energia
Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Para poupar energia, o sistema desativara algumas funções, como o envio de mensagens SMS, até que a energia da rede elétrica seja restaurada"); 
Serial2.write("AT+CSCLK=1"); //Sleep mode do SIM800L
Serial.println("O numero cadastrado foi alertado que o sistema desativará algumas funções, como enviar SMS, para poupar energia");
}
controleSMS = 0; 

case 8: //A energia da rede elétrica retornou
Serial2.print("AT+CMGS=\"" + telefone + "\"\n");
Serial2.write("Ola. A energia da rede elétrica foi restaurada. O sistema ativará todas suas funções novamente");
Serial2.write((char)26);
//if(Serial2.available())
//{SerialGSM = Serial.read();}
//while(SerialGSM.indexOf("+CMGS:") == -1)
//{
Serial.println("O numero cadastrado foi alertado que a energia da rede elétrica retornou e o sistema ativará as funções novamente");
//}
//digitalWrite(DTR, LOW);
Serial2.write("AT+CSCLK=0"); //Sleep mode do SIM800L
break;*/
}
//}
//Estado_SMS = 0;
} 



void Teclado()
{ 
  
  resetou = digitalRead(resetar);
  float entrada = analogRead(botao);
  float tensao;
   
  if (tensao != (entrada * 5/1023)) //Zera a entrada dos botões
  {
    tensao = entrada * 5/1023;    
  }
  
  if (tensao >= 0 && tensao < 0.08 && controleBotao)//Botão da direita pressionado
  {
    //Direita    
    controleBotao = false;
    avanco = 1;
    Serial.println("Direita pressionada!");
    
  }

  if (tensao >= 0.08 && tensao < 0.3 && controleBotao)
  {
    //Baixo
    controleBotao = false;
    subtrair = 1;
    Serial.println("Baixo pressionado!");
  }

  if (tensao >= 0.3 && tensao < 0.66 && controleBotao)
  {
    //Cima    
    controleBotao = false;
    adicionar = 1;
    Serial.println("Cima pressionado!");
  }

  if (tensao >= 0.66 && tensao < 1.29 && controleBotao)
  {
    //Esquerda    
    controleBotao = false;
    retorno = 1;
    Serial.println("Esquerda pressionada!");
  }

  if (tensao >= 1.29 && tensao < 3 && controleBotao)
  {
    //Confirmar    
    controleBotao = false;
    enter = 1;
    Serial.println("Confirmar pressionado!");
  }

  if (tensao >= 3 && !controleBotao)
  {
    //Nenhum botão
    controleBotao = true;
    retorno = 0; // Botão de retorno
    avanco = 0; // Botão de avanço
    adicionar = 0; // Botão de incremento
    subtrair = 0; // Botão de decremento
    enter = 0; // Botão de confirmar
    reset = 0;
    Serial.println("Zerou tudo!");
  }
  if(resetou == 0 && controleBotao)
  {
    Serial.println("Cancelar Pressionado!");
    controleBotao = false;
    reset = 1;
  }
  //Serial.println(tensao);
}//Fim do void Teclado

void butaun(byte maximo, byte progresso){ //controla as ações dos botões

  Teclado(); // Função que verifica quais botões foram apertados
  
  if(progresso == 6)// modo de reutilização de dados
  {     
  if(avanco && d[0] < 7)
    {
   if(d[guia] == 1)
   {
     d[guia]++;
   }   else
      {
        d[guia] = 1; 
      } 
   }
   
     if(retorno && d[0] < 7)
     {
      if(d[guia] == maximo)
      {
       d[guia]--;
      } else
        { 
         d[guia] = maximo;
         }
     }

     
     if(d[guia] == 0)// Exibe branco. (só ocorre no começo)
     {
      lcd.setCursor(14, 1);
      lcd.print("    ");
      delay(500);
     }
     
     if(d[guia] == 1)// Sim
     {
      lcd.setCursor(14, 1);
      lcd.print("Sim.");
      delay(500);
            
     }else if(d[guia] == 2)//Não
      {
       lcd.setCursor(14, 1);
       lcd.print("Nao.");
       delay(500);
      }

      if(enter && d[guia] > 0 )
        {
            lcd.setCursor(0,3);
            lcd.print(" Confirmado!");
            delay(500); // Ver se está bom o comando aqui
            lcd.clear(); // Ver se está bom o comando aqui
              
          if(d[guia] == 1)
          {
            switch(guia)
            {
              case 1: // Telefone
              for(guia = 0; guia<=10; guia++)
              {
              //telefone.concat(EEPROM.read(guia+1));
              }
              guia = 1;// ficar o valor do case novamente
              telefone = Tele;// para teste
              reutiliza_telefone = 1;
              d[0]++;
              
              break;


              case 2: // Módulos
              d[0]++;
              break;


              case 3: // Cultura
                  
              //modulo[modular].cult = EEPROM.read(12+(5*modular));
              modulo[modular].cult = cultur;
             // Serial.print("cultura");
              //Serial.println(modulo[modular].cult);
              //Quando acabar o o concat, executa o comando abaixo
              reutiliza_cultura[modular] = 1;
              d[0]++;
                 
              break;

              
              case 4:
              //modulo[modular].planta1 = EEPROM.read(13+(5*modular));
              modulo[modular].prant1 = p1;
              //Serial.print("planta1");
              //Serial.println(modulo[modular].prant1);
              //Quando acabar o o concat, executa o comando abaixo
              reutiliza_planta1[modular] = 1;
              if(/*EEPROM.read(12+(5*modular)) == 0*/modulo[modular].cult == 0)
                 {//Se for monocultura(?)
                   d[0]++;
                   //Ler valor da fase da planta
                 } else
                    {
                      
                      d[0] = 5;//Verificar se não é 6
                    }
              break;


              case 5:
              modulo[modular].prant2 = p2;
              
             // Serial.print("planta2");
            //  Serial.println(modulo[modular].prant2);
              //modulo[modular].planta2 = EEPROM.read(14+(5*modular) );
              //Quando acabar o o concat, executa o comando abaixo
              d[0]++;
              reutiliza_planta2[modular] = 1;
             
              break;


              case 6:
               modulo[modular].Hr = Hr_Teste;
               //Serial.print("Horas");
              // Serial.println(modulo[modular].Hr);
               //modulo[modular].Hr = EEPROM.read(15+(5*modular);
               reutiliza_Hora[modular] = 1;
               d[0]++;
              
              break;


              case 7:
               modulo[modular].Min = Min_Teste;
               //Serial.print("Minutos");
              // Serial.println(modulo[modular].Min);
               //modulo[modular].Min = EEPROM.read(16+(5*modular));
               reutiliza_Minuto[modular] = 1;
               d[0]++;
              
              break;
            }
          }               
            else if (d[guia] == 2)
            //Se não quiser, não executa nada.
               {    
                  switch(guia)
                  {
                  case 2:// módulos
                  d[0] = 7;
                  break;


                  case 4:// Hortaliça 1
                   if(/*EEPROM.read(12+(5*modular)) == 0*/modulo[modular].cult == 0)
                       {
                         d[0]++;
                       } else 
                          {
                           d[0] = 5;
                          }
                   break;

                   default: //Outros casos
                    d[0]++;
                   break;
                   }
                }
            }
        
      

    
    
  }else if(configuracao == 1) // Se tiver no módulo de configuração. Usada pra anular a função dos outros botões durante o processo de irrigação.
  {
  
 if(subtrair == 1 && controleBotao == 0)//Botão de decremento
    {
    Serial.println("subtrair:");
 
    if(progresso == 1)
    {cultura =!cultura;}
    else if(d[guia] == 0)
   {
     d[guia] = maximo;
   }
   else    
   { 
     d[guia]--;
   }
    delay(500);
  }
  
  if( adicionar == 1 && controleBotao == 0)//Botão de incremento do digito
  {  
    Serial.print("adicionar:");
    
   if(progresso == 1)
    {cultura =!cultura;}
    else if(d[guia] == maximo)
  {
     d[guia] = 0;
   }
   else    
   { 
     d[guia]++;
   }
   delay(500);
  }// fim do incremento 

  if (enter && !confirmar)//Passar para a etapa da confirmação de dados
    { 
           
      switch(statuses)
      {
       Teclado();
      case 0: //Telefone
      if(guia > 9)//se não der bom com 9, troca por 10
          {
      telefone.concat(d[guia]); 
      Serial.print("Tel_Concat:");
      Serial.println(telefone);     
      lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1);
      lcd.print(telefone);//Pensar na separação depois
      Serial.print("Tel_LCD:");
      Serial.println(telefone);
      //o telefone será exibido assim: xxxxxxxxxxx
      //enquanto a formatação é assim: (xx)xxxxx-xxxx
      enter = false;
      confirmar = true; 
        }          
      break;

      case 1: //Cultura
    // confirmar = true; 
      lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1);
          if(cultura == 0)
             {
              lcd.print("Monocultura");
             }
             else
              {
               lcd.print("Policultura");
              }         
           enter = false;
           confirmar = true; 
            
       break;

    case 2://Planta1
      lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1);
    planta1 = d[0];   
    switch(planta1)
   {
   case 0:
   lcd.print("Alface Americana");
   break;

   case 1:
   lcd.print("Brocolis");
   break;

   case 2:
   lcd.print("Tomate");
   break;
   }
     enter = false;
     confirmar = true; 
   break;

      case 3://Planta2 (Se for policultura)
      lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1);
    planta2 = d[0];   
    switch(planta2)
   {
   case 0:
   lcd.print("Alface Americana");
   break;

   case 1:
   lcd.print("Brocolis");
   break;

   case 2:
   lcd.print("Tomate");
   break;
   }
     enter = false;
     confirmar = true;
      break;

     case 4: // etapa de crescimento
     //confirmar = true; 
      lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1);
      Etapa_planta = d[0];
         switch(Etapa_planta)
   {
   case 0:
   lcd.print("Broto");
   break;

   case 1:
   lcd.print("Crescendo");
   break;

   case 2:
   lcd.print("Crescida");
   break;
   }
           enter = false;
           confirmar = true;  
     break;


      case 5://Horário
      if(guia > 2)
     { 
       lcd.clear();
      lcd.noCursor();
      lcd.noBlink();
      pisca_pisca = 0;
      lcd.setCursor(0,0);
      lcd.print("Confirma? ");
      lcd.setCursor(0,1); 
      Hora.concat(d[0]);
      Hora.concat(d[1]);
     
      Minutos.concat(d[2]);
      Minutos.concat(d[3]);
      
     lcd.print(Hora);
     lcd.print("h");
     lcd.print(Minutos);
      
      enter = false;
      confirmar = true; 
      }
     break;             
    
     //Verificar se vai dar bom
   /*  if(avanco)// Sim
     {
      lcd.setCursor(14, 1);
      lcd.print("Sim.");
      delay(500);
      reset = false;
            
     }else if(retorno)//Não
      {
       lcd.setCursor(14, 1);
       lcd.print("Nao.");
       delay(500);
       reset = true;
      }*/
    //Término do verificar se vai dar bom
    
    
    
    }// Statuses

    }// Fim da enter && primeira confirmação

    
    if (enter && confirmar)// Segunda confirmação
      { 
      Serial.println("enter2:");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Confirmado:");
        lcd.setCursor(0,1);

        switch(statuses)
      {
      case 0: //Telefone
      lcd.print(telefone);
      Serial.print("Tel_LCD-Confirma:");
      Serial.println(telefone);
      break;

      case 1: //Cultura
      if(cultura == 0)
       {
        lcd.print("Monocultura");
        //modulo[modular].prant2 = null; // Fazer o teste posteriormente
        mono = 1;
        poli = 0;
       }
        else
        {
        lcd.print("Policultura");
        mono = 0;
        poli = 1;
        }
         modulo[modular].cult = cultura;
         cultura == 0; //Esta sendo zerado só para fins de padronização(mostrar o monocultura primeiro sempre)
      break;

      case 2://Planta1
     
    switch(planta1)
   {
   case 0:
   lcd.print("Alface Americana");
   alfaceM = true;//fins de cálculo de água
   brocolisM = false;//fins de cálculo de água
   tomateM = false;//fins de cálculo de água
   break;

   case 1:
   lcd.print("Brocolis");
   alfaceM = false;//cálculo de água
   brocolisM = true;//fins de cálculo de água
   tomateM = false;//fins de cálculo de água
   break;

   case 2:
   lcd.print("Tomate");//mudar depois
   alfaceM = false;
   brocolisM = false;//fins de cálculo de água
   tomateM = true;//fins de cálculo de água
   break;
   
   }
   
  modulo[modular].prant1 = planta1;
  planta1 = 0;//Esta sendo zerado só para fins de padronização(mostrar o monocultura primeiro sempre)

      break;

      case 3://Planta2 (Se for policultura)
     
        switch(planta2)
         {
          case 0:
          lcd.print("Alface Americana");
          alfaceP = true;//fins de cálculo de água
          brocolisP = false;//fins de cálculo de água
          tomateP = false;//fins de cálculo de água
          break;

          case 1:
          lcd.print("Brocolis");
          alfaceP = false;//fins de cálculo de água
          brocolisP = true;//fins de cálculo de água
          tomateP = false;//fins de cálculo de água
          break;

          case 2:
          lcd.print("Tomate");
          alfaceP = false;//fins de cálculo de água
          brocolisP = false;//fins de cálculo de água
          tomateP = true;//fins de cálculo de água
          break;
        }
   modulo[modular].prant2 = planta2;
   planta2 = 0;//Esta sendo zerado só para fins de padronização(mostrar o monocultura primeiro sempre)
   // se for monocultura, não salvar esse mano na EEPROM // Colocar no código final
   
      break;
      
  case 4://Etapa de crescimento
     
    switch(Etapa_planta)
   {
   case 0:
   lcd.print("Broto   ");
   if(mono)
   {
     brotoM = true;
     brotoP = false;
   }
   if(poli)
   {
     brotoM = false;
     brotoP = true;   
   }
   
   break;

   case 1:
   lcd.print("Crescendo");
   if(mono)
   {
     crescendoM = true;
     crescendoP = false;
   }
   if(poli)
   {
     crescendoM = false;
     crescendoP = true;     
   }
   break;

   case 2:
   lcd.print("Crescida  ");
   if(mono)
   {     
     crescidaM = true;
     crescidaP = false;
   }
   if(poli)
   {
     crescidaM = false;
     crescidaP = true;   
   }
     brotoM = false;
     brotoP = false;

     crescendoM = false;
     crescendoP = false;
    
   break;
   }
   modulo[modular].Etapa = Etapa_planta;
    Etapa_planta == 0; //Esta sendo zerado só para fins de padronização(mostrar o broto primeiro sempre)
   
      case 5://Horário
   
     lcd.print(Hora);
     lcd.print("h");
     lcd.print(Minutos);
     modulo[modular].Hr = Hora;
     modulo[modular].Min = Minutos;
     Hora = "";
     Minutos = "";
      break;
      
      }//Statuses
        
        confirmar = false;
        for(guia = 0; guia<=10; guia++) //zerar todos os dígitos
              {  
               d[guia] = 0;
              }  
        guia = 0;
        if(progresso == 2 && cultura == 0)
        {        
         statuses = 4;
        }
        else
        {
        statuses++;
        }
          delay(3000);//Se ficar muito tempo, troca pelo delay de baixo
         // delay(1000); 
          lcd.clear(); 
      }// Fim da enter && Confirmar

      
    
    if (reset && confirmar)// Botão de cancelamento da configuração atual
      {
      Serial.println("reset:");
            
        confirmar = false;
      
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Cancelado!");

         switch(statuses)
      {
      case 0: //Telefone
      telefone = "";
      Serial.print("Tel_Cancelado:");
      Serial.println(telefone);
      break;

      case 1: //Cultura
      cultura = 0;
      break;

      case 2://Planta1
      planta1 = 0;
      break;

      case 3://Planta2 (Se for policultura)
      planta2 = 0;
      break;

      case 4://Etapa de crescimento
      Etapa_planta = 0;
      break;
      
      case 5://Horário
      Hora = "";
      Minutos = "";
      break;
      }
        delay(1000);
        for(guia = 0; guia<=10; guia++) //zerar todos os dígitos
              {  
               d[guia] = 0;
              }  
              guia = 0;
              Serial.println("Cancelamento Concluido!");
      }//Final do reset && Confirmar  


      if( avanco == 1 && controleBotao == 0) // passar de dígito
      { 
        
        switch(statuses)
        {
          case 0:
          if (guia < 10)
          {
          telefone.concat(d[guia]);
         Serial.print("digito enviado:"); 
         Serial.println(d[guia]); //Para teste  
         Serial.print("Tel_digito-Avanço:");
      Serial.println(telefone);  
        guia++;
        
          //Serial.print("telefone:"); //Para teste
         // Serial.println(telefone);//Para teste
          } 
          break;
          
          case 5:
          
          
      if(guia < 3)
      { 
        guia++; 
      }
       
       break;    
        }// Fim do switch statuses  
       delay(500);
      }// Fim do avanço de dígito
      

      if(retorno == 1 && controleBotao == 0) // retorno de dígito
       { 
        switch(statuses)
        {
          case 0:
          if(guia > 0 && guia <= 10) 
          {
            guia--;
            telefone.remove(guia,1);
                    
          }else if(guia == 0)        
          {                           
           guia--;
           telefone.remove(guia,1);   
          
          }       
         break;

          case 5:
          
          if(guia > 0)
      { 
        guia--; 
      }  
          break;
        }// Fim do switch status
       delay(500); 
      }//Final do retorno de dígito

         
  }
    else if(reset && !controleBotao && configuracao == 0)// retornar ao modo de configuração
    {
     lcd.clear(); //Ver se vai limpar o LCD
     Serial.println("Configuração resetada!");
     configuracao = 1;
     //Estudar melhor as paradas abaixo. Ver de aplicar o sistema para pegar as configurações anteriores
     //telefone.replace(telefone, "");
     telefone = "";
     cultura = 0;
     planta1 = 0;
     planta2 = 0;
     Etapa_planta = 0;
     Hora = "";
     Minutos = "";
     //só precisaria do reset do telefone, já que os outros ocorrem após transferir os dados para o struct correspondente
    // lcd.clear();  // não está limpando o LCD
    }
    
}//Final da voud butaun

