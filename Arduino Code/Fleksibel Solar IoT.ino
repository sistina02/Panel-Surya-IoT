#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>

// Define WiFi credentials
#define WIFI_SSID "AIRLANGGA-HOTSPOT"
#define WIFI_PASSWORD "@irlangg@"

// Define Firebase API Key, Project ID, and user credentials
#define DATABASE_URL "https://panelsurya-edafc-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyCcmyjVxC8E2Lbl1FvMs0_mCkcJ_x5x87k"
#define DATABASE_AUTH "inIvYBWwhrksC28aRHEipBw3QSiSxLSZb8tqJVu8"
#define FIREBASE_PROJECT_ID "panelsurya-edafc"

#define USER_EMAIL "iairel404@gmail.com"
#define USER_PASSWORD "Unair2023"

// Define Firebase Data object, Firebase authentication, and configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long startMillisPZEM;
unsigned long currentMillisPZEM;
const int periodPZEM = 500;
unsigned long startMillisLCD;
unsigned long currentMillisLCD;
const int periodLCD = 1000;
unsigned long previousMillis = 0;
const long interval = 1000; 

Adafruit_MPU6050 mpu;

SoftwareSerial pzemSerial(10, 11); // rx, tx
ModbusMaster node;
static uint8_t pzemSlaveAddr = 0x01;
#define LEDPIN 13

void setup() {
  pzemSerial.begin(9600);
  Serial.begin(9600);

    // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Print Firebase client version
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Assign the API key
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_AUTH;
  Firebase.begin(&config, &auth);
  config.api_key = API_KEY;

  // Assign the user sign-in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the callback function for the long-running token generation task
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Begin Firebase with configuration and authentication
  Firebase.begin(&config, &auth);

  // Reconnect to Wi-Fi if necessary
  Firebase.reconnectWiFi(true);

  // Set Indonesian time (GMT+7) NTP server
  configTime(7 * 3600, 0, "id.pool.ntp.org", "time.nist.gov");
  while (!time(nullptr)) {
    delay(1000);
    Serial.println("Waiting for time to synchronize...");
  }
  Serial.println("Time synchronized");
  
  // resetEnergy(pzemSlaveAddr);
  node.begin(pzemSlaveAddr, pzemSerial);
  pinMode(13, OUTPUT);
  digitalWrite(LEDPIN, 0);
  startMillisPZEM = millis();
  startMillisLCD = millis();
  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
}

/* 
RegAddr Description                 Resolution
0x0000  Voltage value               1LSB correspond to 0.1V       
0x0001  Current value low 16 bits   1LSB correspond to 0.001A
0x0002  Current value high 16 bits  
0x0003  Power value low 16 bits     1LSB correspond to 0.1W
0x0004  Power value high 16 bits  
0x0005  Energy value low 16 bits    1LSB correspond to 1Wh
0x0006  Energy value high 16 bits 
0x0007  Frequency value             1LSB correspond to 0.1Hz
0x0008  Power factor value          1LSB correspond to 0.01
0x0009  Alarm status  0xFFFF is alarm，0x0000is not alarm
*/

