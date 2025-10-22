struct Contactron {
  const bool is_enabled;
  const pin_size_t pin;
  bool state;
};

const size_t NUM_OF_SENSORS = 6;
Contactron sensors[NUM_OF_SENSORS] = {
  {true, PIN_PB0, LOW},
  {true, PIN_PC0, LOW},
  {true, PIN_PC1, LOW},
  {true, PIN_PC2, LOW},
  {true, PIN_PC3, LOW},
  {true, PIN_PA1, LOW}
};



struct LEDStrip {
  const bool is_enabled;
  const pin_size_t pin;
  volatile uint32_t timer;
  volatile bool state;
  volatile unsigned int quick_toggle_counter;
};


const size_t NUM_OF_LEDSTRIPS = 6;
LEDStrip leds[NUM_OF_LEDSTRIPS] = {
  {true, PIN_PB1, 0, LOW, 0},
  {true, PIN_PB2, 0, LOW, 0},
  {true, PIN_PB3, 0, LOW, 0},
  {true, PIN_PB4, 0, LOW, 0},
  {true, PIN_PB5, 0, LOW, 0},
  {true, PIN_PA7, 0, LOW, 0}
};


// timer 64Hz
const uint32_t TICKS_TO_ON = 64 / 3;
const uint32_t TICKS_TO_OFF = 64 * 2;
const uint32_t ON_TICKS_TO_BUZZER = 64UL * 60UL * 10UL;
const uint32_t ON_TICKS_TO_BUZZER_OFF = ON_TICKS_TO_BUZZER + 64UL * 15UL;

const uint32_t QUICK_TOGGLE_MIN_TICKS = TICKS_TO_ON;
const uint32_t QUICK_TOGGLE_MAX_TICKS = TICKS_TO_ON * 5;
const uint32_t QUICK_TOGGLE_TIMEOUT_TICKS = 64 * 15;

const uint32_t QUICK_TOGGLE_NUM_TO_FORCE_ON = 4;
const uint32_t FORCE_ON_DURATION_TICKS = 64UL * 60UL * 3UL;

const pin_size_t BUZZER_PIN = PIN_PA2; 
const pin_size_t BOARD_LED_PIN = PIN_PA6;

const pin_size_t JUMPER_NONC_PIN = PIN_PA4;
const pin_size_t JUMPER_ALL_PIN = PIN_PA5; 



#define S1 sensors[0].state
#define S2 sensors[1].state
#define S3 sensors[2].state
#define S4 sensors[3].state
#define S5 sensors[4].state
#define S6 sensors[5].state

// ===  WARUNKI LED  ===

#define LED_CONDITIONS \
  S1, \
  S1 || S2, \
  S3, \
  S3 || S4, \
  S5, \
  S6, 

  