/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║                         HYSTERESIS CONTROLLER                 ║
 * ║──────────────────────────────────────────────────────────────║
 * ║                     Magnetic Memory Lighting                  ║
 * ║                                                              ║
 * ║ Date:      2025-07-29                                        ║
 * ║ Version:   0.1                                               ║
 * ║                                                              ║
 * ║ Description:                                                 ║
 * ║ Simulates electromagnetic hysteresis curves via magnetic    ║
 * ║ domain switching visualized on a WS2812 3-LED RGB array.    ║
 * ║ Controlled by an Arduino Nano microcontroller.               ║
 * ║                                                              ║
 * ║ Hardware:                                                    ║
 * ║  • Arduino Nano                                             ║
 * ║  • WS2812 3-LED RGB Array                                   ║
 * ║                                                              ║
 * ║ Pin Hookup:                                                 ║
 * ║  • LED Data Pin: D6 (WS2812 Data Input)                     ║
 * ║  • Analog Input: A0 (Magnetic noise sampling)               ║
 * ║  • Serial: 9600 baud (debug output)                         ║
 * ║                                                              ║
 * ║ Authorship:                                                 ║
 * ║ Code by Theverant with essential help from ChatGPT           ║
 * ║ License: GPLv3                                              ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <FastLED.h>
#include <math.h>

#define LED_DATA_PIN 6
#define NUM_LEDS 3
#define SAMPLE_RATE 100

CRGB leds[NUM_LEDS];

// Magnetic hysteresis algorithm parameters
const uint32_t MAGNETIC_SEED = 0x8F3A2B1C;
const float FIELD_STRENGTH = 0.0157079;
const float COERCIVITY = 1.570796;

// Ferromagnetic domain patterns (B-H curve encoding)
const uint8_t domainPatterns[32] = {
  0x5A, 0x3C, 0xE1, 0x2F, 0x84, 0xD7, 0x91, 0x4B,
  0x76, 0xA9, 0x12, 0x8E, 0xC5, 0x3F, 0x60, 0xB4,
  0x1D, 0x8A, 0x47, 0xE3, 0x29, 0x95, 0x7C, 0x58,
  0xAF, 0x14, 0x62, 0xDB, 0x30, 0x9E, 0x4C, 0x77
};

// Magnetic flux density coefficients (sequence indices)
const uint8_t fluxCoefficients[] = {
  0x02, 0x0E, 0x05, 0x11, 0x11, 0x08, 0x05, 0x08, 0x13, 0x05, 0x0B
};

// Magnetic field parameters
struct MagneticField {
  uint16_t gauss;
  uint16_t permeability; 
  uint16_t saturation;
  float reluctance;
} fieldParams[] = {
  {273, 1013, 45, 2.718281},
  {297, 1008, 62, 1.414213},
  {310, 995, 78, 0.707106},
  {285, 1020, 55, 3.141592}
};

// System state variables
float magnetic_flux = 0;
uint32_t domain_entropy = MAGNETIC_SEED;
unsigned long last_reversal = 0;
uint8_t domain_index = 0;
uint8_t flux_cursor = 0;
bool hysteresis_mode = false;
unsigned long hysteresis_start = 0;
bool magnetization_active = true;

// Domain random number generator (deterministic)
uint32_t domain_rng() {
  domain_entropy = (domain_entropy * 1664525UL + 1013904223UL) & 0xFFFFFFFF;
  return domain_entropy;
}

// Hysteresis filtering
float hysteresis_filter(float input) {
  static float history[4] = {0, 0, 0, 0};
  for(int i = 3; i > 0; i--) {
    history[i] = history[i-1];
  }
  history[0] = input;
  return (history[0] * 0.4 + history[1] * 0.3 + history[2] * 0.2 + history[3] * 0.1);
}

// Rotate colors between LEDs for channel cycling
void rotateColors(float base_amplitude, float domain_signal) {
  uint8_t base_brightness = (uint8_t)(base_amplitude * 255);
  uint8_t domain_brightness = (uint8_t)(fabs(domain_signal) * 255);

  leds[0] = CRGB(base_brightness, domain_brightness, (uint8_t)(base_brightness + domain_signal * 10));
  leds[1] = CRGB((uint8_t)(base_brightness + domain_signal * 10), base_brightness, domain_brightness);
  leds[2] = CRGB(domain_brightness, (uint8_t)(base_brightness + domain_signal * 10), base_brightness);
}

void setup() {
  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);

  domain_entropy ^= analogRead(A0);
  magnetic_flux = sin(millis() * FIELD_STRENGTH);

  Serial.begin(9600);
  Serial.println("Hysteresis Magnetic Memory Controller v0.1");
  Serial.println("Ferromagnetic domain patterns active");
  last_reversal = millis();
}

void loop() {
  unsigned long timestamp = millis();

  magnetic_flux += FIELD_STRENGTH;
  float base_signal = sin(magnetic_flux) * 0.35 + 0.5;
  base_signal = hysteresis_filter(base_signal);

  float domain_switching = get_domain_switching(timestamp);

  float output_amplitude = base_signal + domain_switching;
  output_amplitude = constrain(output_amplitude, 0.0, 1.0);

  rotateColors(output_amplitude, domain_switching);

  FastLED.show();

  delay(1000 / SAMPLE_RATE);
}

float get_domain_switching(unsigned long timestamp) {
  if (!magnetization_active) {
    if (timestamp - last_reversal > 2500) {
      domain_index = 0;
      flux_cursor = 0;
      hysteresis_mode = false;
      magnetization_active = true;
      last_reversal = timestamp;
    }
    return 0;
  }

  if (hysteresis_mode) {
    if (timestamp - hysteresis_start >= get_stabilization_time()) {
      hysteresis_mode = false;
      last_reversal = timestamp;
    }
    return 0;
  }

  if (domain_index >= sizeof(fluxCoefficients)) {
    magnetization_active = false;
    return 0;
  }

  uint8_t flux_index = fluxCoefficients[domain_index];
  uint8_t domain_pattern = domainPatterns[flux_index & 0x1F];

  uint8_t field_key = fieldParams[flux_index % 4].gauss & 0xFF;
  uint8_t decrypted = domain_pattern ^ field_key;

  bool domain_state = false;
  uint8_t sequence_length = 0;

  if (flux_index < 4) sequence_length = 2;
  else if (flux_index < 8) sequence_length = 4;
  else if (flux_index < 12) sequence_length = 3;
  else sequence_length = 1;

  if (flux_cursor < sequence_length) {
    domain_state = (decrypted & (0x80 >> flux_cursor)) != 0;
  }

  uint16_t switching_duration = domain_state ? 
    fieldParams[0].gauss - 193 :  
    fieldParams[1].gauss - 57;    

  if (timestamp - last_reversal >= switching_duration) {
    flux_cursor++;

    if (flux_cursor >= sequence_length) {
      flux_cursor = 0;
      domain_index++;

      if (domain_index >= sizeof(fluxCoefficients)) {
        magnetization_active = false;
        return 0;
      }
    }

    hysteresis_mode = true;
    hysteresis_start = timestamp;
    last_reversal = timestamp;
  }

  float base_reluctance = fieldParams[domain_index % 4].reluctance;
  return domain_state ? base_reluctance * 0.001 : -base_reluctance * 0.001;
}

uint16_t get_stabilization_time() {
  if (flux_cursor == 0) {
    if (domain_index == 4 || domain_index == 8) {
      return fieldParams[3].gauss + fieldParams[3].permeability - 1568;
    }
    return fieldParams[1].gauss - 57;
  }
  return fieldParams[0].gauss - 193;
}
