#include <LiquidCrystal.h>

// Connections to the circuit: potentiometer
const int SELECTOR_PIN = A0;

// Connections to the circuit: movement detector
const int MOVE_DETECT_PIN = 6;

// Connections to the circuit: LCD screen
const int LCD_RS_PIN = 12;
const int LCD_ENABLE_PIN = 11;
const int LCD_DATA_PIN_0 = 5;
const int LCD_DATA_PIN_1 = 4;
const int LCD_DATA_PIN_2 = 3;
const int LCD_DATA_PIN_3 = 2;

LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DATA_PIN_0, LCD_DATA_PIN_1, LCD_DATA_PIN_2, LCD_DATA_PIN_3);

const int LCD_ROWS = 2;
const int LCD_COLS = 16;

const int N_DICES = 15;
const int dices[N_DICES] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 20, 22, 24, 30, 100 };

int selectedDiceIndex = -1;
int selectedDiceType = 0;
int prevSwitchState = LOW;

unsigned long secondsSinceStart = 0;

// Murusaki digits logic is: the value is the number of angles
byte murasakiDigits[5][8] = {
  // zero:
  {
    B00000,
    B00001,
    B00010,
    B00100,
    B01000,
    B10000,
    B00000,
  },
  // one:
  {
    B00000,
    B00001,
    B00011,
    B00101,
    B01001,
    B10001,
    B00000,
  },
  // two:
  {
    B00000,
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B00000,
  },
  // three:
    {
    B00000,
    B00001,
    B00011,
    B00101,
    B01001,
    B11111,
    B00000,
  },
  // four
  {
    B00000,
    B11111,
    B10001,
    B10001,
    B10001,
    B11111,
    B00000,
  },
}
;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(SELECTOR_PIN, INPUT);
  pinMode(MOVE_DETECT_PIN, INPUT);
  lcd.begin(LCD_COLS, LCD_ROWS);

  // for debug
  Serial.begin(9600);

  // Create Murasaki numbers
  for (int i=0; i<5; i++) {
    lcd.createChar(i, murasakiDigits[i]);
  }
}

void printMurasakiDigit(int value) {
  lcd.write(byte(value));
}

// Pass -1 as littleDigitCol if you want to just print "here" without setCursor
void printMurasakiNumber(unsigned long value, int littleDigitCol, int row) {
  bool leadingZeros = true;
  unsigned long divider = 1220703125; // 5 to the 13
  for (int i=13; i>=0; i--) {
    int digit = value / divider;
    if (leadingZeros == true && (digit != 0 || i == 0)) {
      leadingZeros = false;
      if (littleDigitCol >= 0) {
        lcd.setCursor(littleDigitCol - i, row);
      }
    }
    if (! leadingZeros) {
      printMurasakiDigit(digit);
    }
    value -= digit * divider;
    divider = divider / 5;
  }
}

void printDiceType(int col, int row) {
  lcd.setCursor(col, row);
  // clear old displayed value
  lcd.print("       ");
  lcd.setCursor(col, row);
  lcd.print("\xBB\xB2\xBA\xDB");
  lcd.print(selectedDiceType, DEC);
}

void selectDice() {
  // Dice selection
  int selectorRawValue = analogRead(SELECTOR_PIN);
  int newSelection = map(selectorRawValue, 1000, 20, 0, N_DICES-1);
  if (selectedDiceIndex != newSelection) {
    // Switch on internal LED for debug
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("SELECT ");
    Serial.println(selectorRawValue);
    
    selectedDiceIndex = newSelection;

    selectedDiceType = dices[selectedDiceIndex];

    printDiceType(0, 0);
    
    // Switch off internal LED for debug
    digitalWrite(LED_BUILTIN, LOW);
  }
}


void rollDice() {
  // put your main code here, to run repeatedly:
  int switchState = digitalRead(MOVE_DETECT_PIN);
  if (switchState != prevSwitchState) {
    prevSwitchState = switchState;
    if (switchState == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("ROLL");

      randomSeed(millis());
      int value = random(1, selectedDiceType + 1);

      // animation
      lcd.setCursor(0, 1);
      for (int i=0; i<LCD_COLS; i++) {
        lcd.write(0xFF);
        delay(25);
      }
      lcd.setCursor(0, 1);
      for (int i=0; i<LCD_COLS; i++) {
        lcd.write(' ');
        delay(25);
      }

      // print actual result
      printDiceType(0, 1);
      lcd.print(": ");
      lcd.print(value, DEC);
      lcd.print(" ");
      printMurasakiNumber(value, -1, -1);
      delay(500);

      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void updateMurasakiClock() {
  unsigned long newSeconds = millis() / 1000;
  if (newSeconds != secondsSinceStart) {
    secondsSinceStart = newSeconds;
    printMurasakiNumber(secondsSinceStart, LCD_COLS-1, 0);
  }
}


void loop() {
  selectDice();
  rollDice();
  updateMurasakiClock();
}
