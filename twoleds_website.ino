#include <WiFi.h>

const char* ssid = "Galaxy M21DE4B";
const char* password = "njpc4983";

WiFiServer server(80);

const int led1 = 27;
const int led2 = 26;

void setup()
{
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status()!=WL_CONNECTED)
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

    if(request.indexOf("/LED1ON")!=-1)
      digitalWrite(led1,HIGH);

    if(request.indexOf("/LED1OFF")!=-1)
      digitalWrite(led1,LOW);

    if(request.indexOf("/LED2ON")!=-1)
      digitalWrite(led2,HIGH);

    if(request.indexOf("/LED2OFF")!=-1)
      digitalWrite(led2,LOW);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type:text/html");
    client.println();

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>

<body style="text-align:center; margin-top:80px">

<h1>ESP32 Two LED Controller</h1>

<a href="/LED1ON">
<button style="padding:20px">LED1 ON</button>
</a>

<a href="/LED1OFF">
<button style="padding:20px">LED1 OFF</button>
</a>

<br><br>

<a href="/LED2ON">
<button style="padding:20px">LED2 ON</button>
</a>

<a href="/LED2OFF">
<button style="padding:20px">LED2 OFF</button>
</a>

</body>
</html>
)rawliteral");

    client.stop();
  }
}