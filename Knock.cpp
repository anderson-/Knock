/*
 * Knock.cpp - Knock detection & recognition library for Arduino
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

void KnockDetector::init() {
}

void KnockDetector::update() {
  if (listenKnock()) {
    searchAndRun();
  }
}

void KnockDetector::registerSequence(Sequence * sequence) {
  sequences[sequencesLength] = sequence;
  sequencesLength++;
}

void KnockDetector::expectOnly(uint8_t id) {
  clear();
  sequences[id]->enabled = true;
}

void KnockDetector::expect(int inputLength, ...) {
  clear();
  va_list arguments;
  va_start(arguments, inputLength);
  int i;
  for (i = 0; i < inputLength; i++) {
    sequences[va_arg(arguments, int)]->enabled = true;
  }
  va_end(arguments);
}

void KnockDetector::clear() {
  int i;
  for (i = 0; i < sequencesLength; i++) {
    sequences[i]->enabled = false;
  }
}

Sequence * KnockDetector::getInputSequence() {
  return &inputSeq;
}

void KnockDetector::onUpdate() {
}

void KnockDetector::onKnock() {
  delay(debounceDelay);
}

void KnockDetector::onInputSequence() {
}

void KnockDetector::onSequenceDetected() {
}

void KnockDetector::onSequenceNotFound() {
}

bool KnockDetector::listenKnock() {
  onUpdate();

  if (analogRead(piezo) > threshold) {
    int i;
    inputLength = 0;
    for (i = 0; i < MAX_KNOCKS; i++) {
      input[i] = tmp[i] = 0;
    }

    long t = millis();
    int maxInterval = 0;

    onKnock();

    while (millis() - t < maxDelay) {
      if (analogRead(piezo) > threshold) {
        tmp[inputLength] = millis() - t;
        t = millis();

        onKnock();

        if (tmp[inputLength] > maxInterval) {
          maxInterval = tmp[inputLength];
        }
        inputLength++;
        if (inputLength == MAX_KNOCKS) {
          break;
        }
      }
    }

    inputSeq.length = inputLength;
    for (i = 0; i < inputLength; i++) {
      input[i] = map(tmp[i], 0, maxInterval, 0, 255);
    }

    onInputSequence();

    return true;
  }
  return false;
}

bool KnockDetector::inputEquals(uint8_t * sequence, uint8_t length) {

  if (inputLength != length) {
    return false;
  }

  int i = 0;
  int maxSeqInterval = 0;
  for (i = 0; i < length; i++) {
    if (sequence[i] > maxSeqInterval) {
      maxSeqInterval = sequence[i];
    }
  }

  maxSeqInterval = 255 - maxSeqInterval;

  float error = 0.0;
  int diff = 0;

  for (i = 0; i < length; i++) {
    diff = abs(input[i] - (sequence[i] + maxSeqInterval));
    error += (float) diff / length;
    if (diff > maxLocalError || error > maxError) {
      return false;
    }
  }

  return true;
}

Sequence * KnockDetector::searchAndRun() {
  int i;
  for (i = 0; i < sequencesLength; i++) {
    if (sequences[i]->enabled && inputEquals(sequences[i]->sequence, sequences[i]->length)) {
      if (sequences[i]->action != NULL) {
        sequences[i]->action();
      }
      onSequenceDetected();
      return sequences[i];
    }
  }
  onSequenceNotFound();
  return NULL;
}
