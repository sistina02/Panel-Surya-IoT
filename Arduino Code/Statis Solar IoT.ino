#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
double Power = 0; // Declare Power as a global variable

// Define WiFi credentials
#define WIFI_SSID "vivo V23e"
#define WIFI_PASSWORD "hokihakihaku"

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

Adafruit_ADS1115 ads;
Adafruit_MPU6050 mpu;
double Voltage = 0.0;
int sensitivity = 66;
int adcValue = 0;
int offsetVoltage = 2500;
double adcVoltage = 0;
double currentValue = 0;
double adc1 = 0;
unsigned long previousMillis = 0;
const long interval = 1000; 

void setup() {
  Serial.begin(9600);
  ads.begin();
  mpu.begin();

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
  
}

void loop() {
  float accel_angle_x, accel_angle_y; // Declare variables here
  float voltage, current, power; // Declare variables here
  FirebaseJson content;
  int16_t adc0;

  adc0 = ads.readADC_SingleEnded(0);
  Voltage = adc0 * (5.0 / 65535.0); // Perbaikan: Menggunakan bilangan desimal untuk konversi

  Serial.print("AIN0: ");
  Serial.print(adc0);
  Serial.print("\tVoltage: ");
  Serial.println(Voltage, 7);
  Serial.println();

  adc1 = ads.readADC_SingleEnded(1);
  adcVoltage = adc1 * (5.0 / 65535.0); // Perbaikan: Menggunakan bilangan desimal untuk konversi
  currentValue = ((adcVoltage - offsetVoltage) / sensitivity);
  Power= adcVoltage*currentValue;

  Serial.print("Raw Sensor Value = ");
  Serial.print(adc1); // Perbaikan: Mencetak nilai ADC yang benar
  Serial.print("\tVoltage(mV) = ");
  Serial.print(adcVoltage, 3);
  Serial.print("\tCurrent = ");
  Serial.println(currentValue, 3);
  Serial.print("\tPower = ");
  Serial.println(Power, 3);

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
  if (Firebase.RTDB.setFloat(&fbdo, "/Statis_Solar/Voltage", adcVoltage) &&
      Firebase.RTDB.setFloat(&fbdo, "/Statis_Solar/Current", currentValue) &&
      Firebase.RTDB.setFloat(&fbdo, "/Statis_Solar/Power", Power) &&

      
      Firebase.RTDB.setFloat(&fbdo, "/Statis_Solar/X axis angle", accel_angle_x) &&
      Firebase.RTDB.setFloat(&fbdo, "/Statis_Solar/Y axis angle", accel_angle_y) &&
      Firebase.RTDB.setString(&fbdo, "/Statis_Solar/Time", timeString)) {
    Serial.println("Upload Firebase RTDB success!");
  } else {
    Serial.println("Upload failed.");
    Serial.println("Reason: " + fbdo.errorReason());
  }

  // Check if the values are valid (not NaN)
  if (!isnan(adcVoltage) && !isnan(currentValue)) {
    // Set the 'voltage' and 'current' fields in the FirebaseJson object
    content.set("fields/Voltage/stringValue", String(adcVoltage, 2));
    content.set("fields/Current/stringValue", String(currentValue, 2));
    content.set("fields/Power/stringValue", String(Power, 2));

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
  String subcollectionPath = "Statis_Solar/" + String(timeFire) + "Data/";
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


  delay(50000);
  }
  
}