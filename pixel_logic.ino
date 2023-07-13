#include "pitches.h"
#include "LedControl.h"

#define joyX      A1
#define joyY      A0
#define joyBtn    4
#define markBtn   2
#define unmarkBtn 3
#define clearBtn  7
#define musicPin  9

#define DIMENSION     8
#define DRAWINGS_NO   6
#define NOTE_DURATION 6

volatile int xMap, yMap;

LedControl lcd = LedControl(12, 10, 11);

int drawingNo;
int crtDrawing[DIMENSION][DIMENSION] = {0};
int allDrawings[DIMENSION][DRAWINGS_NO * DIMENSION] = {
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,   0, 1, 1, 1, 0, 1, 1, 0,   0, 1, 1, 1, 1, 0, 0, 0,   0, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1,   0, 0, 0, 1, 1, 0, 0, 0,   0, 1, 0, 0, 0, 0, 1, 0,   1, 1, 1, 1, 1, 1, 1, 1,   1, 0, 0, 0, 0, 1, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1,   0, 0, 1, 1, 1, 0, 0, 0,   1, 0, 1, 0, 0, 1, 0, 1,   1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 1, 1, 1, 0,   0, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 0, 0, 1, 1, 0,   0, 1, 1, 1, 1, 0, 0, 0,   1, 0, 0, 0, 0, 0, 0, 1,   1, 1, 1, 1, 1, 1, 1, 1,   1, 0, 1, 1, 0, 1, 0, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 1, 1, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,   1, 0, 1, 0, 0, 1, 0, 1,   0, 1, 1, 1, 1, 1, 1, 0,   1, 0, 0, 0, 0, 1, 0, 1,   0, 0, 0, 1, 1, 0, 0, 0,
  1, 1, 0, 1, 1, 0, 0, 0,   1, 1, 1, 1, 1, 1, 1, 1,   1, 0, 0, 1, 1, 0, 0, 1,   0, 0, 1, 1, 1, 1, 0, 0,   1, 0, 0, 0, 0, 1, 0, 1,   0, 0, 0, 1, 1, 0, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0,   0, 1, 1, 1, 1, 1, 1, 0,   0, 1, 0, 0, 0, 0, 1, 0,   0, 0, 0, 1, 1, 0, 0, 0,   1, 0, 0, 0, 0, 1, 1, 0,   1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 0, 0, 0, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,   0, 1, 1, 1, 1, 0, 0, 0,   1, 1, 1, 1, 0, 0, 0, 0,
};

int rowStatus[DIMENSION][5];
int colStatus[5][DIMENSION];

bool isRunning;
bool alreadyPlayed;

void setup() {
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joyBtn, INPUT_PULLUP);

  pinMode(markBtn, INPUT_PULLUP);
  pinMode(unmarkBtn, INPUT_PULLUP);
  pinMode(clearBtn, INPUT_PULLUP);

  pinMode(musicPin, OUTPUT);

  digitalWrite(musicPin, HIGH);

  Serial.begin(9600);

  lcd.shutdown(0, false);

  lcd.setIntensity(0, 1);

  lcd.clearDisplay(0);

  clearDrawing();

  isRunning = true;
  alreadyPlayed = false;

  randomSeed(analogRead(A5));
  drawingNo = random(1, DRAWINGS_NO);

  printStatus();

  attachInterrupt(digitalPinToInterrupt(markBtn), markCell, RISING);
  attachInterrupt(digitalPinToInterrupt(unmarkBtn), unmarkCell, RISING);
}

void loop()
{
  if (digitalRead(clearBtn) == LOW) {
    clearDrawing();

    printStatus();
  }

  if (isRunning) {
    readJoystick();

    paintDrawing();
  } else {
    paintCheckmark();

    playMusic();
  }
}

void playMusic() {
  if (alreadyPlayed == true) {
    return;
  }

  int melody[] = {
    NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G3, NOTE_C4, NOTE_E4,
    NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G3, NOTE_C4, NOTE_E4,
    NOTE_C3, NOTE_D3, NOTE_A3, NOTE_D4, NOTE_F4, NOTE_A3, NOTE_D4, NOTE_F4,
    NOTE_C3, NOTE_D3, NOTE_A3, NOTE_D4, NOTE_F4, NOTE_A3, NOTE_D4, NOTE_F4,
    NOTE_B2, NOTE_D3, NOTE_G3, NOTE_D4, NOTE_F4, NOTE_G3, NOTE_D4, NOTE_F4,
    NOTE_B2, NOTE_D3, NOTE_G3, NOTE_D4, NOTE_F4, NOTE_G3, NOTE_D4, NOTE_F4,
    NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G3, NOTE_C4, NOTE_E4,
    NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G3, NOTE_C4, NOTE_E4,
  };

  int songLength = sizeof(melody) / sizeof(NOTE_DURATION);

  for (int thisNote = 0; thisNote < songLength; thisNote++) {
    int noteDuration = 1000 / NOTE_DURATION;

    tone(musicPin, melody[thisNote], noteDuration);
    digitalWrite(musicPin, HIGH);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    noTone(musicPin);
  }

  digitalWrite(musicPin, HIGH);

  alreadyPlayed = true;
}

