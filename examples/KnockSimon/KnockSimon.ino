/*
 * KnockSimon.ino - Knock detection & recognition library for Arduino
 * Copyright (c) 2014 Anderson de Oliveira Antunes. All right reserved.
 * 
 * This file is part of Knock.
 * 
 * Knock is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Knock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Knock; if not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#include "Knock.h"

#define RED_LED    13
#define GREEN_LED  4
#define SPEAKER    5

class SecretKnock : public KnockDetector {
public:
  SecretKnock() : KnockDetector(0) {}
  void init() {
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(SPEAKER, OUTPUT);

    randomSeed(analogRead(A1));

    registerSequence(&game);
    registerSequence(&highScoreSeq);

    resetGame();

    if (digitalRead(2)) {
      registerSequence(&cheat);
      digitalWrite(RED_LED, HIGH);
      delay(50);
      digitalWrite(RED_LED, LOW);
      expect(3, 0, 1, 2);
      playSequence(cheat);
      delay(200);
    }
  }

protected:
  void onKnock() {
    tone(SPEAKER, 329, 100);
    digitalWrite(GREEN_LED, HIGH);
    delay(debounceDelay);
    digitalWrite(GREEN_LED, LOW);
  }

  void onSequenceDetected() {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  }

  void onSequenceNotFound() {
    tone(SPEAKER, 329, 300);
    delay(300);
    tone(SPEAKER, 161, 300);
    delay(300);

    digitalWrite(RED_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);

    if (!godMode) {
      resetGame();
    }  else {
      delay(500);
      playSequence(game);
      expect(3, 0, 1, 2);
    }
  }

public:
  void playSequence(Sequence seq) {
    int i;
    tone(SPEAKER, 529, 100);
    for (i = 0; i < seq.length && seq.sequence[i] > 0; i++) {
      delay(seq.sequence[i] * 2);
      noTone(SPEAKER);
      delay(50);
      tone(SPEAKER, 529, 100);
    }
  }

  void nextLevel () {
    game.sequence[game.length] = random(1, 5) * 50;
    game.length++;
    if (game.length > highScore) {
      highScore = game.length;
    }
    playSequence(game);
    expectOnly(0);
  }

  void resetGame() {
    game.length = 0;
    int i;
    for (i = 0; i < MAX_KNOCKS; i++) {
      game.sequence[i] = 0;
    }
    delay(500);
    playSequence(game);
    expect(2, 0, 1);
  }

  void playHighScore() {
    int i;
    for (i = 0; i < highScore; i++) {
      tone(SPEAKER, 529, 50);
      delay(50);
      tone(SPEAKER, 829, 100);
      delay(300);
    }
    tone(SPEAKER, 229, 100);
    delay(200);
    resetGame();
  }

  void toggleGodMode() {
    godMode = !godMode;
    tone(SPEAKER, 900, 200);
    delay(200);
  }

private:
  uint8_t highScore = 0;
  bool godMode = false;
  Sequence game = (Sequence) {
    .sequence = new uint8_t[MAX_KNOCKS], .length = 0, .action = fNextLevel, .enabled = false
  };
  Sequence highScoreSeq = (Sequence) {
    .sequence = new uint8_t[1] {255}, .length = 1, .action = fPlayHighScore, .enabled = false
  };

  Sequence cheat = (Sequence) {
    .sequence = new uint8_t[3] {100,100,255}, .length = 3, .action = fGodMode, .enabled = false
  };

};

SecretKnock sk;

void fGodMode() {
  sk.toggleGodMode();
}

void fPlayHighScore() {
  sk.playHighScore();
}

void fNextLevel() {
  sk.nextLevel();
}

void setup() {
  sk.init();
}

void loop() {
  sk.update();
}
