

/*********************************************************************
This is an example sketch for ESP8266-12E with DHT22+ PMS3003 (PM2.5 sensor)  
+ 1602/2004A  LCD Display .

Written by alk298.

*********************************/ 

  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  #include <ESP8266WiFi.h>
 // #include <WiFiClientSecure.h>
  #include <ESP8266WiFi.h>


 #include <ESP8266Ping.h>
 const IPAddress remote_ip(8, 8, 8, 8); // Remote host

// ThingSpeak Settings
/ 使用 GET 傳送資料的格式
// GET /update?key=[THINGSPEAK_KEY]&field1=[data 1]&filed2=[data 2]...;

String GET = "GET /update?key=xxxxxxxx"; your API Write Key


const char* thingSpeakAddress = "api.thingspeak.com";
 

  #include <SoftwareSerial.h>// for PM2.5 definition
   SoftwareSerial mySerial2(12, 14); // GPIO12--> to PM303 TX, GPIO14 ----> PM303 RX
   #define pmsDataLen 24 
    unsigned char buf[pmsDataLen];
    int idx = 0;
    int pm10 = 0;
    int pm25 = 0;
    int pm100 = 0;
    int uppm25=0;

    bool hasPm25Value = false;

 // for CO2 definition --TBC
     int upco2=0;

  #include "DHT.h"
  #define DHTPIN 13 // GPIO 13
  #define DHTTYPE DHT22    

  
// LiquidCrystal_I2C lcd(0x27,16,2); // set for LCD 1602

// set the LCD 2004 address to 0x3f for a 20 chars and 4 line display
//  0x3f for B2 AirQuality System
//  0x27 for B4 Air Quality System 
 
 LiquidCrystal_I2C lcd(0x27,20,4);  



const char* ssid= "xxxxxx";
const char* password = "xxxxxxx";


String  MACAddress = "";
String LocalIP ="";
String MAC="";


DHT dht(DHTPIN, DHTTYPE, 15);


  
  void setup() 
  {
    dht.begin();

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
    

    Wire.begin(4, 5);  // GPIO :4=SDA, GPIO 5 =SCL for LCD 1602/2004
    
    mySerial2.begin(9600); // PMS 3003 UART has baud rate 9600


     lcd.init();                      // initialize the lcd 
     lcd.backlight();
     lcd.home();
     lcd.print("DQE T/H Record");
     lcd.setCursor(0, 1);//for 2004aLCD
     lcd.print("WiFi Connection !!!");//for 2004aLCD

 
     delay (2000);
 
     WiFi.mode(WIFI_STA);

     Serial.begin(115200);
     
  
    WiFi.begin(ssid, password);  

    while ( !Ping.ping(remote_ip)) {

    delay(2000);
    Serial.print(".");
    Serial.println(WiFi.localIP());

    
    lcd.setCursor(0, 1);//for 2004aLCD
    lcd.print("IP=");//for 2004aLCD
    lcd.print(WiFi.localIP());//for 2004A LCD
    lcd.setCursor(0, 2);//for 2004aLCD
    lcd.print("Check WiFi Status !!!");//for 2004aLCD
   
  }//  while ( !Ping.ping(remote_ip)) 

     MACAddress=WiFi.macAddress();
     MAC=MACAddress.substring(12, 17);

     lcd.clear(); // for 1602 LCD
     delay (700);// Dealy for stablize LCD after LCD clear();
   
    lcd.setCursor(0, 2);//for 2004aLCD
    lcd.print("IP=");//for 2004aLCD
    lcd.print(WiFi.localIP());//for 2004A LCD
    lcd.setCursor(0, 1);//for 2004aLCD
    lcd.print(MACAddress);//for 2004A LCD

     Serial.print("Pinging ip : ");
     Serial.println(remote_ip);
 
    // Ping
     if (Ping.ping(remote_ip)) 
         {
          Serial.println("Ping Success!!");
            lcd.setCursor(0, 3);//for 2004aLCD
            lcd.print("Ping Success!!!");//for 2004aLCD
          } else {
            Serial.println("Ping Error :");

           lcd.setCursor(0, 3);//for 2004aLCD
           lcd.print("Ping Error !!!");//for 2004aLCD
           
            }


     delay (5000);
     lcd.clear(); // for 1602 LCD
     delay (700);// Dealy for stablize LCD after LCD clear();

   
  }//  void setup() 



  
  void loop()  
  
  {
    ESP.wdtFeed();

     MACAddress=WiFi.macAddress();
     MAC=MACAddress.substring(12, 17);

    
    lcd.setCursor(0, 0);//for 2004aLCD
    lcd.print(MACAddress);//for 2004A LCD


   if(MACAddress =="")
     {

      
       lcd.clear(); // for 1602 LCD
       delay (700);// Dealy for stablize LCD after LCD clear();

       lcd.setCursor(0, 0);//for 2004 LCD
       lcd.print("MAC=Empty.!!!!!");//for 2004 LCD

       
      return;
     }


    WiFiClient client;


   Serial.println (WiFi.status());

    lcd.setCursor(0, 1);  //for 2004aLCD
    lcd.print("IP=");   //for 2004aLCD
    lcd.print(WiFi.localIP());  //for 2004A LCD


         delay (2000);
    

   float h = dht.readHumidity();
   float t = dht.readTemperature();
   float f = dht.readTemperature(true);


    if (isnan(h) || isnan(t) || isnan(f)) 
    {
      // lcd.setCursor(0, 0);// for 1602 LDC
        lcd.setCursor(0, 2);// for 2004A LCD
       
       lcd.print("Read Fail DHT !!!!!!!");
            delay(10000);


        Serial.println("Failed to read from DHT sensor!");
        return;
   } 




     // Show LCD for Temp and Humid & PM2.5
  
      lcd.setCursor(0, 2);// for 2004A LCD
      lcd.print("T=");// for 2004A LDC
      lcd.print(t);// for for 2004A LDC
      lcd.setCursor(9, 2);// for 2004A LDC
      lcd.print("H=");// for for 2004A LDC
      lcd.print(h);// for for 2004A LDC

      retrievePM25Value();
     
      delay(10000);
   
     //  t=25.55;
     //  h=88.88;


  if (client.connect(thingSpeakAddress, 80))
  {
           
     Serial.println("Connected to thingspeak.com");



    // 準備上傳到 ThingSpeak IoT Server 的資料
        // 已經預先設定好 ThingSpeak IoT Channel 的欄位
        // field1：溫度；field2：濕度; field3: PM2.5;field4: CO2
        String getStr = GET + "&field1=" + String(t) + 
                              "&field2=" + String(h) +
                              "&field3=" + String(uppm25) +
                              "&field4=" + String(upco2) +                  
                              " HTTP/1.1\r\n";;
        client.print( getStr );
        client.print( "Host: api.thingspeak.com\n" );
        client.print( "Connection: close\r\n\r\n" );
        

    Serial.print(getStr);
    Serial.print("\n");
    Serial.println("Datas  sent sent to www.thingspeak.com");

  }// if (client.connect(thingSpeakAddress, 80))

    else
  {
    Serial.println("Connection to thingSpeak Failed");   
    Serial.println();

    

  }
 
     delay(120000);

    lcd.clear(); // for 1602/2004
    delay (700);// Dealy for stablize LCD after LCD clear();

  }//Loop



