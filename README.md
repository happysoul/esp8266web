# esp8266web

## 使用 ESP8266开发板，基于Arduino开发
- 我使用的 NodeMCU 1.0 
- flash size:4M (1M spiffs)

## 功能：
- 1、通电后根据保存的多组wifi密码尝试连接，尝试失败后再尝试内置密码连接，如果依旧无法连接wifi则开启AP（名字：8266，密码：happysoul，192.168.4.1）
- 2、网页功能：基础测试功能、wifi密码配置、spiffs文件（/u/目录下）<br>
	开关板载LED，读写JSON文件
	扫描wifi，暂未实现点击连wifi的功能<br>
	重启8266的按钮<br>
	显示spiffs信息，显示文件列表，删除文件
- 3、OTA升级：默认打开了OTA升级功能，也就是说程序烧到板子以后默认打开了网络升级功能，不需要串口了，随便找个5V或者3.3V的供电就可以了。<br>
	只需要电脑和8266在同一个网络下，重启arduino IDE 在选择com口的位置就能看到8266的ip可以选择。

## TODO	计划增加功能
- 1、完善扫描wifi后保存连接的功能
- 2、完善spiffs文件管理功能：文件上传

## 依赖：
- 1、ESP8266开发板支持<br>
	工具 - 开发板 - 开发板管理器 搜索esp8266并安装，如果没有这个选择先配网址<br>
	文件 - 首选项，开发版管理网址输入：https://arduino.esp8266.com/stable/package_esp8266com_index.json 确定后在上一步位置再搜索esp8266<br>
	安装大约需要下载100M-200M的文件<br>
- 2、[ArduinoJSON](https://github.com/bblanchon/ArduinoJson)（项目-加载库-搜索ArduinoJSON并安装）
- 3、[ESP8266FS](https://github.com/esp8266/arduino-esp8266fs-plugin)  / [下载文件](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.4.0/ESP8266FS-0.4.0.zip)<br>
	解压缩文件 arduino-1.8.9\tools\ESP8266FS\tool\esp8266fs.jar<br>
	重启arduino，工具菜单下就能看到ESP8266 Sketch Data Upload功能<br>
	
## 使用步骤
- 1、确保依赖已经安装
- 2、选择开发板及flash size<br>
	某宝买的带有mini usb接口的一般就选择 NodeMCU 1.0，flash size:4M (1M spiffs) 也就是板载32Mbit，分出来8Mbit空间给spiffs使用<br>
	如果你是 ESP8266 8针脚的，那么你板载flash就是 8Mbit 也就是使用的时候需要选择 1M(512k spiffs)<br>
- 3、编译上传
- 4、工具 - ESP8266 Sketch Data Upload<br>
	这个步骤做的是将 data 文件夹下的文件复制到 esp8266 的spiffs空间中<br>
	如果只修改了 data 文件夹中的内容不需要上传 ino 程序，只需要上传文件即可<br>
	同理，如果只修改了 ino 文件，则不需要重复上传 data 的文件<br>
