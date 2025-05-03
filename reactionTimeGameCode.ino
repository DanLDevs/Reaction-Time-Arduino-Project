// Pins
const int middleLED = 4;
const int redLED = 11;
const int blueLED = 3;
const int startButton = 7;
const int topButton = 13;
const int bottomButton = 2;


// Game State
bool waitingForFlash = false;
bool gameActive = false;
bool winnerDeclared = false;
bool topWasFasterInTie = true;  // Default; will be updated during TIE detection
unsigned long waitStartTime = 0;
unsigned long delayDuration = 0;
unsigned long winnerTime = 0;
unsigned long topPressTime = 0;
unsigned long bottomPressTime = 0;

enum Winner { NONE, TOP, BOTTOM, TIE };
Winner winner = NONE;

void setup()
{
  pinMode(middleLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(startButton, INPUT);
  pinMode(topButton, INPUT);
  pinMode(bottomButton, INPUT);
  Serial.begin(9600);
  randomSeed(analogRead(A0)); // Helps with better randomness
}

void loop()
{
  // Start the game
  if (!waitingForFlash && digitalRead(startButton) == LOW) {
    delay(50); // Debounce
    if (digitalRead(startButton) == HIGH) {
       waitingForFlash = true;
      gameActive = false;
      winnerDeclared = false;
      winner = NONE;
      waitStartTime = millis();
      delayDuration = random(2000, 5000); // Random 2-5 seconds
      
      // Turn off all LEDs at game start
      digitalWrite(middleLED, LOW);
      analogWrite(redLED, 0);
      analogWrite(blueLED, 0);
      
      Serial.println("Game started. Waiting to flash...");
    }
  }

  // Flash the middle LED after random delay
  if (waitingForFlash && millis() - waitStartTime >= delayDuration) {
    digitalWrite(middleLED, HIGH);
    delay(500);  // Flash duration
    digitalWrite(middleLED, LOW);
    waitingForFlash = false;
    gameActive = true;
    topPressTime = 0;
    bottomPressTime = 0;
    Serial.println("Flash! React now!");
  }

  // Check for winner
  if (gameActive && !winnerDeclared) {
    if (digitalRead(topButton) == HIGH && topPressTime == 0) {
      topPressTime = millis(); 
    }
    if (digitalRead(bottomButton) == HIGH && bottomPressTime == 0) {
      bottomPressTime = millis(); 
    }
    
    if (topPressTime > 0 || bottomPressTime > 0) {
      if (topPressTime > 0 && bottomPressTime > 0) {
        // Both players pressed
        if (abs((long)(topPressTime - bottomPressTime)) <= 100) {
          winner = TIE;
          topWasFasterInTie = (topPressTime == bottomPressTime) ? random(0, 2) == 0 : topPressTime < bottomPressTime;
          Serial.println("Tie! Both players pressed!");
        } else if (topPressTime < bottomPressTime) {
          winner = TOP;
          Serial.println("Top player wins!");
        } else {
          winner = BOTTOM;
          Serial.println("Bottom player wins!");
        }
      } else if (topPressTime > 0) {
        winner = TOP;
        Serial.println("Top player wins!");
      } else if (bottomPressTime > 0) {
        winner = BOTTOM;
        Serial.println("Bottom player wins!");
      }
      gameActive = false;
      winnerDeclared = true;
      winnerTime = millis();
    }
  }
  
 // LED fade logic
  if (winnerDeclared) {
    unsigned long elapsed = millis() - winnerTime;

    if (winner == TOP) {
      if (elapsed <= 3000) {
        analogWrite(redLED, 255);
      } else if (elapsed <= 8000) {
        int fade = max(0, 255 - (int)(255.0 * (elapsed - 3000) / 5000));
        analogWrite(redLED, fade);
      } else {
        analogWrite(redLED, 0);
        winnerDeclared = false;
      }
    }

    else if (winner == BOTTOM) {
      if (elapsed <= 3000) {
        analogWrite(blueLED, 255); 
      } else if (elapsed <= 8000) {
        int fade = max(0, 255 - (int)(255.0 * (elapsed - 3000) / 5000));
        analogWrite(blueLED, fade);
      } else {
        analogWrite(blueLED, 0);
        winnerDeclared = false;
      }
    }

    else if (winner == TIE) {
      bool topWasFaster = (topPressTime == bottomPressTime) ? random(0, 2) == 0 : topPressTime < bottomPressTime;

      // Winner gets 3s full + 5s fade
      if (elapsed <= 3000) {
        analogWrite(topWasFasterInTie ? redLED : blueLED, 255);
      } else if (elapsed <= 8000) {
        int fade = max(0, 255 - (int)(255.0 * (elapsed - 3000) / 5000));
        analogWrite(topWasFasterInTie ? redLED : blueLED, fade);
      } else {
        analogWrite(topWasFasterInTie ? redLED : blueLED, 0);
      }

      // Loser fades immediately over 2.5s
      if (elapsed <= 2500) {
        int fastFade = max(0, 255 - (int)(255.0 * elapsed / 2500));
        analogWrite(topWasFasterInTie ? blueLED : redLED, fastFade);
      } else {
        analogWrite(topWasFasterInTie ? blueLED : redLED, 0);
      }

      if (elapsed >= 8000) {
        winnerDeclared = false;
      }
    }
  }
}