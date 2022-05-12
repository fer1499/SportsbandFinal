//Change the code below by your sketch
#include <Wire.h>
#include <WiFi.h>



#define BaudRate 115200
#define Acc_Sensitivity 16384.00 
#define LED_B   0
#define LED_G   2 
#define LED_R   4
#define SDApin  21    //IO 21 SDA
#define SCLpin  22    //IO 22 SCL

#define ssid "IoT"
#define password "1t3s0IoT18"

long accl_X;
long accl_Y; 
long accl_Z;


/*void setup(){
  Serial.begin(9600);
    connectWiFi(ssid, password);
  
  RequestOptions options;
    options.method = "GET";
  
  char* ip="192.168.0.3";
  char* port="8080";

  //char direction=ip+":"+port+"?accX="+accl_X+"&accY="+accl_Y+"&accZ="+accl_Z;
  char* direction = "google.com";
  
  Response response = fetch(direction, options);
    
    // Printing response body as plain text.
    Serial.println();
    Serial.println(response.text());
}
*/

//long accl_X;
//long accl_Y; 
//long accl_Z;

bool blinkState = false;
uint32_t tStart;
float Timer;
uint32_t period = 360*1000L; // timer = 1 Stunde für die Daten lesen. Kann geändert werden.
void setup() {
  Wire.begin(SDApin, SCLpin);//, 100000); // configure before use.
  WiFi.begin(ssid, password);
  
  setupMPU6050(); // Aufrufen der Funktion setupMPU6050 
  Serial.begin(BaudRate); 
  while (!Serial)
    {
    ; // warten auf Connection. Nur USB-Port 
    }
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void loop() {
  recordRegister();
  blinkState = !blinkState;
  digitalWrite(LED_G, blinkState);
  PrintData();
  delay(50);
}

void setupMPU6050()
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x6B);
  Wire.write(0b00000000);           //MPU wake up
  uint8_t err= Wire.endTransmission();
  if (err != 0) {
     Serial.printf("MPU wakeup Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
  Wire.beginTransmission(0b1101000);
  Wire.write(0x1B);                 //Gyro
  Wire.write(0b00000000);
  err = Wire.endTransmission();
  if (err != 0) {
     Serial.printf("Gyro wakeup Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
  
  Wire.beginTransmission (0b1101000); 
  Wire.write(0x1C);                 //Acc
  Wire.write(0b00000000); // setzen Beschleunigungssensor auf Skalierung +-2g 
  //Wire.write(0b00000010);   //Skalierung des Sensor auf +- 8g 
  //Wire.write(0b00011000); // voll Skalierung +-16g 
  err = Wire.endTransmission ();
  if (err != 0) {
     Serial.printf("Acc configure Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
 
}
void recordRegister()
{
  Wire.beginTransmission (0b1101000); // I2C Adresse 
  Wire.write(0x3B); // setzen Register 3B für erste Beschleunigung auf Lesen  
  uint8_t err =Wire.endTransmission ();
  if(err != 0) {
     Serial.printf("set Accel register fail, err=%d(%s)", err ,Wire.getErrorText(err));
   }
 
  Wire.requestFrom(0b1101000,6); //Beschleunigung 6 Register (von 3B bis 40) 
  err = Wire.lastError();
  if( err != 0 ){
     Serial.printf("Read accel registers Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
 
  //Rohdaten Beschleunigungssensor 
  // while (Wire.available ()<6); // available() will never increase, requestfrom() sets it on exit
  
  if(Wire.available() == 6) {
    accl_X = Wire.read()<<8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) und 0x3C (ACCEL_XOUT_L) 
    accl_Y = Wire.read()<<8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) und 0x3E (ACCEL_YOUT_L) 
    accl_Z = Wire.read()<<8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) und 0x40 (ACCEL_ZOUT_L)
  }

  accl_X = accl_X/Acc_Sensitivity;
  accl_Y = accl_Y/Acc_Sensitivity;
  accl_Z = accl_Z/Acc_Sensitivity;
  
  sendData(accl_X,accl_Y,accl_Z);
  delay(5000);
}
void sendData(long X, long Y, long Z){
  // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 8080;
    char* host="148.201.216.16";
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    String device= "http://148.201.216.16";
    String url = device+":"+httpPort+"?accX="+X+"&accY="+Y+"&accZ="+Z;
    

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: keep-alive\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 30000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
  
}
void PrintData()
{
  Serial.print("| accl_X: ");
  Serial.print(accl_X);
  Serial.print(" g");
  Serial.print("\t");

  Serial.print("| accl_Y ");
  Serial.print(accl_Y);
  Serial.print(" g");
  Serial.print("\t");

  Serial.print("| accl_Z ");
  Serial.print(accl_Z);
  Serial.print(" g");
  Serial.print("\n");
}
