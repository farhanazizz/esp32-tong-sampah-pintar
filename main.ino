#include <ESP32Servo.h>

#define TRIG_PIN   5
#define ECHO_PIN   18
#define SERVO_PIN  14

// --- Threshold (cm) ---
const float OPEN_THRESHOLD_CM  = 30.0; // <= 30 cm -> buka cepat
const float CLOSE_THRESHOLD_CM = 60.0; // >= 60 cm -> mulai jeda 2s lalu tutup halus
const uint32_t CLOSE_DELAY_MS  = 2000; // jeda sebelum mulai nutup

// --- Servo angles ---
const int ANGLE_CLOSED = 0;
const int ANGLE_OPEN   = 90;  // sesuaikan dengan mekanikmu

// --- Servo speeds (deg/s) ---
const float SPEED_OPEN_DEG_S  = 600.0; // cepat (buka)
const float SPEED_CLOSE_DEG_S = 120.0; // halus (tutup)

// --- Loop timing ---
const uint32_t READ_INTERVAL_MS = 80;  // frekuensi baca sensor
const uint32_t STEP_INTERVAL_MS = 10;  // frekuensi update servo

Servo myServo;

// state untuk jeda penutupan
bool waitingToClose = false;
uint32_t closeRequestAt = 0;

// posisi servo & target
float currentAngle = ANGLE_CLOSED;
float targetAngle  = ANGLE_CLOSED;

// scheduler
uint32_t lastReadAt = 0;
uint32_t lastStepAt = 0;

// filter jarak (EMA)
float filteredCm = 200.0f;       // inisialisasi default
const float EMA_ALPHA = 0.3f;    // 0..1 (makin besar makin responsif)

float readDistanceCm() {
  // trigger 10 us
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // timeout 30 ms (sekitar 5m jarak max)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return 999.0f; // timeout -> anggap sangat jauh

  float cm = duration * 0.0343f / 2.0f;
  if (cm < 1.0f) cm = 1.0f;
  if (cm > 400.0f) cm = 400.0f;
  return cm;
}

void setTargetOpenFast() {
  targetAngle = ANGLE_OPEN;
  // buka cepat: biarkan speed open di updateServoStep()
  waitingToClose = false; // batalin rencana tutup kalau sebelumnya nunggu tutup
}

void requestCloseWithDelay() {
  if (!waitingToClose) {
    waitingToClose = true;
    closeRequestAt = millis();
  }
}

void startSmoothClose() {
  targetAngle = ANGLE_CLOSED;
  waitingToClose = false;
}

void updateServoStep() {
  uint32_t now = millis();
  if (now - lastStepAt < STEP_INTERVAL_MS) return;
  float dt = (now - lastStepAt) / 1000.0f; // detik
  lastStepAt = now;

  // tentukan speed sesuai arah
  float speedDegS = (targetAngle > currentAngle) ? SPEED_OPEN_DEG_S : SPEED_CLOSE_DEG_S;
  float maxStep = speedDegS * dt; // derajat maksimal per tick

  float error = targetAngle - currentAngle;
  if (fabs(error) <= maxStep) {
    currentAngle = targetAngle;
  } else {
    currentAngle += (error > 0 ? maxStep : -maxStep);
  }

  myServo.write((int)roundf(currentAngle));
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Attach servo (kalau perlu ubah min/max pulse sesuai servo kamu)
  myServo.attach(SERVO_PIN /*, 500, 2400*/);

  // posisi awal: tutup
  currentAngle = ANGLE_CLOSED;
  targetAngle  = ANGLE_CLOSED;
  myServo.write(currentAngle);

  Serial.println("HC-SR04 + Servo (30/60 cm, close delay 2s, smooth close, fast open)");
}

void loop() {
  uint32_t now = millis();

  // ====== Baca & filter jarak per interval ======
  if (now - lastReadAt >= READ_INTERVAL_MS) {
    lastReadAt = now;

    float cm = readDistanceCm();
    // EMA filter
    filteredCm = EMA_ALPHA * cm + (1.0f - EMA_ALPHA) * filteredCm;

    Serial.print("Jarak(EMA): ");
    Serial.print(filteredCm, 1);
    Serial.print(" cm  |  Servo: ");
    Serial.print((int)currentAngle);
    Serial.print("°  |  Target: ");
    Serial.print((int)targetAngle);
    Serial.print("°");
    if (waitingToClose) Serial.print("  [waitingToClose]");
    Serial.println();

    // ====== Logika Histeresis & Delay ======
    // Buka cepat saat objek dekat (<= 30 cm)
    if (filteredCm <= OPEN_THRESHOLD_CM) {
      setTargetOpenFast();
    }

    // Jika objek menjauh (>= 60 cm): mulai hitung jeda 2 detik untuk tutup
    if (filteredCm >= CLOSE_THRESHOLD_CM) {
      requestCloseWithDelay();
    } else {
      // kalau balik masuk < 60 cm saat menunggu -> batalkan penutupan
      if (waitingToClose) {
        waitingToClose = false;
      }
    }

    // Eksekusi tutup setelah 2 detik konsisten jauh
    if (waitingToClose && (now - closeRequestAt >= CLOSE_DELAY_MS)) {
      // Pastikan masih jauh saat jeda habis
      if (filteredCm >= CLOSE_THRESHOLD_CM) {
        startSmoothClose();
      } else {
        waitingToClose = false; // batal karena objek balik lagi
      }
    }
  }

  // ====== Gerakkan servo halus sesuai target ======
  updateServoStep();
}
