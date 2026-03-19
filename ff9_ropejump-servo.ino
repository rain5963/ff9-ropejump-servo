#include <Servo.h>

Servo jumpServo;

// ===== ピン =====
const int SERVO_PIN = 9;

// ===== 角度 =====
const int HOME_ANGLE  = 20;
const int PRESS_ANGLE = 40;

// ===== 押下動作 =====
const int PRESS_HOLD_TIME = 80;  // 押下時間
const int RETURN_TIME     = 80;  // 戻り時間

// ===== 状態 =====
bool running = false;
int pressCount = 0;
const long MAX_PRESS = 999999; // 最大回数

// ===== 時間管理 =====
unsigned long nextJumpTime = 0;

void setup() {
    Serial.begin(9600);
    jumpServo.attach(SERVO_PIN);
    jumpServo.write(HOME_ANGLE);

    Serial.println("a : start");
    Serial.println("s : reset");
}

void loop() {
    // ===== 入力 =====
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == 'a') {
            running = true;
            pressCount = 0;
            nextJumpTime = millis();  // 開始時刻
            Serial.println("START");
        }
        if (cmd == 's') {
            running = false;
            pressCount = 0;
            jumpServo.write(HOME_ANGLE);
            Serial.println("RESET");
        }
    }

    if (!running) return;

    unsigned long now = millis();
    if (now < nextJumpTime) return; // 次ジャンプまで待機

    pressCount++;

    // ===== ログ =====
    Serial.print("Press: ");
    Serial.println(pressCount);

    // ===== ジャンプ動作 =====
    jumpServo.write(PRESS_ANGLE);
    delay(PRESS_HOLD_TIME);
    jumpServo.write(HOME_ANGLE);
    delay(RETURN_TIME);

    // ===== ジャンプ間隔（アルティマニア表記） =====
    float interval;

    if (pressCount == 1) interval = 730;           // スタート
    else if (pressCount <= 19) interval = 520;     // 1〜19回 遅い
    else if (pressCount <= 49) interval = 370;     // 20〜49回 ふつう
    else if (pressCount <= 99) interval = 310;     // 50〜99回 やや速い
    else if (pressCount <= 199) interval = 275;    // 100〜199回 速い
    else if (pressCount <= 299) {                  // 200〜299回 テクニカルジャンプ
        if ((pressCount - 200) % 2 == 0) {
            interval = 175; // 偶数回（200,202…）は短め
        } else {
            interval = 274; // 奇数回（201,203…）は長め
        }
    }
    else interval = 241;                            // 300回以降 ハイスピードジャンプ

    // ===== 1599回以降 100回ごとに +10ms補正（長距離用） =====
    if (pressCount >= 1599 && (pressCount - 1599) % 100 == 0) {
        interval += 10;
    }

    // ===== 追加補正（3000以降500回ごと）=====
    if (pressCount >= 3000 && pressCount % 500 == 0) {
     interval += 50;
    }

    // ===== 次ジャンプの絶対時間 =====
    nextJumpTime = now + interval + PRESS_HOLD_TIME + RETURN_TIME;

    // ===== 終了判定 =====
    if (pressCount >= MAX_PRESS) {
        running = false;
        Serial.println("FINISH (MAX_PRESS)");
    }
}