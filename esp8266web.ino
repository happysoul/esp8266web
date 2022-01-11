#include "Arduino.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//OTA升级用
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//需要安装第三方库 ArduinoJson
#include <ArduinoJson.h>

//web服务器使用80端口
ESP8266WebServer server(80);

// 默认机器名字
const char* host = "8266";

// 手动修改这里连接你的路由器
// 把234换成你路由器wifi的名字，密码是你路由器wifi的密码
String ssid     = "234"; // 需要连接的wifi热点名称  
String password = "happysoul"; // 需要连接的wifi热点密码  

// 如果默认存储的wifi和上面配置的地址无法连接则打开AP等待用户连接
// 这个是8266开AP之后你手机或电脑可以搜到8266，密码是happysoul
String apssid      = "8266";
String appassword = "happysoul";

// nodemcu 默认板载led灯的编号是D4 对应的常量值是2,12F等板子可能是4
const int led = 2;
String ledStatus = "1";
String configFile = "/config.ini";

// 是否开启OTA升级
boolean otaFlag = true;

// 上传文件用
File fsUploadFile;


//通过ssid和密码尝试连接wifi，成功返回true，失败返回false
boolean connectWifi(String ssid,String pwd){
  Serial.println("尝试连接:"+ssid);
  int connectCount = 0;  
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);  
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
  // 初始化网络  
  WiFi.mode(WIFI_STA);
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  SPIFFS.begin();
  //输出空行
  Serial.println("");

  //连接状态 0未连接 1已连接
  int connect_status = 0;
  // 读取配置文件
  if(SPIFFS.exists(configFile)){
    File file = SPIFFS.open(configFile, "r");
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
      delay(500);
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
      boolean ap_status = WiFi.softAP(apssid, appassword);
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
  // 循环处理,因为ESP8266的自带的中断已经被系统占用,只能用过循环的方式来处理网络请求
  server.handleClient();
  if(otaFlag){
    // 增加升级功能
    ArduinoOTA.handle();
  }
//  MDNS.update();
}
