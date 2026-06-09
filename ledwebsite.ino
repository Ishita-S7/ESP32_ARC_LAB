#include <WiFi.h>

const char* ssid = "Galaxy M21DE4B";
const char* password = "njpc4983";

WiFiServer server(80);

const int ledPin = 18;

void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  WiFiClient client = server.available();

  if(client)
  {
    String request = client.readStringUntil('\r');

    Serial.println(request);

    if(request.indexOf("/ON") != -1)
      digitalWrite(ledPin, HIGH);

    if(request.indexOf("/OFF") != -1)
      digitalWrite(ledPin, LOW);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type:text/html");
    client.println();

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 LED Control</title>
</head>

<body style="text-align:center; margin-top:100px">

<h1>ESP32 LED Controller</h1>

<a href="/ON">
<button style="padding:20px;font-size:20px">
LED ON
</button>
</a>

<br><br>

<a href="/OFF">
<button style="padding:20px;font-size:20px">
LED OFF
</button>
</a>

</body>
</html>
)rawliteral");

    client.stop();
  }
}