void retrievePM25Value()
{
 Serial.println("Enter PMsensor d for Reading") ;
  hasPm25Value = false;
  int  count = 0;
  int ckecksum = 0 ;
  int exptsum = 0 ;
  memset(buf, 0, pmsDataLen);

  


 // while (( Serial2.available()>0) && (count < pmsDataLen) )
  while (( mySerial2.available()>0) && (count < pmsDataLen) )

  
  {
    //    Serial1.readBytes(buf,pmsDataLen);
    //buf[count] = Serial2.read();

    buf[count] = mySerial2.read();

    
  Serial.print(".");
    /*
      Serial.print("NO(") ;
      Serial.print(count) ;
      Serial.print(") , ") ;
      Serial.print( buf[count],DEC) ;
      Serial.print("\n") ;
    */
    if (buf[0] == 0x42 && count == 0 )
    {
      count = 1 ;
      continue ;

    }
    if (buf[1] == 0x4d && count == 1 )
    {
      count = 2 ;
      continue ;

    }
    if (count >= 2 )
    {
      count++ ;
      if (count >= (pmsDataLen) )
      {
        hasPm25Value = true ;
        break ;
      }
      continue ;
    }

  } //void retrievePM25Value()


 if (hasPm25Value )
  {
    for (int i = 0 ; i < (pmsDataLen - 2) ; i++)
      ckecksum = ckecksum + buf[i] ;
    exptsum = ((unsigned int)buf[22] << 8 ) + ((unsigned int)buf[23]) ;
    if (ckecksum == exptsum)
    {
      hasPm25Value = true ;
    }
    else
    {
      hasPm25Value = false ;
      Serial.print("\n \n ERROR Check sum");
      Serial.print("\n Sensor Check sum is : ");
      Serial.print(exptsum);
      Serial.print(", \n And Data Check sum is :");
      Serial.print(ckecksum);
      Serial.println("");
      return ;
    }
    pm25 = ( buf[12] << 8 ) | buf[13];
    pm10 = ( buf[10] << 8 ) | buf[11];
    pm100 = ( buf[14] << 8 ) | buf[15];

    Serial.print("\n pm2.5: ");
    Serial.print(pm25);
    Serial.print(" ug/m3\t");
    Serial.print("pm1.0: ");
    Serial.print(pm10);
    Serial.print(" ug/m3\t");
    Serial.print("pm10: ");
    Serial.print(pm100);
    Serial.print(" ug/m3");
    Serial.println("");
   

   //Show PM25 to LCD 2004
      uppm25=pm25;
      lcd.setCursor(0, 3);//for 2004 LCD  
      lcd.print("PM2.5= ");//for 2004 LCD
      lcd.print(pm25);///for 2004 LCD
      lcd.print(" ug/m3");//for 2004 LCD
      

  }


}







  
