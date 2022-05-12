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

#define ssid "Totalplay-18A5"
#define password "18A5120Bs556zB84"

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
uint32_t period = 360*1000L; // timer = 1 la lectura de datos. Puede ser cambiado.
void setup() {
  Wire.begin(SDApin, SCLpin);//, 100000); // config antes de usar
  WiFi.begin(ssid, password);
  
  setupMPU6050(); // Llamar a la función setupMPU6050
  Serial.begin(BaudRate); 
  while (!Serial)
    {
    ; // esperando conexcion USB-Port 
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
  Wire.write(0b00000000); // ajuste el acelerómetro a escala +-2g
  //Wire.write(0b00000010);   //Escalando el sensor a +- 8g 
  //Wire.write(0b00011000); // escala completa +-16g 
  err = Wire.endTransmission ();
  if (err != 0) {
     Serial.printf("Acc configure Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
 
}
void recordRegister()
{
  Wire.beginTransmission (0b1101000); // I2C  
  Wire.write(0x3B); // establecer registro 3B para leer para la primera aceleración 
  uint8_t err =Wire.endTransmission ();
  if(err != 0) {
     Serial.printf("set Accel register fail, err=%d(%s)", err ,Wire.getErrorText(err));
   }
 
  Wire.requestFrom(0b1101000,6); //Aceleración 6 registros (de 3B a 40)
  err = Wire.lastError();
  if( err != 0 ){
     Serial.printf("Read accel registers Fail, err=%d(%s)", err, Wire.getErrorText(err));
   }
 
  //Acelerómetro de datos sin procesar
  // while (Wire.disponible()<6); // disponible() nunca aumentará, requestfrom() lo establece al salir
  
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
    char* host="192.168.100.12";
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    String device= "http://192.168.100.12";
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
