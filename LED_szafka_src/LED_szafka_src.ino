#include "header.h"

bool inverse_sensor_logic = false;
bool all_leds_when_any_sensor = false;

volatile uint32_t sys_ticks = 0;
volatile uint32_t last_quick_toggle_ticks = 0;
volatile uint32_t force_on_mode_ticks = 0;
bool last_conditions[NUM_OF_LEDSTRIPS] = {0};

void setup() {
  
  pinMode(JUMPER_NONC_PIN, INPUT_PULLUP);
  pinMode(JUMPER_ALL_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BOARD_LED_PIN, OUTPUT);
  digitalWrite(BOARD_LED_PIN, HIGH);

  delay(1000);
  
  digitalWrite(BOARD_LED_PIN, LOW);
  // digitalWrite(BUZZER_PIN, LOW);

  delay(1000);

  inverse_sensor_logic = (digitalRead(JUMPER_NONC_PIN) == LOW);
  all_leds_when_any_sensor = (digitalRead(JUMPER_ALL_PIN) == LOW);

  for (size_t i=0; i < NUM_OF_SENSORS; i++) {
    if (sensors[i].is_enabled) {
      pinMode(sensors[i].pin, INPUT_PULLUP);
    } else {
      pinMode(sensors[i].pin, INPUT);
    }
  }

  for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
    pinMode(leds[i].pin, leds[i].is_enabled);
  }


  cli();

  // RTC PIT
  CLKCTRL.MCLKCTRLA = 0x1;
  RTC_CLKSEL = 0x0;

  RTC_PITINTCTRL = 0x1;
  RTC_PITCTRLA = RTC_PERIOD_CYC512_gc + 1;
  // timer 64 Hz

  sei();
}

void loop() {
  delay(10);
  
  // toggle led
  // digitalWrite(BOARD_LED_PIN, !digitalRead(BOARD_LED_PIN));

  delay(10);


  // read sensors
  for (size_t i=0; i < NUM_OF_SENSORS; i++) {
    if (sensors[i].is_enabled) {
      // internal pullup -> sensors are active LOW
      sensors[i].state = ( digitalRead(sensors[i].pin) == inverse_sensor_logic );
    } 
  }



  // handle led timers
  bool enable_buzzer = false;
  for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
    if (leds[i].state == LOW && leds[i].timer >= TICKS_TO_ON && leds[i].timer < ON_TICKS_TO_BUZZER_OFF) {
      // enable led when condition met for enough time
      leds[i].state = HIGH;
      leds[i].timer = 0;
    } else if (leds[i].timer == 0) {
      leds[i].state = LOW;
    } else if (leds[i].state == HIGH && leds[i].timer >= ON_TICKS_TO_BUZZER && leds[i].timer < ON_TICKS_TO_BUZZER_OFF) {
      // enable buzzer when on for too long
      enable_buzzer = true;
    } else if (leds[i].timer >= ON_TICKS_TO_BUZZER_OFF) {
      // disable led on for too long
      leds[i].state = LOW;
      leds[i].timer = ON_TICKS_TO_BUZZER_OFF + 1;
    }
  }


  if (force_on_mode_ticks > 0) {
    for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
      if (leds[i].is_enabled) {
        leds[i].state = HIGH;
      }
    }

    if (force_on_mode_ticks > FORCE_ON_DURATION_TICKS - 64 * 2) {
      enable_buzzer = true;
    }
  }

  digitalWrite(BUZZER_PIN, enable_buzzer);




  // write led state to physical output
  for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
    if (leds[i].is_enabled) {
      digitalWrite(leds[i].pin, leds[i].state);
    } else {
      digitalWrite(leds[i].pin, LOW);
    }
  }

}


void tickHandler() {
  sys_ticks++;

  bool conditions[NUM_OF_LEDSTRIPS] = {
    LED_CONDITIONS
  };

  bool any = false;
  for (size_t i=0; i < NUM_OF_SENSORS; i++) {
    if (conditions[i]) {
      any = true;
      break;
    }
  }


  digitalWrite(BOARD_LED_PIN, any);

  // handle all_leds_when_any_sensor
  if (all_leds_when_any_sensor) {
    for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
      conditions[i] = any;
    }
  }


  // Handle quick toggle functionality -> gestures to enable special modes
  for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {

    if (leds[i].timer > QUICK_TOGGLE_MAX_TICKS || (sys_ticks - last_quick_toggle_ticks) > QUICK_TOGGLE_TIMEOUT_TICKS) {
      leds[i].quick_toggle_counter = 0;
    }

    bool is_falling_edge = conditions[i] == LOW && last_conditions[i] == HIGH;
    if (is_falling_edge) {
      if (leds[i].timer >= QUICK_TOGGLE_MIN_TICKS && leds[i].timer < QUICK_TOGGLE_MAX_TICKS) {
        leds[i].quick_toggle_counter++;
        last_quick_toggle_ticks = sys_ticks;
        leds[i].timer = 0;

        if (leds[i].quick_toggle_counter >= QUICK_TOGGLE_NUM_TO_FORCE_ON) {
          force_on_mode_ticks = FORCE_ON_DURATION_TICKS;
          leds[i].quick_toggle_counter = 0;
        }

      }
    }

  }

  // tick timer when condition is true, decrease when false
  for (size_t i=0; i < NUM_OF_LEDSTRIPS; i++) {
    if (conditions[i] && leds[i].is_enabled) {
      leds[i].timer++;
    } else {
      if (leds[i].timer > TICKS_TO_OFF) {
        leds[i].timer = TICKS_TO_OFF;
      } else if (leds[i].timer > 0) {
        leds[i].timer--;
      } else {
        leds[i].timer = 0;
      }
    }
  
  }
  
  memcpy(last_conditions, conditions, sizeof(conditions));


  if (force_on_mode_ticks > 0) {
    force_on_mode_ticks--;
  }

}

ISR(RTC_PIT_vect) {
  cli();

  tickHandler();

  // reset RTC interrupt
  RTC_PITINTFLAGS = 1;
  sei();
}


