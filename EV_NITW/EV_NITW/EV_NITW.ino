#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

// Firebase configuration
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Single Wi-Fi credentials
const char* ssid = "Sai Sasivardhan";
const char* password = "Saisasi2004";

// Firebase credentials
const char* firebaseHost = "https://evnitw-default-rtdb.firebaseio.com";
const char* firebaseAuth = "0LCF4YS6PqHTO7SMJUnrM7UiW0VsTeaKDUEbOs9l";

// Define push button pins
const int BUTTON_INCREMENT = D3;
const int BUTTON_DECREMENT = D4;
const int BUTTON_RESET = D5;

int seatCount = 0; 
String seatCountPath = "/seatCountEV1";

// DFPlayer module
DFRobotDFPlayerMini dfPlayer;
SoftwareSerial audioSerial(12, 13); // RX, TX for DFPlayer

void setup() {
    Serial.begin(115200);
    audioSerial.begin(9600);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    // Add a delay before initializing DFPlayer
    delay(100);

    // Initialize DFPlayer Mini
    if (!dfPlayer.begin(audioSerial)) {
        Serial.println("Unable to begin DFPlayer Mini");
        while (true); // Stop execution if initialization fails
    }
    Serial.println("DFPlayer Mini started successfully");
    
    dfPlayer.setTimeOut(500);
    dfPlayer.volume(30);

    // Firebase configuration
    config.host = firebaseHost;
    config.signer.tokens.legacy_token = firebaseAuth;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Initialize buttons
    pinMode(BUTTON_INCREMENT, INPUT_PULLUP);
    pinMode(BUTTON_DECREMENT, INPUT_PULLUP);
    pinMode(BUTTON_RESET, INPUT_PULLUP);
}

void playSound(int audioIndex) {
    if (audioIndex > 0) {
        dfPlayer.play(audioIndex);
    } else {
        Serial.println("Unknown audio file index.");
    }
}

void playStopRequestSound(const String& stopLocation) {
    int audioIndex = 0;

    if (stopLocation == "Main_Gate") audioIndex = 1;
    else if (stopLocation == "1K_Hostel") audioIndex = 2;
    else if (stopLocation == "1.8K_Hostel") audioIndex = 3;
    else if (stopLocation == "Ladies_Hostel") audioIndex = 4;
    else if (stopLocation == "Old_Auditorium") audioIndex = 5;
    else if (stopLocation == "Blocks") audioIndex = 6;
    else if (stopLocation == "Food_Street") audioIndex = 7;
    else if (stopLocation == "Chemical_Department") audioIndex = 8;
    else if (stopLocation == "Metallurgy_Department") audioIndex = 9;

    if (audioIndex == 0) return; // Invalid stop location
    playSound(audioIndex);
}

void playEmergencyStopRequestSound(const String& stopLocation) {
    int audioIndex = 0;

    if (stopLocation == "Main_Gate") audioIndex = 10;
    else if (stopLocation == "1K_Hostel") audioIndex = 11;
    else if (stopLocation == "1.8K_Hostel") audioIndex = 12;
    else if (stopLocation == "Ladies_Hostel") audioIndex = 13;
    else if (stopLocation == "Old_Auditorium") audioIndex = 14;
    else if (stopLocation == "Blocks") audioIndex = 15;
    else if (stopLocation == "Food_Street") audioIndex = 16;
    else if (stopLocation == "Chemical_Department") audioIndex = 17;
    else if (stopLocation == "Metallurgy_Department") audioIndex = 18;

    if (audioIndex == 0) return; // Invalid emergency stop location
    playSound(audioIndex);
}

void checkStopRequest() {
    if (Firebase.getString(firebaseData, "/stopRequests/stopLocation")) {
        String stopLocation = firebaseData.stringData();
        playStopRequestSound(stopLocation);
        Firebase.setString(firebaseData, "/stopRequests/stopLocation", "");
    }
}

void checkEmergencyStopRequest() {
    if (Firebase.getString(firebaseData, "/emergencyStopRequests/stopLocation")) {
        String emergencyStopLocation = firebaseData.stringData();
        playEmergencyStopRequestSound(emergencyStopLocation);
        Firebase.setString(firebaseData, "/emergencyStopRequests/stopLocation", "");
    }
}

void updateSeatCount() {
    if (Firebase.setInt(firebaseData, seatCountPath, seatCount)) {
        Serial.println("Seat count updated: " + String(seatCount));
    } else {
        Serial.println("Failed to update seat count: " + firebaseData.errorReason());
    }
}

void loop() {
    // Read the state of each button
    int buttonIncrementState = digitalRead(BUTTON_INCREMENT);
    int buttonDecrementState = digitalRead(BUTTON_DECREMENT);
    int buttonResetState = digitalRead(BUTTON_RESET);

    // Increment button
    if (buttonIncrementState == LOW) {
        seatCount++;
        updateSeatCount();
        delay(50);  // Debounce delay
    }
    
    // Decrement button
    if (buttonDecrementState == LOW) {
        if (seatCount > 0) {
            seatCount--;
            updateSeatCount();
            delay(50);  // Debounce delay
        }
    }

    // Reset button
    if (buttonResetState == LOW) {
        seatCount = 0;
        updateSeatCount();
        delay(50);  // Debounce delay
    }

    // Debug: Print current seat count
    Serial.println(seatCount);

    // Check Firebase for stop and emergency stop requests
    checkStopRequest();
    checkEmergencyStopRequest();
}
