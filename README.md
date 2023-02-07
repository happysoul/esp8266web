# esp8266web

## 使用 ESP8266开发板，基于Arduino开发
- 我使用的 NodeMCU 1.0 
- flash size:4M (FS:2MB OTA:~1019KB) 只要不选FS:none就行

2023-01-09 修改LittleFS，代码适配ESP32（无法显示文件系统的信息，因ESP32的arduino库没有实现这部分功能）

## 文件结构
- data文件夹(必须是这个名字)，使用工具上传到LittleFS空间中。库文件使用gz进行压缩，减少http访问时间，网页显示内容都在index.html中，u文件夹为用户上传下载目录
- png系统运行截图，只用于展示部署后的web界面
- esp8266web.ino arduino程序文件，外部文件夹名字是esp8266web (ota.ino和web.ino中的方法会自动随esp8266web.ino一同编译)
- ota.ino 升级相关代码
- web.ino web相关操作在此文件中


## 功能：
- 1、通电后根据保存的多组wifi密码尝试连接，尝试失败后再尝试内置密码连接，如果依旧无法连接wifi则开启AP（IP:192.168.4.1 SSID:8266 密码:happysoul）
- 2、网页功能：基础测试功能、wifi密码配置、LittleFS文件（/u/目录下）<br>
	开关板载LED，读写JSON文件
	扫描wifi，暂未实现点击连wifi的功能<br>
	重启8266的按钮<br>
	显示LittleFS信息，显示文件列表，删除文件
	文件上传，存放到LittleFS空间内
- 3、OTA升级：默认打开了OTA升级功能，也就是说程序烧到板子以后默认打开了网络升级功能，不需要串口了，随便找个5V或者3.3V的供电就可以了。<br>
	只需要电脑和8266在同一个网络下，重启arduino IDE 在选择com口的位置就能看到8266的ip可以选择。

## TODO	计划增加功能
- 1、完善扫描wifi后保存连接的功能 这部分是纯JS的功能了。。。懒 

## 依赖：
- 1、ESP8266开发板支持<br>
	工具 - 开发板 - 开发板管理器 搜索esp8266并安装，如果没有这个选择先配网址<br>
	文件 - 首选项，开发版管理网址输入：https://arduino.esp8266.com/stable/package_esp8266com_index.json 确定后在上一步位置再搜索esp8266<br>
	安装大约需要下载100M-200M的文件<br>
- 2、[ArduinoJSON](https://github.com/bblanchon/ArduinoJson)（项目-加载库-搜索ArduinoJSON并安装）
- 3、~~[ESP8266FS](https://github.com/esp8266/arduino-esp8266fs-plugin) ~~ 不使用 SPIFFS 这部分不用看了
	存储使用了 LittleFS (官方弃用SPIFFS改为LittleFS)<br>
	ESP8266上传使用 [ESP8266FS](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases)  
	jar 文件存放位置 <home_dir>/Arduino/tools/ESP8266LittleFS/tool/esp8266littlefs.jar
	ESP32上传使用 [ESP32fs](https://github.com/lorol/arduino-esp32fs-plugin/releases)  
	jar 文件存放位置 <home_dir>/Arduino/tools/ESP32FS/tool/esp32fs.jar  
	重启arduino，工具菜单下就能看到上传功能了
	
## 使用步骤
- 1、确保上述依赖已经安装
- 2、选择开发板及flash size<br>
	某宝买的带有mini usb接口的一般就选择 NodeMCU 1.0，flash size:4M (FS:2MB OTA:~1019KB) 也就是板载32Mbit，分出来16Mbit空间(2MB)给LittleFS使用,其中部分空间预留给OTA升级使用<br>
	如果你是 ESP8266 8针脚的，那么你板载flash就是 8Mbit 也就是使用的时候需要选择 1M(FS:512KB OTA:~246KB)<br>
- 3、编译上传
- 4、工具 ~~ESP8266 Sketch Data Upload~~ 原来的工具只适用于SPIFFS文件系统使用<br>
	使用 ESP8266 LittleFS Data Upload 
	这个步骤做的是将 data 文件夹下的文件复制到 esp8266 的LittleFS空间中<br>
	如果只修改了 data 文件夹中的内容不需要上传 ino 程序，只需要上传文件即可<br>
	同理，如果只修改了 ino 文件，则不需要重复上传 data 的文件<br>

ide配置和预览图片在png目录下

![Image text](https://gitee.com/happysoul/esp8266web/raw/master/png/ide.png)

![Image text](https://gitee.com/happysoul/esp8266web/raw/master/png/01.png)

![Image text](https://gitee.com/happysoul/esp8266web/raw/master/png/02.png)

![Image text](https://gitee.com/happysoul/esp8266web/raw/master/png/03.png)


## 添加新按钮及后台代码方法

- 1、修改网页。修改 data/index.html。搜索 userButton1 可以看到2部分代码，html和js，分别对应页面显示和点击按钮事件
- 2、修改web.ino文件。需要注册url的响应代码：server.on("/userButton1", userButton1);然后在 void userButton1(){}中写入你的响应事件

同理如果需要加按钮可以复制上面样例代码可以扩展多个按钮
