

/*********************************************************************
This is an example sketch for ESP8266-12E with DHT22+ PMS3003 (PM2.5 sensor)  
+ 2004A I2C LCD Display + SensorAir S8 CO2 Sensor.

Written by Wayne

*********************************/ 


  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
  #include <SoftwareSerial.h>


 #include <ESP8266Ping.h>
 const IPAddress remote_ip(8, 8, 8, 8); // Remote host for TWN


 //************* for SensorAir S8 CO2 Sensor definition ****************
  SoftwareSerial mySerial4(2, 15); // GPIO2--> SenSorAir S8 TX GPIO15 ----> SenSorAir S8 RX

   byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
   byte response[] = {0, 0, 0, 0, 0, 0, 0}; //create an array to store the response
   // multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB  
    int valMultiplier = 1;
    unsigned long valCO2=0;
    unsigned long OrginalvalCO2=0;
    int upco2=0; //upload CO2  data to PHP Server.

 //********************************************************************


//******// for PM2.5 definition**********************
   SoftwareSerial mySerial2(12, 14); // GPIO12--> to PM303 TX, GPIO14 ----> PM303 RX
   #define pmsDataLen 24 
    unsigned char buf[pmsDataLen];
    int idx = 0;
    int pm10 = 0;
    int pm25 = 0;
    int pm100 = 0;
    int uppm25=0;

    bool hasPm25Value = false;
//****************************************************

//************ for DHT definition  ******************
  #include "DHT.h"
  #define DHTPIN 13 // GPIO 13
  #define DHTTYPE DHT22    
  
 DHT dht(DHTPIN, DHTTYPE, 15);
//****************************************************
  

 //*********** set the LCD 2004 address  for a 20 chars and 4 line display************

 // Please check LCD I2C Address before Flash  
 // LiquidCrystal_I2C lcd(0x27,16,2);  
    LiquidCrystal_I2C lcd(0x27,20,4);  
//***************************************************************************
    

//************* Access Point Infomation  ****************************




const char* ssid= "ABC";
const char* password = "12345678910";


const char* ssid1= "CDF";
const char* password1 = "12345678910";


//******************************************************************




String  MACAddress = "";
String LocalIP ="";
String MAC="";

const int httpPort = 443;
const char* host = "xxx.xxx.xx";// your own DNS Server.



  int AP=0;
  int Autodisconnect=0;
  int exitcount =0;



//********* Define SHA1 for Https Using ************************

// Use web browser to view and copy  SHA1 fingerprint of the certificate

//const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";


