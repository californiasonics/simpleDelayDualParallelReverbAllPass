#include "DaisyDuino.h"

static DaisyHardware hw;

#define RIGHT_LED 22 // LED 1
#define LEFT_LED 23 // LED 2

#define MAX_DELAY_SAMPLES 48000
static float DSY_SDRAM_BSS buffer[MAX_DELAY_SAMPLES];
static float DSY_SDRAM_BSS buffer1[MAX_DELAY_SAMPLES]; // Second delay
static float DSY_SDRAM_BSS buffer2[MAX_DELAY_SAMPLES]; // Third delay

// Pointers and delay times
size_t write_ptr = 0;
size_t write_ptr1 = 0;
size_t write_ptr2 = 0;
/*
size_t delay_samples = MAX_DELAY_SAMPLES * 0.023; 
size_t delay_samples1 = MAX_DELAY_SAMPLES * 0.028; 
size_t delay_samples2 = MAX_DELAY_SAMPLES * 0.005; 
*/
size_t delay_samples  = 1103; // ~22.98 ms (Prime)
size_t delay_samples1 = 1399; // ~29.15 ms (Prime)
size_t delay_samples2 = 241;  // ~5.02 ms (Prime)

float feedback = 0.82f;
float feedback1 = 0.82f;
float feedback2 = 0.4f;

float mix = 0.3f;

void AudioCallback(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float dry_in = in[0][i];

    // 1. Calculate read position wrapped to the buffer size
    size_t read_ptr = (write_ptr + MAX_DELAY_SAMPLES - delay_samples) % MAX_DELAY_SAMPLES;

    // 2. Read the delayed sample
    float delayed_sample_val = buffer[read_ptr];

    // 3. Write incoming audio + feedback into current buffer position
    buffer[write_ptr] = dry_in + (delayed_sample_val * feedback);

    // 4. Advance and wrap the write pointer
    write_ptr = (write_ptr + 1) % MAX_DELAY_SAMPLES;

    // And just do it again using the second buffer
    float dry_in1 = in[0][i];
    size_t read_ptr1 = (write_ptr1 + MAX_DELAY_SAMPLES - delay_samples1) % MAX_DELAY_SAMPLES;
    float delayed_sample_val1 = buffer1[read_ptr1];
    buffer1[write_ptr1] = dry_in1 + (delayed_sample_val1 * feedback1);
    write_ptr1 = (write_ptr1 + 1) % MAX_DELAY_SAMPLES;

    //Sum delay one and two
    float early_sum = (delayed_sample_val + delayed_sample_val1) * 0.5f;
  
    // set the read pointer
    size_t read_ptr2 = (write_ptr2 + MAX_DELAY_SAMPLES - delay_samples2) % MAX_DELAY_SAMPLES;
    float delayed_sample_val2 = buffer2[read_ptr2];

    // Allpass
    // Feedback
    float ap_node = early_sum + (delayed_sample_val2 * feedback2);
    buffer2[write_ptr2] = ap_node;
    write_ptr2 = (write_ptr2 + 1) % MAX_DELAY_SAMPLES;
    //Feed forward
    float wet_mix = delayed_sample_val2 - (feedback2 * ap_node);

    // Master Dry / Wet mix
    float output_sample = (dry_in * (1.0f - (mix))) + (wet_mix * (mix));

    out[0][i] = output_sample;
    out[1][i] = output_sample;

  }
}

void setup() {
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);

  // Clear SDRAM buffer to prevent startup noise
  for (size_t i = 0; i < MAX_DELAY_SAMPLES; i++) {
    buffer[i] = 0.0f;
    buffer1[i] = 0.0f;
    buffer2[i] = 0.0f;
  }

  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);

  DAISY.begin(AudioCallback);
}

void loop() {

  digitalWrite(RIGHT_LED, HIGH);   
  digitalWrite(LEFT_LED, HIGH); 

}
