
#include "Arduino.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//需要安装第三方库 ArduinoJson
#include <ArduinoJson.h>

ESP8266WebServer server(80);

// 手动修改这里连接你的路由器
String ssid     = "244"; // 需要连接的wifi热点名称  
String password = "happysoul"; // 需要连接的wifi热点密码  

// 如果默认存储的wifi和上面配置的地址无法连接则打开AP等待用户连接
String apssid      = "8266";
String appassword = "happysoul";

// nodemcu 默认板载led灯的编号是D4 对应的常量值是2
const int led = D4;
String ledStatus = "1";
String configFile = "/config.ini";

/** 
 * 根据文件后缀获取html协议的返回内容类型 
 */
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
/* NotFound处理 
 * 用于处理没有注册的请求地址 
 * 一般是处理一些页面请求 
 */
void handleNotFound() {
  String path = server.uri();
  Serial.print("url : "+path + " - ");
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz)){
      path += ".gz";
    }
    Serial.println(path);
    File file = SPIFFS.open(path, "r");
    //size_t sent = 
    server.streamFile(file, contentType);
    file.close();  
    return;
  }else{
    Serial.print("File not Found:");
    Serial.println(path);
  }
  String message = "404 File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message +=(server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for(uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
void handleIndex(){
  /* 返回信息给浏览器（状态码，Content-type， 内容） 
   * 这里是访问当前设备ip直接返回一个String 
   */
  Serial.println("/index.html");
  File file = SPIFFS.open("/index.html","r");
  //size_t sent = 
  server.streamFile(file,"text/html");
  file.close();
  return;
}

//板载led开关
void handleLed(){
  if(server.hasArg("led")){
    String a = server.arg("led");
    if(a=="0"){
      ledStatus="0";
      digitalWrite(led, HIGH);
    }else if(a=="1"){
      ledStatus="1";
      digitalWrite(led, LOW);
    }
  }
  server.send(200, "application/json", "{\"status\":\""+ledStatus+"\"}");    
}

//扫描wifi
void scanWifi(){
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    server.send(200, "application/json", "{\"num\":\""+String(n)+"\"}");
    return;
  } else {
    Serial.println(" networks found:"+n);

    //构建返回json串
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    for(int i = 1; i <= n; i++){
      //如果wifi隐藏则跳过
      if(WiFi.isHidden(i))continue;
      
      JsonObject wifi = array.createNestedObject();
      wifi["ssid"]=String(WiFi.SSID(i).c_str());
      wifi["rssi"]=String(WiFi.RSSI(i));
//      wifi["bssid"]=String(WiFi.BSSID(i));
      wifi["channel"]=String(WiFi.channel(i));
//      wifi["hidden"]=String(WiFi.isHidden(i));
      wifi["encryptionType"]=String(WiFi.encryptionType(i));
      /*
      5 : ENC_TYPE_WEP - WEP
      2 : ENC_TYPE_TKIP - WPA / PSK
      4 : ENC_TYPE_CCMP - WPA2 / PSK
      7 : ENC_TYPE_NONE - open network
      8 : ENC_TYPE_AUTO - WPA / WPA2 / PSK
      */
      
      delay(10);
    }

    //格式化json放入output
    String output;
    serializeJson(array, output);
    Serial.println(output);
    
    server.send(200, "application/json", "{\"num\":\""+String(n)+"\",\"wifis\":"+output+"}");
    return;
  }
}

// wifi网络管理，save保存文件，edit返回文件内容，remove删除文件
void configWifi(){
  if(server.hasArg("a")){
    String a = server.arg("a");
    /*
    if(a=="test"){
      StaticJsonDocument<200> doc;
      JsonArray arr = doc.to<JsonArray>();
      JsonObject wifi1 = arr.createNestedObject();
      JsonObject wifi2 = arr.createNestedObject();
      wifi1["ssid"] = "ssid1";
      wifi1["pwd"] = "ssid1_pwd";
      wifi2["ssid"] = "ssid2";
      wifi2["pwd"] = "ssid2_pwd";
      String arrString;
      serializeJson(arr, arrString);

      //返回输出
      DynamicJsonDocument result(200);
      JsonObject reObj = result.to<JsonObject>();
      reObj["result"]="true";
      reObj["msg"]=arrString;
      String output;
      serializeJson(reObj, output);
      Serial.println(output);
      
      server.send(200, "application/json", output);
      return;
    }else */
    if(a=="edit"){
      if(SPIFFS.exists(configFile)){
        File file = SPIFFS.open(configFile, "r");
        DynamicJsonDocument doc(512);
        deserializeJson(doc, file);
        JsonArray arr = doc.as<JsonArray>();
        String arrString;
        serializeJson(arr, arrString);

        DynamicJsonDocument result(200);
        JsonObject reObj = result.to<JsonObject>();
        reObj["result"]="true";
        reObj["msg"]=arrString;
        
        String output;
        serializeJson(reObj, output);
//        Serial.println(output);
        
        server.send(200, "application/json", output);
        return;
      }else{
        server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"file not found\"}");
        return;
      }
    }else if(a=="save"){
      if(server.hasArg("wifis")){
        //不知道是否需要先删文件
//        if(SPIFFS.exists(configFile)){
//          SPIFFS.remove(configFile);
//        }
        String wifis = server.arg("wifis");
        File file = SPIFFS.open(configFile, "w");
        //file.print("[{\"ssid\":\"ssid1\",\"pwd\":\"pwd1\"},{\"ssid\":\"ssid2\",\"pwd\":\"pwd2\"}]");
        file.print(wifis);
        server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"json saved to config.ini\"}");
        return;
      }else{
        server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"Empty\"}");
        return;
      }
    }else if(a=="remove"){
      if(SPIFFS.exists(configFile)){
        SPIFFS.remove(configFile);
      }
      server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"Removed\"}");
    }else{
      server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"Arg a error\"}");
      return;
    }
  }
  server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"Args miss\"}");
}

