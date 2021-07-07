#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define sensorLDR D0
#define sensorSuhu A0
#define Motor1 D5
#define Motor2 D6
#define Motor3 D7
#define Servo D8
#define MSG_BUFFER_SIZE (50)

const char *ssid = "SemarAgung";//silakan disesuaikan sendiri
const char *password = "ayamgoreng";//silakan disesuaikan sendiri
const char *mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
SimpleDHT22 dht22(D1);

int nilaiSensorsuhu;
int nilaiSensor;
char msg[MSG_BUFFER_SIZE];
long now = millis();
long lastMeasure = 0;
int value = 0;
unsigned long lastMsg = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe("UAS/@S1");
      client.subscribe("UAS/@S2");
      client.subscribe("UAS/@S3");

    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void cepat()
{
  digitalWrite(Motor3, HIGH);
  digitalWrite(Motor2, HIGH);
  digitalWrite(Motor1, LOW);
}

void sedang()
{
  digitalWrite(Motor1, HIGH);
  digitalWrite(Motor2, HIGH);
  digitalWrite(Motor3, LOW);
}

void lambat()
{
  digitalWrite(Motor2, HIGH);
  digitalWrite(Motor1, HIGH);
  digitalWrite(Motor3, LOW);
}

void servoOff()
{
  digitalWrite(Motor2, HIGH);
  digitalWrite(Motor1, HIGH);
  digitalWrite(Motor3, HIGH);
}

void servo()
{
  digitalWrite(Servo, LOW);
}
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if (String(topic) == "UAS/@S1")
  {
    if (messageTemp == "true")
    {
      servoOff();
      cepat();
    }
    else
    {
      servoOff();
    }
  }
  else if (String(topic) == "UAS/@S2")
  {
    if (messageTemp == "true")
    {
      servoOff();
      sedang();
    }
    else
    {
      servoOff();
    }
  }
  else
  {
    if (messageTemp == "true")
    {
      servoOff();
      lambat();
    }
    else
    {
      servoOff();
    }
  }

  if (String(topic) == "UAS/@Arah")
  {
    if (messageTemp == "true")
    {
      servo();
    }
  }
  
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  servoOff();
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2, OUTPUT);
  pinMode(Motor3, OUTPUT);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP8266Client");
  }
  now = millis();
  if (now - lastMeasure > 2000)
  {
    lastMeasure = now;
    int err = SimpleDHTErrSuccess;
    nilaiSensor = analogRead(sensorLDR);
    nilaiSensorsuhu = analogRead(sensorSuhu);
    nilaiSensorsuhu = map(nilaiSensorsuhu, 1023, 0, 0, 100);
    byte temperature = 0;
    byte humidity = 0;
    if ((err = dht22.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      Serial.print("Pembacaan DHT22 gagal, err=");
      Serial.println(err);
      delay(1000);
      return;
    }
    static char temperatureTemp[7];
    dtostrf(temperature, 4, 2, temperatureTemp);
    Serial.println(temperatureTemp);
    client.publish("UAS/@Suhu", temperatureTemp);
    

    static char ldrScore[7];
    dtostrf(nilaiSensor, 4, 2, ldrScore);
    Serial.println(ldrScore);
    client.publish("UAS/@Cahaya", ldrScore);
  }
}