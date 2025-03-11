#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Wi-Fi credentials
const char* ssid = "Sai Sasivardhan";
const char* password = "Saisasi2004";

// Firebase credentials
const char* firebaseHost = "https://evnitw-default-rtdb.firebaseio.com";  // Replace with your Firebase Database URL
const char* firebaseAuth = "0LCF4YS6PqHTO7SMJUnrM7UiW0VsTeaKDUEbOs9l"; // Replace with your Firebase Authentication Token

// GPS module configuration
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D1, D2); // RX, TX for GPS module (D1 = RX, D2 = TX)

// Firebase configuration
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);           // Start Serial Monitor communication
  gpsSerial.begin(9600);          // Start communication with GPS module
  Serial.println("Waiting for GPS data...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize Firebase
  config.host = firebaseHost;
  config.signer.tokens.legacy_token = firebaseAuth;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {


while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());

        if (gps.location.isUpdated()) {
            float latitude = gps.location.lat();
            float longitude = gps.location.lng();

            Serial.print("Latitude: ");
            Serial.print(latitude, 6);
            Serial.print(" Longitude: ");
            Serial.println(longitude, 6);

            // Upload coordinates to Firebase
            String path = "/coordinatesEV1"; 
            Firebase.setFloat(firebaseData, path + "/latitude", latitude);
            Firebase.setFloat(firebaseData, path + "/longitude", longitude);
            delay(100); // Upload every 0.1 seconds
        }
}
}
