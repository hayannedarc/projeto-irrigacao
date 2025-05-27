//Projeto de Irrigação Automatizada e Monitoramento do Solo via MQTT
//Autora  : Hayanne Darc Castilho
//Projeto para a disciplina de Objetos Inteligentes e Conectados da Universidade Persbiteriana Mackenzie

//Incluindo as Bibliotecas da Placa NodeMCU ESP8266 para estabelecer conexão via WiFi
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Definindo as variáveis e os pinos e entradas correspondentes de cada componente
#define DEBUG
#define pino_sinal_analogico A0

#define pino_led_vermelho 13
#define pino_led_verde 12
#define pino_bomba 14

//Variáveis
int valor_analogico;
String strMSG = "0";
char mensagem[30];

//Informações da Rede WIFI para conexão com o HiveMQ, que estabelece conectividade com o Broker que fará o monitoramento dos Solos
const char* ssid = "Hayanne";           //SSID da rede WIFI
const char* password =  "D@rc2961";    //senha da rede wifi
//Informações da Instância do broker MQTT
const char* mqttServer = "2fc9a9ccd211409690351ea7a10f9862.s1.eu.hivemq.cloud"; //server
const char* mqttUser = "Hayanne";        //user
const char* mqttPassword = "Ga190511";  //password
const int mqttPort = 8883;   //port
const char* mqttTopicSub = "hay123/irrigacao";//tópico que será assinado no Broker

WiFiClientSecure espClient;
PubSubClient client(espClient);

//Definindo as entradas correspondentes aos pinos e os Prints de Resposta para o monitor serial
void setup() {
  Serial.begin(9600);
  pinMode(pino_sinal_analogico, INPUT);
  pinMode(pino_led_vermelho, OUTPUT);
  pinMode(pino_led_verde, OUTPUT);
  pinMode(pino_bomba, OUTPUT);

  WiFi.begin(ssid, password);
  espClient.setInsecure(); // dispensa o uso de certificado para autenticação no HiveMQ
 
  Serial.print("Entrando no Setup");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG
    Serial.println("Aguarde! Conectando ao WiFi...");
#endif
  }
#ifdef DEBUG
  Serial.println("Conectado na rede WiFi com sucesso!");
#endif

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
#ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
#endif

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
#ifdef DEBUG
      Serial.println("Conectado com sucesso");
#endif

    } else {
#ifdef DEBUG
      Serial.print("falha estado  ");
      Serial.print(client.state());
#endif
      delay(2000);
    }
  }

  client.subscribe(mqttTopicSub);

}

void loop() {

  if (!client.connected()) {
    Serial.print("Passou no Connected");
    reconect();

  }
 
  Serial.print(valor_analogico);
  valor_analogico = analogRead(pino_sinal_analogico);

  Serial.print("Porta analogica: ");
  Serial.print(valor_analogico);

  //Definindo o parâmetro para um Solo devidamente úmido, acendendo o LED VERDE
    if (valor_analogico >= 0 && valor_analogico < 900)
    {
      Serial.println("Status: Solo umido");
    
      digitalWrite (pino_led_vermelho, LOW);
      digitalWrite (pino_led_verde, HIGH);
      digitalWrite (pino_bomba, HIGH);
  
      sprintf(mensagem, "1");
      Serial.print("Mensagem enviada: ");
      Serial.println(mensagem);
      client.publish("hay123/irrigacao", mensagem);
      Serial.println("Mensagem enviada com sucesso...");
  
    }

 
  //Definindo o parâmetro para um Solo Seco, acendendo o LED VERMELHO
    if (valor_analogico >= 900 && valor_analogico <= 1024)
    {
      Serial.println(" Status: Solo seco");
      
      digitalWrite (pino_led_verde, LOW);
      digitalWrite (pino_led_vermelho, HIGH);
      digitalWrite (pino_bomba, LOW);
  
      sprintf(mensagem, "0");
      Serial.print("Mensagem enviada: ");
      Serial.println(mensagem); 
      client.publish("hay123/irrigacao", mensagem);
      Serial.println("Mensagem enviada com sucesso...");
  
    }
  delay(2000);

  client.loop();

}

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';
  strMSG = String((char*)payload);

#ifdef DEBUG
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("-----------------------");
#endif

}

//Função pra reconectar ao servidor MQTT
void reconect() {
  while (!client.connected()) {
#ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
#endif

    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client.connect("ESP8266Client");

    if (conectado) {
#ifdef DEBUG
      Serial.println("Conectado!");
#endif
      client.subscribe(mqttTopicSub, 1); //nivel de qualidade: QoS 1
    } else {
#ifdef DEBUG
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client.state()).c_str());
      Serial.println("Tentando novamente em 5s");
#endif
      delay(5000);
    }
  }
}
