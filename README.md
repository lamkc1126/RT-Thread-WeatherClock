# RT-Thread-WeatherClock
基于STM32F407的RT-Thread系统温湿度天气时钟
## 一、功能介绍

-   LCD上显示日期与时间，且实现时间的联网自动校准；
-   探测开发板所处空间的温度与湿度；
-   LCD显示当前地区天气情况，且实现天气数据的联网实时更新；
-   温湿度数据的上传与监控；

## 二、整体框架

本设计使用了RT-Spark 星火一号 开发板，CPU为STM32F407，外部传感器使用ATH21采集环境温湿度，使用RW007连接室内WIFI获取时间与天气信息，然后通过ST7789V3驱动LCD屏显示我们所获取的信息，此外采集到的温湿度信息也会通过WIFI网络上传到ONENET平台。

![微信图片_20230725172004.png](https://oss-club.rt-thread.org/uploads/20230725/903b43831a542b2f28eb7b0c1bed59a8.png)

## 三、硬件介绍

RT-Spark 星火1号 开发板采用STM32F407作为主控制器。STM32F407 芯片是一款功能强大且高度集成的微控制器，具有灵活的可编程性和丰富的外设支持。

![微信图片_20230725175541.png](https://oss-club.rt-thread.org/uploads/20230725/2a2588cb4cdbd1a29ecd7cf23e4dbd8c.png.webp)

## 四、软件实现

软件层面基于 RT-Thread，为了实现温湿度天气时钟项目的功能，使用了以下组件  
AHT10: AHT10系列温湿度传感器的驱动。  
Onenet: 针对 OneNET 平台连接做的的适配，可以让设备完成数据的发送、接收、设备的注册和控制等功能。  
RW007-WiFi: RW007 模块的 SPI 驱动  
Netutils：RT-Thread 网络小工具集

软件层面主要实现了以下功能:  
1.传感器通信: 通过I2C3总线与温湿度传感器进行通信，获取实时的环境数据  
2.数据处理: 将传感器数据进行处理，提取关键信息并进行上传显示。  
3.LCD显示: 在ST7787显示屏上显示信息，包括传感器数据、时间和实时天气。.  
4.NTP客户端: 通过NTP客户端，联网获取实时时间信息。  
5.WiFi模组通信: 利用 RW007 WiFi 模组将传感数据和滑条信息上传到云端界面显示。

## 五、成果展示

LCD时钟  
![时钟版面.jpg](https://oss-club.rt-thread.org/uploads/20230725/02f0f3356acb4c95d8d6924e27407d2f.jpg.webp)  
此外，我们设计了LED矩阵，根据屏幕显示的时间点亮LED矩阵至时针所指位置。（下午五时，最外围LED灯珠从12点位置顺时针点亮6颗，即表示当前时间为 17：00）

温度数据上传  
![温湿度.png](https://oss-club.rt-thread.org/uploads/20230725/ab85d7efdc2f9e4854f9d577d7e31e13.png.webp)

湿度数据上传  
![温湿度2.png](https://oss-club.rt-thread.org/uploads/20230725/ed742252ef397e6a1415a6444b95bc88.png.webp)

[视频链接](https://www.bilibili.com/video/BV1Wk4y1V7wo/?buvid=Y746DE7B3804154247DB85B56E686FC2797B&is_story_h5=false&mid=BWePuioqSFYzIyGbAvDmdA%3D%3D&p=1&plat_id=116&share_from=ugc&share_medium=iphone&share_plat=ios&share_session_id=DF5E21EE-9D0B-4557-A12C-1AA5B503D51B&share_source=WEIXIN&share_tag=s_i%C3%97tamp=1690286661&unique_k=asUr8hh&up_id=19853909 "视频链接")
