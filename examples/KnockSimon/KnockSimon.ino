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

/*
 * INSTRUCTIONS:
 *
 * [!]:knock [_]:wait [~]:vibration
 * [S]:simon [U]:player
 *
 * U: *pull cord*
 * S: <ON> !
 * options:
 *   U:!) Start game
 *     S: !!
 *     U: !!
 *     [sequence increases by one random pause between knocks]
 *     ...
 *     S: !!_!... (random sequence)
 *     U: !_!!... (wrong sequence)
 *     S: ~ (game over)
 *     S: *knock once for each level*
 *     S: (if highscore) ~_~
 *   U:!!) Highscore
 *     S: *knock once for each level*
 *   U:!!!) Turn Off
 *   U:!!!!) Reverse Game
 *   U:!!!_!) Enable God Mode
 *
 *
 */



#include "Knock.h"

#define RED_LED    13
#define GREEN_LED  5
#define ACTR_S     5
#define ACTR_V     5

void fGodMode();
void fPlayHighScore();
void fNextLevel();

class SecretKnock : public KnockDetector {
public:
  SecretKnock() : KnockDetector(0) {}
  void init() {
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(ACTR_V, OUTPUT);
    pinMode(ACTR_S, OUTPUT);

    randomSeed(analogRead(A1));

    registerSequence(&game);
    registerSequence(&highScoreSeq);
    registerSequence(&cheat);

    resetGame();
  }

protected:

  void onKnock() {
    knockBack(100);
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
    digitalWrite(RED_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);

    if (reverseGame) {
      delay(500);
      playSequence(game);
      playSequence(*getInputSequence());
      expectOnly(1);
    } else if (!godMode) {
      knockBack(100);
      resetGame();
    } else {
      vibrate(200);
      delay(500);
      playSequence(game);
      expect(3, 0, 1, 2);
    }
  }

public:

  void knockBack(int t){
    digitalWrite(ACTR_S, HIGH);
    delay(50);
    digitalWrite(ACTR_S, LOW);
    delay(t);
  }

  void vibrate(int t){
    digitalWrite(ACTR_V, HIGH);
    delay(t);
    digitalWrite(ACTR_V, LOW);
    delay(50);
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

  void playSequence(Sequence seq) {
    int i;
    knockBack(0);
    for (i = 0; i < seq.length && seq.sequence[i] > 0; i++) {
      delay(seq.sequence[i] * 2);
      delay(50);
      knockBack(0);
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

  void playHighScore() {
    int i;
    for (i = 0; i < highScore; i++) {
      knockBack(0);
      delay(300);
    }
    vibrate(200);
    delay(200);
    vibrate(200);
    resetGame();
  }

  void toggleGodMode() {
    godMode = !godMode;
    vibrate(100);
    knockBack(0);
    vibrate(100);
    knockBack(0);
    delay(200);
  }

private:
  uint8_t highScore = 0;
  bool godMode = false;
  bool reverseGame = false;
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