void readJoystick() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);

  xMap = map(xValue, 0, 1023, 7, -1);
  yMap = map(yValue, 0, 950, 7, 0);

  lcd.clearDisplay(0);

  lcd.setLed(0, xMap, yMap, true);
}

void paintDrawing() {
  for (int row = 0; row < DIMENSION; row++) {
    for (int col = 0; col < DIMENSION; col++) {
      lcd.setLed(0, row, col, crtDrawing[row][col]);
    }
  }
}

void paintCheckmark() {
  for (int row = 0; row < DIMENSION; row++) {
    for (int col = 0; col < DIMENSION; col++) {
      lcd.setLed(0, row, col, allDrawings[row][col]);
    }
  }
}

void clearDrawing() {
  for (int row = 0; row < DIMENSION; row++) {
    for (int col = 0; col < DIMENSION; col++) {
      crtDrawing[row][col] = 0;
    }
  }
}

void markCell() {
  crtDrawing[xMap][yMap] = 1;

  printStatus();
}

void unmarkCell() {
  crtDrawing[xMap][yMap] = 0;

  printStatus();
}

void printStatus() {
  for (int i = 0; i < 25; i++) {
    Serial.println();
  }

  getColumnStatus();

  printColumnStatus();

  getRowStatus();

  printRowStatus();

  int totalMarks = 0;
  for (int i = 0; i < DIMENSION; i++) {
    totalMarks += rowStatus[i][4] + colStatus[4][i];
  }

  if (totalMarks == 2 * DIMENSION) {
    isRunning = false;
  }
}

void getColumnStatus() {
  for (int col = 0; col < DIMENSION; col++) {
    int colStatusIndex = 3;
    bool colFinished = true;
    int crtSequenceLength = 0;

    for (int row = DIMENSION - 1; row >= 0; row--) {
      if (crtDrawing[row][col] != allDrawings[row][col + drawingNo * DIMENSION]) {
        colFinished = false;
      }

      if (allDrawings[row][col + drawingNo * DIMENSION]) {
        crtSequenceLength++;
      } else {
        if (crtSequenceLength) {
          colStatus[colStatusIndex--][col] = crtSequenceLength;
        }

        crtSequenceLength = 0;
      }
    }

    // if last bit is set
    if (crtSequenceLength) {
      colStatus[colStatusIndex--][col] = crtSequenceLength;
    }

    colStatus[4][col] = colFinished ? 1 : 0;
  }
}

void printColumnStatus() {
  for (int i = 0; i < 4; i++) {
    String line = "          ";
    for (int j = 0; j < DIMENSION; j++) {
      if (colStatus[i][j]) {
        line += colStatus[i][j];
      } else {
        line += " ";
      }
      line += " ";
    }
    Serial.println(line);
  }


  String line = "          ";
  for (int j = 0; j < DIMENSION; j++) {
    line += colStatus[4][j] == 1 ? "V" : "X";
    line += " ";
  }

  Serial.println(line);
}

void getRowStatus() {
  for (int row = 0; row < DIMENSION; row++) {
    int rowStatusIndex = 3;
    bool rowFinished = true;
    int crtSequenceLength = 0;

    for (int col = DIMENSION - 1; col >= 0; col--) {
      if (crtDrawing[row][col] != allDrawings[row][col + drawingNo * DIMENSION]) {
        rowFinished = false;
      }

      if (allDrawings[row][col + drawingNo * DIMENSION]) {
        crtSequenceLength++;
      } else {
        if (crtSequenceLength) {
          rowStatus[row][rowStatusIndex--] = crtSequenceLength;
        }

        crtSequenceLength = 0;
      }
    }

    // if last bit is set
    if (crtSequenceLength) {
      rowStatus[row][rowStatusIndex--] = crtSequenceLength;
    }

    rowStatus[row][4] = rowFinished ? 1 : 0;
  }
}

void printRowStatus() {
  for (int i = 0; i < DIMENSION; i++) {
    String line = "";
    for (int j = 0; j < 4; j++) {
      if (rowStatus[i][j]) {
        line += rowStatus[i][j];
      } else {
        line += " ";
      }
      line += " ";
    }
    line += rowStatus[i][4] == 1 ? "V" : "X";
    line += " ";

    for (int j = 0; j < DIMENSION; j++) {
      if (crtDrawing[i][j]) {
        line += "1";
      } else {
        line += " ";
      }
      line += " ";
    }

    Serial.println(line);
  }
}
