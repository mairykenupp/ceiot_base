#include <ESPDateTime.h>
#include <time.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SEALEVELPRESSURE_HPA (1013.25) //Pressão nível do mar

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp; // I2C

float temperature, humidity, pressure, altitude;

/*Conexão WiFI*/


const char* ssid = "KNPP";
const char* password = "ZLWZ1716";

unsigned long lastMs = 0;
unsigned long ms = millis();

const char* ntpServer = "br.pool.ntp.org"; //NTP Server para o Brasil
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

WebServer server(80); 
          
void setup() {
  Serial.begin(115200);
  delay(100);

  bmp.begin(0x76);   

  Serial.println("Conectando a ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Conectado!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("Servidor HTTP Iniciado");
}

void setupDateTime() {
   //setup this after wifi connected
   //you can use custom timeZone,server and timeout
   //DateTime.setTimeZone(-4);
     //DateTime.setServer("br.pool.ntp.org");
     //DateTime.begin(15 * 1000);
  //DateTime.setServer("br.pool.ntp.org"); //servidor de data e hora do Observatório Nacional(ON) do Brasil
  DateTime.setServer("south-america.pool.ntp.org"); //servidor de data e hora para a América do Sul
  DateTime.setTimeZone("UTC-3"); //TimeZone do Brasil
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  } else {
    Serial.printf(DateTime.toISOString().c_str());
    Serial.printf("Timestamp is %ld\n", DateTime.now());
  }
  }

//void printLocalTime()
//{
  //struct tm timeinfo;
  //if(!getLocalTime(&timeinfo)){
    //Serial.println("Failed to obtain time");
    //return;
  //}
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//}

void loop() {
  server.handleClient();
  
  temperature = bmp.readTemperature();
//  humidity = bmp.readHumidity();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude();
    
    Serial.print(F("Temperatura = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressao Atmosferica = "));
    Serial.print(bmp.readPressure());
    Serial.println(" hPa");

    Serial.print(F("Altitude = "));
    Serial.print(bmp.readAltitude()); /* Altitude */
    Serial.println(" m");

     DateTimeParts p = DateTime.getParts();
     Serial.printf("%04d/%02d/%02d %02d:%02d:%02d %ld (%s)\n", p.getYear(),
                p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(),
                p.getSeconds(), p.getTime(), p.getTimeZone());
   //printLocalTime();
  Serial.println("--------------------");

  long rnd = random(1,10);
  HTTPClient client;

  client.begin("http://jsonplaceholder.typicode.com/comments?id=" + String(rnd));
  int httpCode = client.GET();

  if (httpCode > 0) {
    String payload = client.getString();
    Serial.println("\Statuscode: " + String(httpCode));
    Serial.println(payload);

    char json[500];
    payload.replace(" ","");
    payload.replace("\n", "");
    payload.trim();
    payload.remove(0,1);
    payload.toCharArray(json, 500);

    StaticJsonDocument<200> doc;
    deserializeJson(doc, json);

    int id = doc["id"];
    const char* email = doc["email"];
  }
  else {
    Serial.println("Erro ao requisitar HTTP");
  }
  //StaticJsonBuffer<200> jsonBuffer; //arduinojson versao 5
  //StaticJsonDocument<200> doc; 
  //char json[] = "{\"temperature\":\"pressure\":\"altitude}";
  //JsonObject& root = json.Buffer.parseObject(json); //arduinojson versao 5
  //JsonObject root = deserializeJson(doc, json);
  //double temperature = root["data"][0];
 // double pressure = root["data"][1];
  //double altitude = root["data"][2];

  //Serial.println(root);


//print timestamp com base no void setupDateTime()
// if (!DateTime.isTimeValid()) {
//    Serial.println("Failed to get time from server.");
 // } else {
    //Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
    //Serial.printf("Timestamp is %ld\n", DateTime.now());
//}
    Serial.println();
    delay(2000);
  server.send(200, "text/html", SendHTML(temperature,pressure,altitude)); 
  
        
   // char json[500];
    //s.replace(" ", "");
   // s.replace("\n", "");
   // s.trim();
   // s.remove(0,1);
   // s.toCharArray(json,500);

  }

void handle_OnConnect() {
  temperature = bmp.readTemperature();
//  humidity = bmp.readHumidity();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  server.send(200, "text/html", SendHTML(temperature,pressure,altitude)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float pressure,float altitude){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<meta http-equiv='refresh' content='7'/>";
  ptr +="<title>Sensor BMP280</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".temperature .reading{color: #F29C1F;}";
  ptr +=".humidity .reading{color: #3B97D3;}";
  ptr +=".pressure .reading{color: #26B99A;}";
  ptr +=".altitude .reading{color: #955BA5;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>Leituras Sensor BMP280</h1>";
  ptr +="<h4>Adaptado por Mairy Kenupp para a classe de IIoT da Universidade de Buenos Aires</h4>"; //Mairy Kenupp_MK
  ptr +="<h2>Brasilia, Brasil</h2>"; //Local_MK
// ptr +="<h3 id='current-time'></h3>"; //Mostra data e horavia script_MK
  ptr +="<div class='container'>";
  ptr +="<div class='data temperature'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Temperatura</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=temperature;
  ptr +="<span class='superscript'>&deg;C</span></div>";
  ptr +="</div>";
  ptr +="<div class='data humidity'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Pressao<br>Atmosferica</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=pressure;
  ptr +="<span class='superscript'>hPa</span></div>";
  ptr +="</div>";
  ptr +="<div class='data altitude'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902";
  ptr +="c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004";
  ptr +="c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994";
  ptr +="C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0";
  ptr +="c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004";
  ptr +="C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813";
  ptr +="C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Altitude</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=altitude;
  ptr +="<span class='superscript'>m</span></div>";
  ptr +="</div>";
  ptr +="</div>";
//  ptr +="<script>
//    let time = document.getElementById('current-time');
//    setInterval(() =>{
//      let d = new Date();
//      time.innerHTML = d.toLocaleTimeString();
//      },1000) //script para hora e data locais
//  </script>"; //tentative de script para data e hora_MK
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}