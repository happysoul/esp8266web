#include "Arduino.h"
#include <FS.h>
#include <LittleFS.h>
//#include <WiFiClient.h>
//OTA升级用
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//需要安装第三方库 ArduinoJson
#include <ArduinoJson.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h>
  //web服务器使用80端口
  ESP8266WebServer server(80);
#endif

#ifdef ESP32
  #include <WiFi.h>
  #include <esp_wifi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  //web服务器使用80端口
  WebServer server(80);
#endif

//读取分区失败的时候是否格式化，默认false，这里也使用false，因为必须有文件才能使用
//ESP8266无此参数，只适用于ESP32
#define FORMAT_LITTLEFS_IF_FAILED false

// 默认机器名字
const char* host = "ESP8266";

// 手动修改这里连接你的路由器
// 把234换成你路由器wifi的名字，密码是你路由器wifi的密码
String ssid     = "234"; // 需要连接的wifi热点名称  
String password = "happysoul"; // 需要连接的wifi热点密码  

// 如果默认存储的wifi和上面配置的地址无法连接则打开AP等待用户连接
// 这个是8266开AP之后你手机或电脑可以搜到ESP，密码是happysoul
String apssid      = "8266";
String appassword  = "happysoul";

// nodemcu 默认板载led灯的编号是D4 对应的常量值是2
// 其他板子的LED可能有4
const int led = 2;
String ledStatus = "1";//LED默认开
String configFile = "/config.ini";//配置文件的文件名

// 是否开启OTA升级 默认false不开启
boolean otaFlag = false;

// 上传文件用
File fsUploadFile;


//通过ssid和密码尝试连接wifi，成功返回true，失败返回false
boolean connectWifi(String ssid,String pwd){
  Serial.println("尝试连接:"+ssid);
  int connectCount = 0;  
  WiFi.begin(ssid.c_str(), pwd.c_str());
  delay(500);
  while(WiFi.status() != WL_CONNECTED) { 
    //如果初始化连接不上可以适当增加延时时间
    delay(800);
    Serial.print(".");
    // 重连10次
    if(connectCount > 5) {
      Serial.println("Connect fail!");
      break;
    }
    connectCount += 1;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    connectCount = 0;
    WiFi.setAutoReconnect(true);
    MDNS.begin(host);
    return true;
  }else{
    return false;  
  }
}

// 主程序入口
void setup() {
  delay(1000);
  pinMode(led, OUTPUT);
  Serial.begin(115200);
//  SPIFFS.begin();
  //输出空行
  Serial.println("");
  Serial.println("begin");
  
  // 初始化网络
  WiFi.mode(WIFI_STA);

#ifdef ESP32  
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
#endif

#ifdef ESP8266
  if(!LittleFS.begin()){
    Serial.println("LittleFS Mount Failed");
    return;
  }
#endif

  //连接状态 0未连接 1已连接
  int connect_status = 0;
  // 读取配置文件
  if(LittleFS.exists(configFile)){
    File file = LittleFS.open(configFile, "r");
    String data = file.readString();
    Serial.print("configFile:");
    Serial.println(data);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonArray arr = doc.as<JsonArray>();
    Serial.print("JsonSize:");
    Serial.println(arr.size());
    for(unsigned int i=0;i<arr.size();i++){
      if(connectWifi(arr[i]["ssid"],arr[i]["pwd"])){
        connect_status = 1;
        break;
      }
    }
    //最后关闭文件
    file.close();
  }else{
    Serial.println("File not found:"+configFile);
  }

  //未连接wifi的时候使用内置ssid和密码尝试连接，如果仍无法连接到wifi则开启AP
  if(connect_status==0){
    Serial.println("使用内置连接尝试");
    if(connectWifi(ssid.c_str(), password.c_str())){
      connect_status = 1;
    }else{
      Serial.println("使用内置连接失败,开启AP等待用户连接");
      Serial.println("SSID:"+apssid);
      Serial.println("PASSWORD:"+appassword);
      WiFi.mode(WIFI_AP);
      //WIFI_AP_STA模式不稳定，不建议使用
//      WiFi.mode(WIFI_AP_STA);

      delay(600);
      /*
      ssid:热点名,最大63个英文字符
      password:可选参数,可以没有密码
      channel:信道1-13,默认1
      hidden:是否隐藏,true是隐藏
      WiFi.softAP(ssid, password, channel, hidden);

      配置网关等
      IPAddress local_IP(192,168,4,4);
      IPAddress gateway(192,168,4,1);  
      IPAddress subnet(255,255,255,0);
      WiFi.softAPConfig(local_IP, gateway, subnet);
      */

	  //打开AP模式 默认地址是 192.168.4.1
      boolean ap_status = WiFi.softAP(apssid.c_str(), appassword.c_str());
      if(ap_status){
        Serial.println("AP Ready!!");
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP : ");
        Serial.println(myIP);  
      }else{
        Serial.println("Fail!!");  
      }
    }
  }

  // 配置web请求
  configweb();
  
//  MDNS.addService("http", "tcp", 80);
//  Serial.printf("Ready! Open http://%s.local in your browser\n", host);

  //是否开启ota升级功能
  if(otaFlag){
    Serial.println("Open OTA start");
    //方法在ota.ino中
    openOTA();
    Serial.println("Open OTA started");
  }
}

void loop(){
  // 循环处理,因为ESP自带的中断已经被系统占用,只能用过循环的方式来处理网络请求
  server.handleClient();
  if(otaFlag){
    // OTA升级功能
    ArduinoOTA.handle();
  }
//  MDNS.update();
}
