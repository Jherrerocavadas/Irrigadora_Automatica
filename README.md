![Irrigadora_Automatica_Logo](https://user-images.githubusercontent.com/105129994/182738834-022fd367-2209-4994-9141-cda121e27faa.png)

<h1 align = "center">Irrigadora Automatica </h1>

<!--
![](https://img.shields.io/static/v1?label=Plataforma&message=<ArduinoMega2560>&color=<COLOR>&style=<STYLE>&logo=<LOGO>) -->

# Sobre
Irrigadora Automática com Banco de dados - Trabalho de Conclusão do Curso de Eletrônica na Etec Presidente Vargas

Esse projeto foi desenvolvido com o intuito de fornecer um sistema que gerencia a rega de uma ou mais hortaliças de maneira altamente autônoma. Dentre suas funcionalidades estão:

- Monitoramento da temperatura ambiente.
- Monitoramento da umidade do ar.
- Monitoramento da umidade do solo.
- Relógio em tempo real (RTC).
- Envio de mensagens de status via SMS.
- Irrigação por aspersão programada.
- Armazenamento de dados cruciais, persistentes à queda de energia.
- Sistema de alimentação elétrica secundária (por exemplo, baterias, em caso de queda de energia da rede elétrica).
- Indicador de carga da bateria.
- Flexibilidade de uso (o projeto pode trabalhar com a irrigação de uma área ou mais).

Observação: Nem todos os recursos podem estar implementados e/ou funcionando corretamente. Para verificar quais são as funcionalidades implementadas consulte a guia "[Funcionalidades](#Funcionalidades)", mais abaixo.

# Índice

* [Sobre](#sobre)
* [Índice](#indice)
* [Funcionalidades](#funcionalidades)
* [Demonstração](#demonstracao)
* [Pré Requisitos](#pre-requisitos)
  * [Hardware](#hardware)
  * [Software](#software)
  * [Outros](#outros)
* [Tecnologias](#tecnologias)
* [Autores](#autores)


# Funcionalidades

- [x] Monitoramento da temperatura ambiente.
- [x] Monitoramento da umidade do ar.
- [x] Monitoramento da umidade do solo.
- [x] Relógio em tempo real (RTC).
- [ ] Configuração intuitiva do horário no RTC.
- [ ] Envio de mensagens de status via SMS.
- [x] Irrigação por aspersão programada.
- [ ] Armazenamento de dados cruciais, persistentes à queda de energia.
- [ ] Sistema de alimentação elétrica secundária (por exemplo, baterias, em caso de queda de energia da rede elétrica).
- [ ] Indicador de carga da bateria.
- [x] Flexibilidade de uso (o projeto pode trabalhar com a irrigação de uma área ou mais).


# Demonstração
  A inserir

# Pré Requisitos

  #### Hardware
  - Arduino Mega 2560.
  - Módulo GSM GPRS SIM800L (Necessário um chip SIM para uso pelo módulo também).
  - LCD I2C 20x4 (necessário verificar e alterar o endereço no código).
  - RTC DS1302 (E uma bateria tipo moeda de 3,3 volts, caso queira salvar o horário após o desligamento do circuito)
  - Sensor de temperatura e umidade do ar DHT11(caso vá usar outro modelo altere o ```DHTTYPE``` no código, e verifique a compatibilidade deste com o DHT11)
  - Fonte(s) de alimentação (o Arduino e o LCD operam em 5 volts, enquanto o módulo GSM opera em tensões um pouco menores, entre 3,7 volts e 4,2 volts.)
  - Regulador de tensão (Caso seja usado apenas uma fonte de alimentação.)


  #### Software
  - Arduino IDE (ou outro programa para enviar o código ao Arduino).



  #### Outros
  - Noções básicas em Eletrônica podem ser interessantes.
  - Noções do protocolo AT também podem ajudar. Sugestão de material [aqui](https://www.elecrow.com/wiki/images/2/20/SIM800_Series_AT_Command_Manual_V1.09.pdf).
  - Também é interessantes ter em mãos a documentação e especificações técnicas (datasheets) dos componentes.


Observação: O LCD e o módulo GSM podem ser substituídos pelo console do Arduino IDE, bastando adaptar as linhas de código conforme o exemplo abaixo:

``` C++ (Arduino)
// Código original

// Saída para o LCD
 lcd.setCursor(0,0);
 lcd.print("Iniciando Interface");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");


 //... Mais código aqui


 // Saída para o GSM
 Serial.println("O numero cadastrado foi alertado que a plantacao sera regada");


```

Código Adaptado (Duas possibilidades):

``` C++ (Arduino)
// Código adaptado (Versão 1)

// Saída para o LCD
 Serial.print("Iniciando Interface");
 Serial.print(".");
 Serial.print(".");
 Serial.print(".");


 //... Mais código aqui


 // Saída para o GSM
 Serial.print("O numero cadastrado foi alertado que a plantacao sera regada");

/*-------------------------------------------------------*/

// Código adaptado (Versão 2)
 /* Nesse caso você pode reaproveitar o código para usar com o hardware adequado posteriormente. */

// Saída para o LCD
 lcd.setCursor(0,0);
 lcd.print("Iniciando Interface");
 lcd.print(".");
 lcd.print(".");
 lcd.print(".");

 // Não precisa excluir o código acima
 Serial.print("Iniciando Interface");


 //... Mais código aqui


 // Saída para o GSM
 Serial.println("O numero cadastrado foi alertado que a plantacao sera regada");

 // Não precisa excluir o código acima
 Serial.print("O numero cadastrado foi alertado que a plantacao sera regada");

```


Observação 2: O Sensor de umidade do solo pode ser simulado usando um potênciômetro. O uso do sensor de temperatura e umidade do ar pode ser substituído alterando o código conforme o trecho abaixo (Usar somente uma das possibilidades de adaptação por vez):

``` C++ (Arduino)
//... Mais código aqui

void funcionamento(float Umid_Solo)
{

  // Código original
umid = dht.readHumidity();
temp = dht.readTemperature();

//... Mais código aqui

```

Código Adaptado (Duas possibilidades):

``` C++ (Arduino)
//... Mais código aqui

void funcionamento(float Umid_Solo)
{

  // Código Adaptado (Se quiser que irrigue dependendo do valor no sensor do solo)
umid = 30;
temp = dht.readTemperature();


//... Mais código aqui

/*-------------------------------------------------------*/

//... Mais código aqui

void funcionamento(float Umid_Solo)
{

  // Código Adaptado (Se quiser não irrigue por a temperatura estar muito alta)
umid = 35;
temp = dht.readTemperature();


//... Mais código aqui

```

# Tecnologias
<!-- - [Arduino Mega 2560]()
- [Arduino IDE]()
- [Módulo GSM GPRS SIM800L]()
- [LCD I2C 20x4]()
- [RTC DS1302]() -->

- [Arduino IDE](https://www.arduino.cc/en/software)


# Autores
### Johann Herrero Cavadas
[![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/jherrerocavadas/)
[![Gmail](https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:jherrerocavadas@gmail.com?Subject=Contato%20github%20-%20Repositório%20Irrigadora_Automática)
<!-- [![Github](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)]() -->

### Marcos Vinícius Silva de Souza
<!-- [![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)]() -->
<!-- [![Gmail](https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white)]() -->

### Matheus Rodrigues Salomão

### Rodrigo Gernohovski
<!-- [![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)]() -->
<!-- [![Gmail](https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white)]() -->

### Vinícius Pereira
<!-- [![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)]() -->
<!-- [![Gmail](https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white)]() -->
