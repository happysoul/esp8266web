# esp8266web
## 基于ESP8266

基于NodeMCU 1.0
flash size:4M (1M spiffs)

功能：
默认连接无法连到wifi的时候会自动开启AP（名字：8266，密码：happysoul）
能开关板子上的LED
能扫描wifi
开机自动连接wifi的配置
重启8266的按钮
增加文件管理，未实现文件上传、删除、显示spiffs信息的功能



依赖：
ArduinoJSON（项目-加载库-搜索ArduinoJSON并安装）

需要下载并放到ArduinoIDE中的文件
https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.4.0/ESP8266FS-0.4.0.zip

将里面的jar放到arduino的目录下，重启arduino，工具菜单下就能看到ESP8266 Sketch Data Upload功能
arduino-1.8.9\tools\ESP8266FS\tool\esp8266fs.jar
点这个功能是将data目录下的文件传到arduino的spiffs目录中
首先是你要用ESP8266的板子，另外选择了spiffs空间