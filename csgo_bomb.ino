#include <Keypad.h>
#include <LiquidCrystal.h>

const int SWITCH_PIN = 1;
const int BUZZER_PIN = 0;
const int LED_PIN = 9;
const int CONTRAST_PIN = A3;
const int ROW_NUM = 4;
const int COLUMN_NUM = 3;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'x','0','#'}
};

char code[7] = "*******";

byte rowPins[ROW_NUM] = {5, 7, 8, 6};
byte colPins[COLUMN_NUM] = {2, 3, A6};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROW_NUM, COLUMN_NUM);

LiquidCrystal lcd(10, 11, 12, A0, A1, A2);

bool armed = false;
bool countdown = false;
int second = 0;
unsigned long lastMillis = 0;// added better variable assignment
unsigned long lastBeepMillis = 0;
const int beepLengths[] = {125, 100, 75, 50, 25};//beep rates and lengths can be structred in an array
const int beepRates[] = {1000, 500, 250, 100, 50};

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite(CONTRAST_PIN, 255);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("*******");
  lcd.noDisplay();
  digitalWrite(LED_PIN, LOW);
}

void armSystem() {
  armed = true;
  lcd.display();
  Serial.println("ARMED");
  tone(BUZZER_PIN, 2500, 100);
  delay(200);
  tone(BUZZER_PIN, 2500, 100);
  delay(200);
}

void checkCode() {
  if (strcmp(code, "7355608") == 0) { // Correct code
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("BOMB PLANTED");
    countdown = true;
    armed = false;
  } else { // Incorrect code
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("*******");
    tone(BUZZER_PIN, 2500, 100);
    delay(200);
    tone(BUZZER_PIN, 2500, 100);
    delay(200);
    memset(code, '*', sizeof(code)); // Reset code
  }
}

void performCountdown() {
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    second++;
  }

  int stage = 0;
  while (stage < 5 && second > beepRates[stage]) {
    stage++;
  }

  if (second >= 40) {
    // Handle end of countdown
    second = 0;
    for (int i = 150; i < 10000; i += 50) {
      tone(BUZZER_PIN, i);
      delay(10);
    }
    for (int i = 0; i < 8; i++) {
      tone(BUZZER_PIN, 5000, 50);
      delay(100);
    }
    armed = false;
    countdown = false;
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("*******");
    lcd.noDisplay();
    digitalWrite(LED_PIN, LOW);
  } else {
    int beepLength = beepLengths[stage];
    if (millis() - lastBeepMillis > beepRates[stage] + beepLength) {
      lastBeepMillis = millis();
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 2146, beepLength);
      delay(beepLength);
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void loop() {
  if (!armed && !countdown && digitalRead(SWITCH_PIN) == LOW) {
    armSystem();
  }

  if (armed) {
    char key = keypad.getKey();
    if (key) {
      tone(BUZZER_PIN, 2000);
      delay(50);
      noTone(BUZZER_PIN);
      
      for (int i = 0; i < 7; i++) {
        if (code[i] == '*') {
          code[i] = key;
          break;
        }
      }
      
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print(code);

      if (code[6] != '*') { // Full code entered
        checkCode();
      }
    }
  }

  if (countdown) {
    performCountdown();
  }
}

