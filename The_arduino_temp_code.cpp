#include <DHT.h>

// === Constants ===
#define DHTPIN 3              // DHT11 Data Pin
#define DHTTYPE DHT11         // Using DHT11 Sensor
#define BUZZER_PIN 8          // Buzzer pin
#define BUTTON_PIN 2          // Button pin (internal pull-up)
#define ALERT_INTERVAL 5000   // 5 seconds between alerts
#define SERIAL_REFRESH 1000   // Serial dashboard refresh rate (1 sec)

// === Temperature Zones ===
#define TEMP_OPTIMAL_LOW 22
#define TEMP_OPTIMAL_HIGH 24
#define TEMP_NEAR_LOW 20
#define TEMP_NEAR_HIGH 26
#define TEMP_CLOSE 25

// === States ===
enum AlertState { IDLE, OPTIMAL, CLOSE, NEAR, MUTE };
AlertState currentState = IDLE;

DHT dht(DHTPIN, DHTTYPE);

// === Global Variables ===
bool systemMuted = false;
unsigned long lastAlertTime = 0;
unsigned long lastSerialUpdate = 0;

// === Function Prototypes ===
void initializeSystem();
void checkButton();
void readSensors(float &temp, float &humidity);
void updateDashboard(float temp, float humidity);
void evaluateState(float temp);
void playAlert(AlertState state);
void stopAlerts();
void printHeader();
void printDivider();
void showMutedMessage();
void soundChime();
void soundBeep();
void soundLongBeep();

void setup() {
  initializeSystem();
}

void loop() {
  checkButton();

  if (systemMuted) {
    stopAlerts();
    showMutedMessage();
    delay(500);
    return;
  }

  float temperature, humidity;
  readSensors(temperature, humidity);

  unsigned long now = millis();

  if (now - lastSerialUpdate >= SERIAL_REFRESH) {
    updateDashboard(temperature, humidity);
    lastSerialUpdate = now;
  }

  evaluateState(temperature);
  playAlert(currentState);

  delay(300); // Sensor friendly delay
}

// === Setup & Initialization ===
void initializeSystem() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  dht.begin();
  printHeader();
  Serial.println("✅ System Ready: Monitoring Sleep Environment");
  printDivider();
}

// === Button Logic ===
void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    systemMuted = true;
    Serial.println("🛑 ALERTS MUTED BY USER");
    printDivider();
  }
}

// === Sensor Reading ===
void readSensors(float &temp, float &humidity) {
  temp = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("⚠️ ERROR: Failed to read from DHT11");
    delay(2000);
    return;
  }
}

// === Dashboard Display ===
void updateDashboard(float temp, float humidity) {
  printDivider();
  Serial.print("🌡️ Temperature: ");
  Serial.print(temp);
  Serial.print(" °C  |  💧 Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  printDivider();
}

// === State Evaluation ===
void evaluateState(float temp) {
  if (temp >= TEMP_OPTIMAL_LOW && temp <= TEMP_OPTIMAL_HIGH) {
    currentState = OPTIMAL;
  } else if (temp == TEMP_CLOSE) {
    currentState = CLOSE;
  } else if ((temp >= TEMP_NEAR_LOW && temp < TEMP_OPTIMAL_LOW) || 
             (temp > TEMP_OPTIMAL_HIGH && temp <= TEMP_NEAR_HIGH)) {
    currentState = NEAR;
  } else {
    currentState = IDLE;
  }
}

// === Alert Sound Management ===
void playAlert(AlertState state) {
  unsigned long now = millis();

  if (state == OPTIMAL && now - lastAlertTime >= ALERT_INTERVAL) {
    soundChime();
    Serial.println("🎵 Optimal Temp: Perfect for sleep.");
    lastAlertTime = now;
  }
  else if (state == CLOSE && now - lastAlertTime >= ALERT_INTERVAL) {
    soundBeep();
    Serial.println("🔊 Close: Temp is 25°C.");
    lastAlertTime = now;
  }
  else if (state == NEAR) {
    Serial.println("🔔 Near Optimal: Prepare to sleep.");
    soundLongBeep();
    lastAlertTime = now;
  }
  else if (state == IDLE) {
    noTone(BUZZER_PIN);
  }
}

// === Stop Alerts ===
void stopAlerts() {
  noTone(BUZZER_PIN);
}

// === Print Helpers ===
void printHeader() {
  Serial.println("==========================================");
  Serial.println("😴 Sleep Temp & Humidity Monitor - v1.0 😴");
  Serial.println("==========================================");
}

void printDivider() {
  Serial.println("------------------------------------------");
}

void showMutedMessage() {
  Serial.println("🔕 SYSTEM IS IN SILENT MODE");
}

// === Sound Functions ===
void soundChime() {
  tone(BUZZER_PIN, 1500, 200);
}

void soundBeep() {
  tone(BUZZER_PIN, 1000, 200);
}

void soundLongBeep() {
  tone(BUZZER_PIN, 1000);
  delay(5000);
  noTone(BUZZER_PIN);
}