//通过ssid和密码尝试连接wifi，成功返回true，失败返回false
boolean connectWifi(String ssid,String pwd){
  Serial.println("尝试连接:"+ssid);
  int connectCount = 0;  
  WiFi.begin(ssid, pwd);  
  while(WiFi.status() != WL_CONNECTED) { 
    delay(1000);  
    Serial.print(".");
    // 重连10次
    if(connectCount > 10) {  
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
    return true;
  }else{
    return false;  
  }
}


// 重启
void restartESP(){
  Serial.println("Restart ESP!!");
  server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"OK\"}");
  delay(2000);
  ESP.restart();
}

//读取用户目录下的文件夹列表
void getUserDir(){
  DynamicJsonDocument doc(512);
  JsonObject jo = doc.to<JsonObject>();
  JsonArray array = doc.createNestedArray("files");

  //读取文件夹
  Dir dir = SPIFFS.openDir("/u");
  while (dir.next()) {
    JsonObject file = array.createNestedObject();
    File f = dir.openFile("r");
    file["name"]=String(dir.fileName());
    file["size"]=String(f.size());
    f.close();      
  }

  if(array.size()==0){
    jo["result"]="false";
    jo["msg"]="用户文件夹为空";
  }else{
    jo["result"]="true";
    jo["msg"]="读取完成";
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

//删除fs文件
void deleteFS(){
  if(server.hasArg("n")){
    String n = server.arg("n");
    if(n.length()>31 || n.length()==0){server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"删除失败,参数长度异常\"}");return;}
    if(n.indexOf("..")>-1 || n.indexOf("/u/")!=0){server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"删除失败,参数非法\"}");return;}
    if(!SPIFFS.exists(n)){server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"删除失败,文件不存在\"}");return;}
    else{SPIFFS.remove(n);server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"删除成功\"}");return;}
  }else{
    server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"删除失败,参数缺失\"}"); 
  }
}

/*
    size_t totalBytes;//整个文件系统的大小
    size_t usedBytes;//文件系统所有文件占用的大小
    size_t blockSize;//SPIFFS块大小
    size_t pageSize;//SPIFFS逻辑页数大小
    size_t maxOpenFiles;//能够同时打开的文件最大个数
    size_t maxPathLength;//文件名最大长度（包括一个字节的字符串结束符）
 */
//获取spiffs文件系统信息
void getFS(){
  DynamicJsonDocument doc(512);
  JsonObject jo = doc.to<JsonObject>();
  
  FSInfo fs;
  SPIFFS.info(fs);

  jo["result"]="true";
  jo["msg"]="文件系统信息读取完成";
  jo["totalBytes"]=String(fs.totalBytes);
  jo["usedBytes"]=String(fs.usedBytes);
  jo["blockSize"]=String(fs.blockSize);
  jo["pageSize"]=String(fs.pageSize);
  jo["maxOpenFiles"]=String(fs.maxOpenFiles);
  jo["maxPathLength"]=String(fs.maxPathLength);
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}


//上传文件
void uploadFS(){
  //上传
  File fsUploadFile;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {

    String filename = upload.filename;
    
    
    //文件名、长度等基础信息校验
    if(filename.lenght()>29 || filename.indexOf(" ")>-1){
      server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"上传失败,文件长度需要小于29,且不能有非字母数字的字符\"}");
    }
    
    
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    filename = "/u"+filename;
    Serial.print("handleFileUpload Name: ");Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
  
  server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"上传成功\"}");
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
      delay(1000);
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

  //配置web服务响应连接
  server.on("/", handleIndex);//主页
  server.on("/led", handleLed);//led灯
  server.on("/configWifi", configWifi);//配置网络
  server.on("/scanWifi", scanWifi);//扫wifi
  server.on("/getFS", getFS);//读取spiffs文件系统
  server.on("/deleteFS", deleteFS);//删除spiffs文件
  server.on("/uploadFS", uploadFS);//删除spiffs文件
  server.on("/getUserDir", getUserDir);//读取spiffs用户文件
  server.on("/restartESP", restartESP);//重启esp8266
  
  server.onNotFound(handleNotFound); // NotFound处理，图片、js、css等也会走这里
  server.begin();
  Serial.println("HTTP server started");
}

void loop(){
 // 循环处理,因为ESP8266的自带的中断已经被系统占用,只能用过循环的方式来处理网络请求
 server.handleClient();
}
