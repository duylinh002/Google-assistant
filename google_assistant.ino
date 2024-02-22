#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
// thư viện mqtt
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
// thư viện gửi mail
#include <EMailSender.h>
#define WIFI_SSID "Wifi Cui Bap"
#define WIFI_PASSWORD "wifichua2021A"
//define relay pin
const int relay = 5;

// cổng mqtt đã setup trên adfruit.com
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "idragon1st"
#define MQTT_PASS "aio_shFK08LpVN78C3FYJBFvHY6gqLGR"

//Set up MQTT and WiFi clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//khai báo kênh esp sẽ subcribe vào để nhận lệnh
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/onoff");

uint8_t connection_state = 0;
uint16_t reconnect_interval = 10000;
//đăng nhập email gửi
EMailSender emailSend("clonedarkness1@gmail.com", "Yuichan1");

//chân cảm biếng quang học
#define SENSOR_PIN A0

bool sent = false;
int L = 0;
void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //subcribe kênh onoff
  mqtt.subscribe(&onoff);
  
  pinMode(relay, OUTPUT);
  
}

void loop(){
  //kết nối mqtt, kiểm tra tín hiệu gửi về nếu là on thì bật đèn, ngược lại
  MQTT_connect();
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    if (subscription == &onoff)
    {
      //Print the new value to the serial monitor
      Serial.print("onoff: ");
      Serial.println((char*) onoff.lastread);
     //If the new value is  "ON", turn the light on.
     //Otherwise, turn it off.
      if (!strcmp((char*) onoff.lastread, "ON"))
      {
        digitalWrite(relay, HIGH);
        Serial.print("ON");
      }
      else
      {
        Serial.print("OFF");
        digitalWrite(relay, LOW);
      }
    }
  }
  // ping server mqtt để giữ cho kết nối luôn hoạt động
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
  //đọc dữ liệu từ cảm biến quang
  L = analogRead(SENSOR_PIN);
  Serial.println(L);
  //nếu dữ liệu trả về bằng 1024 == đèn trên cảm biếng quang ngắt == trời mưa, ngược lại nếu giá trị khác thì 
  //cảm biếng quang vẫn đang nhận được ánh sáng == trời không mưa
  //biến sent để kiểm tra mail đã gửi chưa, đảm bảo chỉ gửi 1 lần khi trời có mưa, tránh spam
  if(L == 1024){
      Serial.println("Raining,...");
      if (sent == false) {
        sent = true;
        sendmail();
     } 
   }
   else
   {
     sent = false;
     Serial.println("No rain,....");
   }
  delay(1000);
}

//hàm gửi mail, tới mail s2kirbys2 với nội dung đang có mưa, resp.status sẽ trả về mail đã được gửi thành công hay chưa
void sendmail() {
    EMailSender::EMailMessage message;
    message.subject = "ESP8266 Rain Notify";
    message.message = "Đang có mưa";
    EMailSender::Response resp = emailSend.send("s2kirbys2@gmail.com", message);
    Serial.println("Sending status: ");
    Serial.println(resp.status);
    Serial.println(resp.code);
    Serial.println(resp.desc);
}
//hàm connect mqtt
void MQTT_connect()
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0)
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
} //END CODE