void loop() {
  uint8_t result;
  float accel_angle_x, accel_angle_y; // Declare variables here
  float voltage, current, power; // Declare variables here
  FirebaseJson content;
  digitalWrite(LEDPIN, 1);
  result = node.readInputRegisters(0x0000, 9); // read the 9 registers of the PZEM-014 / 016
  digitalWrite(LEDPIN, 0);
  currentMillisLCD = millis();
  if (currentMillisLCD - startMillisLCD >= periodLCD) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convert accelerometer data to angles (in degrees)
    accel_angle_x = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
    accel_angle_y = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;


    Serial.print("X: ");
    Serial.print(accel_angle_x, 1);
    Serial.print(" deg");

   
    Serial.print("Y: ");
    Serial.print(accel_angle_y, 1);
    Serial.print(" deg");

    startMillisLCD = millis(); ; // Update start time for next period
  }

  currentMillisPZEM = millis();
  if (currentMillisPZEM - startMillisPZEM >= periodPZEM) {
    if (result == node.ku8MBSuccess) {
      voltage = node.getResponseBuffer(0x0000) / 10.0;

      uint32_t tempdouble = 0x00000000;

      tempdouble |= node.getResponseBuffer(0x0003); // LowByte
      tempdouble |= node.getResponseBuffer(0x0004) << 8; // highByte
      power = tempdouble / 10.0;

      tempdouble = node.getResponseBuffer(0x0001); // LowByte
      tempdouble |= node.getResponseBuffer(0x0002) << 8; // highByte
      current = tempdouble / 1000.0;

      uint16_t energy;
      tempdouble = node.getResponseBuffer(0x0005); // LowByte
      tempdouble |= node.getResponseBuffer(0x0006) << 8; // highByte
      energy = tempdouble;
      
   Serial.print(voltage);
   Serial.print("V   ");

  
   Serial.print(current);
   Serial.print("A   ");

  
   Serial.print(power);
   Serial.print("W  ");
      
      startMillisPZEM= millis() ; // Update start time for next period
    } else {
      Serial.println("Failed to read modbus");
    }
  }
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    time_t now = time(nullptr);
    struct tm *timeinfo;
    char timeString[25];
    
    timeinfo = localtime(&now);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    Serial.print("Current date and time: ");
    Serial.println(timeString);

    Serial.print("X: ");
    Serial.print(accel_angle_x);
    Serial.println(" deg");

    Serial.print("Y: ");
    Serial.print(accel_angle_y);
    Serial.println(" deg");

      // Send data to Firebase
  if (Firebase.RTDB.setFloat(&fbdo, "/PZEM Energy Monitor Data/Voltage", voltage) &&
      Firebase.RTDB.setFloat(&fbdo, "/PZEM Energy Monitor Data/Current", current) &&
      Firebase.RTDB.setFloat(&fbdo, "/PZEM Energy Monitor Data/Power", power) &&

      Firebase.RTDB.setFloat(&fbdo, "/Accelerometer Data/X axis angle", accel_angle_x) &&
      Firebase.RTDB.setFloat(&fbdo, "/Accelerometer Data/Y axis angle", accel_angle_y) &&
      Firebase.RTDB.setString(&fbdo, "/Time", timeString)) {
    Serial.println("Upload Firebase RTDB success!");
  } else {
    Serial.println("Upload failed.");
    Serial.println("Reason: " + fbdo.errorReason());
  }

  // Check if the values are valid (not NaN)
  if (!isnan(voltage) && !isnan(current)) {
    // Set the 'Temperature' and 'Humidity' fields in the FirebaseJson object
    content.set("fields/Voltage/stringValue", String(voltage, 2));
    content.set("fields/Current/stringValue", String(current, 2));
    content.set("fields/Power/stringValue", String(power, 2));

    content.set("fields/X_axis_angle/stringValue", String(accel_angle_x, 2));
    content.set("fields/Y_axis_angle/stringValue", String(accel_angle_y, 2));
    content.set("fields/Time/stringValue", String(timeString));

    Serial.print("Update/Add Data... ");
    // Increment the last entry value
      char timeFire[25];
  strftime(timeFire, sizeof(timeFire), "%Y%m%d%H%M%S", timeinfo);
  Serial.print("Tanggal dan waktu saat ini: ");
  Serial.println(timeFire);

  // Construct the subcollection path with the incremented value as the document ID
  String subcollectionPath = "Solar/" + String(timeFire) + "Data/";
  Serial.println("Subcollection Path: " + subcollectionPath);
    
    //    FIRE STORE CODE!!
    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "Voltage") && 
    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "Current") && 
    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "Power") && 

    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "X_axis_angle") && 
    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "Y_axis_angle") && 
    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", subcollectionPath.c_str(), content.raw(), "Time")) {
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      } else {
            Serial.println(fbdo.errorReason());
          }
        } else {
          Serial.println("Failed to read data.");
        }


  delay(2000);
  }
}