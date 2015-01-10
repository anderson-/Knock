/*
 * Knock.h - Knock detection & recognition library for Arduino
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

#ifndef KNOCK_H
#define KNOCK_H

#include <stdint.h>
#include <Arduino.h>
#include <stdarg.h>

#ifndef CAPACITY
#define CAPACITY 10
#endif

#ifndef MAX_KNOCKS
#define MAX_KNOCKS 30
#endif

struct Sequence {
  uint8_t * sequence;
  uint8_t length;
  void (*action)();
  bool enabled;
};

class KnockDetector {
public:
  KnockDetector(int piezo) : piezo(piezo) {}
  virtual void init();
  void update();
  void registerSequence(Sequence * sequence);
  void expectOnly(uint8_t id);
  void expect(int num, ...);
  void clear();
  Sequence * getInputSequence();
  bool listenKnock();
  bool inputEquals(uint8_t * sequence, uint8_t length);
  Sequence * searchAndRun();

protected:
  virtual void onUpdate();
  virtual void onKnock();
  virtual void onInputSequence();
  virtual void onSequenceDetected();
  virtual void onSequenceNotFound();

protected:
  int threshold = 15;
  int debounceDelay = 100;
  int maxDelay = 1500;
  int maxLocalError = 64;
  int maxError = 40;
private:
  int piezo;
  int tmp[MAX_KNOCKS];
  uint8_t input[MAX_KNOCKS];
  uint8_t inputLength = 0;

  Sequence inputSeq = (Sequence) {
    .sequence = input, .length = 1, .action = NULL, .enabled = false
  };

  Sequence * sequences[CAPACITY];
  uint8_t sequencesLength = 0;
};

#endif //KNOCK_H
