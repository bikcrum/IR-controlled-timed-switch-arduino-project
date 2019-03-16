#include <SimpleTimer.h>
#include <IRremote.h>

#define REMOTE_UP_BUTTON 1153697755
#define REMOTE_DOWN_BUTTON 3691091931
#define REMOTE_LEFT_BUTTON 713627999
#define REMOTE_RIGHT_BUTTON 4131161687
#define REMOTE_POWER_BUTTON 3855596927
#define INPUT_DIGIT_DELAY 4000
#define TIME_GAP 5000

long remote_number_buttons[10] = {
  3238126971, 4039382595, 2538093563, 2259740311, 2327013275, 2666828831, 2747854299, 1541889663, 4287727287, 4084712887
};

int RECEIVER_PIN = 3;
IRrecv irrecv(RECEIVER_PIN);
decode_results results;

int RELAY_PIN = 5;

/* pin map of seven segments
_______11
_______---
___13 |   | 9
______| 7 |
_______---
____2 |   | 6
______|   |
_______---
________4
 */

#include "SevSeg.h"
SevSeg sevseg;
SimpleTimer timer;

int remaining_time = 999;

void setup()
{
  setupSevSeg();
  setupIRRemote();
  pinMode(RELAY_PIN, OUTPUT);
  timeAvailable();
}

void setupSevSeg() {
  byte numDigits = 2;
  byte digitPins[] = {
    10, 8
  };
  byte segmentPins[] = {
    11, 9, 6, 4, 2, 13, 7
  };
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(100);
}

void setupIRRemote() {
  ///Serial.begin(9600);
  irrecv.enableIRIn();
}

boolean waiting_for_next_input_digit = false;
unsigned long waiting_for_next_input_digit_expiry_time = 0;

void loop() {
  if (irrecv.decode(&results)) {
    //    Serial.println(results.value);
    signalReceived(results.value);
    irrecv.resume();
  }


  sevseg.setNumber(remaining_time, 1);

  sevseg.refreshDisplay();
  timer.run();

  if (waiting_for_next_input_digit && millis() >= waiting_for_next_input_digit_expiry_time) {
    waiting_for_next_input_digit = false;
  }
}

int timer_id = 0;

void signalReceived(unsigned long value) {
  switch (value) {
    case REMOTE_UP_BUTTON:
      remaining_time = (remaining_time + 1) % 100;
      updateTimer();
      break;
    case REMOTE_DOWN_BUTTON:
      if (remaining_time == 0) {
        remaining_time = 99;
      }
      else {
        remaining_time--;
      }
      updateTimer();
      break;
    case REMOTE_LEFT_BUTTON:
      remaining_time = remaining_time - 10;
      if (remaining_time < 0) {
        remaining_time += 100;
      }

      updateTimer();
      break;
    case REMOTE_RIGHT_BUTTON:
      remaining_time = (remaining_time + 10) % 100;
      updateTimer();
      break;
    case REMOTE_POWER_BUTTON:
      deleteTimer();
      waiting_for_next_input_digit = false;
      remaining_time = remaining_time == 0 ? 999 : 0;
      if (remaining_time == 0) {
        timeExpired();
      }
      else {
        timeAvailable();
      }
      break;
    default:
      for (int i = 0; i < 10; i++) {
        if (value == remote_number_buttons[i]) {
          if (waiting_for_next_input_digit) {
            remaining_time = remaining_time * 10 + i;
            waiting_for_next_input_digit = false;
          }
          else {
            remaining_time = i;
            waiting_for_next_input_digit = true;
            waiting_for_next_input_digit_expiry_time = millis() + INPUT_DIGIT_DELAY;
          }
          updateTimer();
          break;
        }
      }
  }
}

void deleteTimer() {
  timer.deleteTimer(timer_id);
}
void updateTimer() {
  deleteTimer();
  remainingTimeCheck();
  timer_id = timer.setInterval(TIME_GAP, countdown);
}

void timeExpired() {
  //do something
  //Serial.println("time expired");
  digitalWrite(RELAY_PIN, LOW);
}

void timeAvailable() {
  //do something
  //Serial.println("time available");
  digitalWrite(RELAY_PIN, HIGH);
}

void countdown() {
  remaining_time --;
  remainingTimeCheck();
}

void remainingTimeCheck() {
  if (remaining_time <= 0) {
    remaining_time = 0;
    timer.deleteTimer(timer_id);
    timeExpired();
  }
  else {
    timeAvailable();
  }
}







