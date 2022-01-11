
// 配置web请求及响应function
void configweb(){
  //配置web服务响应连接
  server.on("/", handleIndex);//主页
  server.on("/led", handleLed);//led灯
  server.on("/configWifi", configWifi);//配置网络
  server.on("/scanWifi", scanWifi);//扫wifi
  server.on("/getFS", getFS);//读取spiffs文件系统
  server.on("/deleteFS", deleteFS);//删除spiffs文件
  server.on("/uploadFS",HTTP_POST,[](){ server.send(200); },uploadFS);//上传文件
  server.on("/getUserDir", getUserDir);//读取spiffs用户文件
  server.on("/restartESP", restartESP);//重启esp8266

  //用户自定义按钮（2个参数，前面是html中的js代码访问的地址，后面是function方法，在上面）
  //对应web请求地址是 http://192.168.4.1/userButton1 
  //AP模式地址是 192.168.4.1，客户端模式是你实际分配到的ip
  server.on("/userButton1", userButton1);
  //如果还有其他请求写在这里
  
  server.onNotFound(handleNotFound); // NotFound处理，图片、js、css等也会走这里
  //启动web服务
  server.begin();
  Serial.println("HTTP server started");  
}


// 新加功能代码样例
// ----------------------------------------------------------------------------------------
// 用户自定义按钮userButton1对应事件
void userButton1(){
  Serial.println("userButton1!!");

  //中间位置写你要执行的事件，开关灯或者调用显示屏等功能
  
  //自定义返回值，我这里定义result为成功失败，meg用于显示处理结果
  server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"OK\"}");
  //根据前面执行结果使用if else 输出不同的返回结果，如true，false
  //server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"fail\"}");
}
// ----------------------------------------------------------------------------------------


/** 
 * 根据文件后缀获取html协议的返回内容类型 
 */
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  //下载流,此类型浏览器会弹出下载  
  else if(filename.startsWith("/u/")) return "application/octet-stream";
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
  // 判断文件是否存在
  if(SPIFFS.exists("/index.html")){
    File file = SPIFFS.open("/index.html","r");
    server.streamFile(file,"text/html");
    file.close();
  }else{
    server.send(200, "text/plain", "未发现index.html文件,请使用arduino上传data文件,参考https://gitee.com/happysoul/esp8266web");
  }
  return;
}


//板载led开关
void handleLed(){
  //判断请求中是否有led这个参数
  if(server.hasArg("led")){
    String a = server.arg("led");
    // 根据页面传递的0 1处理开关灯
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
      //如果wifi隐藏ssid则跳过
      if(WiFi.isHidden(i))continue;
      
      JsonObject wifi = array.createNestedObject();
      wifi["ssid"]=String(WiFi.SSID(i).c_str());
      wifi["rssi"]=String(WiFi.RSSI(i));
//      wifi["bssid"]=String(WiFi.BSSID(i));
      wifi["bssidstr"]=String(WiFi.BSSIDstr(i));
      wifi["channel"]=String(WiFi.channel(i));
      //是否是隐藏的ssid
//      wifi["hidden"]=String(WiFi.isHidden(i));
      wifi["encryptionType"]=String(WiFi.encryptionType(i));
      /*
      扫描代码后返回值对应的加密类型
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
  HTTPUpload& upload = server.upload();
  Serial.print("-- upload status:");
  Serial.println(upload.status);
  
  if (upload.status == UPLOAD_FILE_START) {

    String filename = upload.filename;
    Serial.print("1.FileName: ");
    Serial.println(filename);
    
    //文件名、长度等基础信息校验
    if(filename.length()>29 || filename.indexOf(" ")>-1){
      server.send(200, "application/json", "{\"result\":\"false\",\"msg\":\"上传失败,文件长度需要小于29,且不能有非字母数字的字符\"}");
      return;
    }
    
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    filename = "/u"+filename;
    // 重新设置名字
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    int currentSize = upload.currentSize;
    Serial.print("2.Data size: "); 
    Serial.println(currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.println("3.To be close"); 
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("3.Totle Size: ");
    Serial.println(upload.totalSize);

    //上传完成，返回成功
    server.send(200, "application/json", "{\"result\":\"true\",\"msg\":\"上传完成\"}");
  } else{
    Serial.println("4.end");
  }
  
}