const char* fingerprint = "DA 39 A3 EE 5E 6B 4B 0D 32 55 BF EF 95 60 18 90 AF D8 07 09";
//******************************************************************


  
  void setup() 
  {
    dht.begin();

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
    

    Wire.begin(4, 5);  // GPIO :4=SDA, GPIO 5 =SCL for LCD 1602/2004
    
    mySerial2.begin(9600); // PMS 3003 UART has baud rate 9600
    mySerial4.begin(9600); // SenseAir S8  UART has baud rate 9600


     lcd.init();                      // initialize the lcd 
     lcd.backlight();
     lcd.home();
     lcd.print("DQE IOT System");
     lcd.setCursor(0, 1);//for 2004aLCD
     lcd.print("WiFi Connection !!!");//for 2004aLCD

 
     delay (2000);
 
     WiFi.mode(WIFI_STA);

     Serial.begin(115200);
   
  
    WiFi.begin(ssid, password);  


    //******** Change Check WiFi Status from WL_CONNECTED to Ping*************
    // while (WiFi.status() != WL_CONNECTED) {
    //************************************************************************
    while ( !Ping.ping(remote_ip)) 
    {

    delay(2000);
 

    Serial.print(".");
    Serial.println(WiFi.localIP());

    
    lcd.setCursor(0, 1);//for 2004aLCD
    lcd.print("IP=");//for 2004aLCD
    lcd.print(WiFi.localIP());//for 2004A LCD
    lcd.setCursor(0, 2);//for 2004aLCD
    lcd.print("Check WiFi Status !!!");//for 2004aLCD

    
     
      ssid=ssid1;
      password =password1;
     WiFi.begin(ssid, password); 
         delay(2000);

   
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
 
    // Check Internet Workable by Ping...
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
    exitcount=0;

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

  //******** Use WiFiClient class(Http) to create TLS connection *************
  // WiFiClient client;
  //***********************************************************************

  //******** Use WiFiClientSecure class to create TLS connection *************
  
      WiFiClientSecure client;
   //***********************************************************

   Serial.println (WiFi.status());

 //************* Hppts Connection Faile *******************
   if (!client.connect(host, httpPort)) 

  { 
 
    Serial.println("connection failed");

    lcd.setCursor(0, 1);// for 2004A LCD

    lcd.print("PHP Failed!!!!!!");
    
    lcd.setCursor(0, 2);//for 2004aLCD
    lcd.print("IP=");//for 2004aLCD
    lcd.print(WiFi.localIP());//for 2004A LCD

    

      Serial.print("Pinging ip : ");
      Serial.println(remote_ip);
 
    // Ping
     if (Ping.ping(remote_ip))
        {
           Serial.println("Ping Success !!!!");
            lcd.setCursor(0, 3);//for 2004aLCD
            lcd.print("Ping Success!!");//for 2004aLCD
          } else 
           {
              lcd.setCursor(0, 3);//for 2004aLCD
               lcd.print("Ping Error !!!!");//for 2004aLCD
           
              Serial.println("Ping Error :");
 
            }
          
    
    delay(10000);

    
 if (AP==0)
  {

      WiFi.begin(ssid, password); 
      delay(2000);
      Serial.println ("WiFi.Beging=1\n");

       Serial.println (WiFi.status());
    
  //  while (WiFi.status() != 3)
    while (!Ping.ping(remote_ip))

    
     {
        delay(2000);
        WiFi.begin(ssid, password); 
       delay(2000);
       Serial.println ("WiFi.Beging=2\n");

       Serial.println (WiFi.status());
     
 //  lcd.setCursor(0, 0);//for 1602 LCD

   lcd.setCursor(0, 1);//for 2004A LCD
 
    lcd.print("Connecting ............");

    Serial.print(".");
     exitcount = exitcount+1;

    if (exitcount >10)
    {

      Serial.println("wifi-rety 10 cycles faile  !!!!\n");
       Serial.println (WiFi.status());
     //  WiFi.disconnect(true);
     // WiFi.mode(WIFI_OFF);
      delay (5000);// Dealy for stablize LCD after LCD clear(); lcd.clear(); // for 1602 LCD
              
      lcd.clear(); // for 1602 LCD
      delay (700);// Dealy for stablize LCD after LCD clear(); lcd.clear(); // for 1602 LCD


      return;   

      }
     
    
      }//while (WiFi.status() != WL_CONNECTED) {
    

    }// if (AP==0)

     
      lcd.clear(); // for 1602 LCD
      delay (700);// Dealy for stablize LCD after LCD clear(); lcd.clear(); // for 1602 LCD
      return;   
      

   }// if (!client.connect(host, httpPort)) 
 //************* Hppts Connection Faile *******************



    lcd.setCursor(0, 1);  //for 2004aLCD
    lcd.print("IP=");   //for 2004aLCD
    lcd.print(WiFi.localIP());  //for 2004A LCD


     delay (2000);
    

   float h = dht.readHumidity();
   float t = dht.readTemperature();
   float f = dht.readTemperature(true);


    if (isnan(h) || isnan(t) || isnan(f)) 
    {
        lcd.setCursor(0, 2);// for 2004A LCD
       
        lcd.print("Read Fail DHT !!!!!!!");
            delay(10000);

        Serial.println("Failed to read from DHT sensor!");
        return;
    } 

     // Show LCD for Temp and Humid & PM2.5
  
      lcd.setCursor(0, 1);// for 2004A LCD
      lcd.print("T=");// for 2004A LDC
      lcd.print(t);// for for 2004A LDC
      lcd.print("degC");
      lcd.setCursor(10, 1);// for 2004A LDC
      lcd.print("H=");// for for 2004A LDC
      lcd.print(h);// for for 2004A LDC
      lcd.print("%");


      retrievePM25Value(); //Get PM25 data 
     
      delay(5000);

   //Get SenSorAir S8 Data    
     sendRequest(readCO2);
      OrginalvalCO2 = getValue(response); 
   
      if(OrginalvalCO2 >25000)// if CO2 Value is abnormal , keep previous vaule.
         {
            valCO2=valCO2;
         }
       else 
            valCO2=OrginalvalCO2; 

       upco2=valCO2;

        Serial.print("\n C02: ");
        Serial.print(valCO2);
        Serial.print(" PPM");



      lcd.setCursor(0, 3);// for 2004A LCD
      lcd.print("CO2=");// for 2004A LDC  
      lcd.print(valCO2);// for for 2004A LDC
      lcd.print(" PPM");// for 2004A LDC  


 //************ Set up Https Payload *****************
    String url = "/ESP8266/esp.php?";
    url += "&mac="; 
    url +=MACAddress; 
    
    url += "&pm25="; 
    url +=uppm25; 
    
     url += "&co2="; 
    url +=upco2; 
    
    url += "&t="; 
    url += t;
    url += "&h="; 
    url += h;

   Serial.print("\n ");

    Serial.println(url);

    //************ Set up Http Payload without Sercuity  *****************
 
   // client.print(String("GET ") + url + " HTTP/1.1\r\n" +
   //            "Host: " + host + "\r\n" + 
   //            "Connection: close\r\n\r\n");  

    //************ Set up Http Payload  without Sercuity*****************
 
   

 //************ Set up Https Payload *****************


//check Https connection is ok before send Payload.
 if (client.connect(host, httpPort)) 
  {
      Serial.println("Server respond ok !!!\n");

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
               
  }            

//********* Debbuging Using for  Read all the lines of the reply from server and print them to Serial********
  // while(client.available())
  // {
  // String line = client.readStringUntil('\r');
  // Serial.println(line);
   // }
   
 //*********************************************************************************************************
  
 
      delay(120000);// System Dealy for Next sensor reading cycles..

    lcd.clear(); // for 1602/2004
    delay (700);// Dealy for stablize LCD after LCD clear();

  }//Loop





///***************** Function for requesting data from the PM3003 PM2.5 Sensor ******************

void retrievePM25Value()
{
 Serial.println("Enter PMsensor d for Reading") ;
  hasPm25Value = false;
  int  count = 0;
  int ckecksum = 0 ;
  int exptsum = 0 ;
  memset(buf, 0, pmsDataLen);

  
//Wayne


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
      lcd.setCursor(0, 2);//for 2004 LCD  
      lcd.print("PM2.5= ");//for 2004 LCD
      lcd.print(pm25);///for 2004 LCD
      lcd.print(" ug/m3");//for 2004 LCD
      

  }// if (hasPm25Value )


}//void retrievePM25Value()
//****************************************************************************************




//***************** Function for requesting data from the CO2-sensor ******************

void sendRequest(byte packet[])
{
  int exitcount =0;
  while (!mySerial4.available()) //keep sending request until we start to get a response
  {
    mySerial4.write(readCO2, 7);
    delay(50);
    exitcount=exitcount+1;
   // if (exitcount >30)
   // {

   //   return;
      
   // }
    
  }

  int timeout = 0; //set a timeoute counter
  while (mySerial4.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;
    if (timeout > 10)   //if it takes to long there was probably an error
    {
      while (mySerial4.available()) //flush whatever we have
        mySerial4.read();

      break;                        //exit and try again
    }
    delay(50);
  }

  for (int i = 0; i < 7; i++)
  {
    response[i] = mySerial4.read();
  }
}

unsigned long getValue(byte packet[])
{
  int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
  int low = packet[4];                         //low byte for value is 5th byte in the packet


  unsigned long val = high * 256 + low;              //Combine high byte and low byte with this formula to get value
  return val * valMultiplier;
}

//********************************************************************************




  
