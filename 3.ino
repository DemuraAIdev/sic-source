// include wire library
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <DHT.h>
// #include "ptches.h"
// WIfi Secour client
#include <WiFiClientSecure.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 4
#define DHTTYPE DHT11

const char *ssid = "THETA";
const char *password = "11223344";

const char *mqtt_server = "0894e758ae594242b41480fb24e2f7de.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
unsigned long lastMsg = 0;
const char *mqtt_user = "cyberai";
const char *mqtt_password = "Min2kota";

float h;
float t;

const int buzzerPin = 5;

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure espClient;
PubSubClient client(espClient);

// HiveMQ Cloud Let's Encrypt CA certificate
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void setup()
{
  Serial.begin(115200);
  delay(1000);
  WiFi.begin(ssid, password);

  //  OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display();

  // small tone for startup sound
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);

  digitalWrite(buzzerPin, LOW);
  delay(500);
  digitalWrite(buzzerPin, HIGH);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  dht.begin();

  // cert
  espClient.setCACert(root_ca);

  // connect to mqtt server
  client.setServer(mqtt_server, mqtt_port);

  // connect to mqtt server
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("cyberai", mqtt_user, mqtt_password))
    {
      Serial.println("connected");

      client.publish("esp32/sensor", "hello world");
      client.subscribe("esp32/sensor");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void reconnect()
{
  // set buzzer to low
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("cyberai", mqtt_user, mqtt_password))
    {
      Serial.println("connected");

      client.publish("esp32/sensor", "hello world");
      client.subscribe("esp32/sensor");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  unsigned long now = millis();

  if (now - lastMsg > 1000)
  {
    lastMsg = now;

    h = dht.readHumidity();
    t = dht.readTemperature();
    // analog read mq135
    int sensorValueMQ = analogRead(34);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print("%  Temperature: ");
    Serial.print(t);
    Serial.print(" C ");

    Serial.print(F("MQ135: "));
    Serial.println(sensorValueMQ);

    // publish to mqtt to one topic only , and value is json
    char msg[100];
    snprintf(msg, 75, "{\"temperature\": %f, \"humidity\": %f, \"mq135\": %d}", t, h, sensorValueMQ);
    client.publish("esp32/sensor", msg);

    if (t > 35.00)
    {
      digitalWrite(buzzerPin, LOW);
    }
    else
    {
      digitalWrite(buzzerPin, HIGH);
    }

    client.loop();
  }

  if (now % 10000 == 0)
  {
    h = dht.readHumidity();
    t = dht.readTemperature();
    int sensorValueMQ = analogRead(34);
    char msg[100];
    snprintf(msg, 75, "{\"temperature\": %f, \"humidity\": %f, \"mq135\": %d}", t, h, sensorValueMQ);
    client.publish("esp32/sensor/train", msg);
  }
}

// reconnect to mqtt server
