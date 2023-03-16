#define BITRATE 38400
#define channels 6
#define MODE_RAW 0
#define MODE_VOLTAGE 1
#define PIN_PWM_DAC 5

int samples[channels];
bool stream = false;
bool sample_analogs = false;
bool generate_sine_wave = true;
int mode = MODE_RAW;
uint8_t duty_cycle = 0;
uint8_t duty_cycle_written = 0;
uint16_t loops = 0;


void setup() {
  // enable serial communication
  Serial.begin(BITRATE);

  // start all analog sample values at 0
  for (int i = 0; i < channels; i++) {
    samples[i] = 0;
  }
}

void send_all_channels() {

  for (int i = 0; i < channels; i++) {
    if (i > 0) Serial.print(",");
    send_channel(i);
  }

  Serial.println();
}

void send_channel(int channel_number) {
  if (mode == MODE_VOLTAGE) {
    Serial.print(samples[channel_number] / 1023.0 * 5.0);
  }

  if (mode == MODE_RAW) {
    Serial.print(samples[channel_number]);
  }
}

void update_duty_cycle() {

  if (generate_sine_wave) {
    unsigned long current_millis = millis();
    duty_cycle = (uint8_t)next_sine_value(122.5, 0.25, current_millis, 127.5, 0.0);
    // Serial.print(duty_cycle);
    // Serial.println(",0,255");
    if (duty_cycle_written != duty_cycle) {
      duty_cycle_written = duty_cycle;
      analogWrite(PIN_PWM_DAC, duty_cycle);
    }
  } else {
    if (loops == 0) duty_cycle += 1;
    if (duty_cycle_written != duty_cycle) {
      duty_cycle_written = duty_cycle;
      Serial.print(">>Duty Cycle Now: ");
      Serial.println(duty_cycle);
      analogWrite(PIN_PWM_DAC, duty_cycle);
    }
  }
}

void update_analogs() {
  // sample analog to digital converters
  if (sample_analogs) {
    for (int i = 0; i < channels; i++) {
      samples[i] = analogRead(i);
    }
  }
}

// Function to calculate the next sine wave value
float next_sine_value(float amplitude, float frequency, unsigned long timeMillis, float offset, float phaseShift) {
  float timeSeconds = timeMillis / 1000.0;
  float sineValue = amplitude * sin(2 * PI * frequency * timeSeconds + phaseShift) + offset;

  // Clamp the value between the limits
  if (sineValue > 250) {
    sineValue = 250;
  } else if (sineValue < 5) {
    sineValue = 5;
  }

  return sineValue;
}

void process_serial_data() {
  char command;

  // check if a command byte has arrived from serial
  if (Serial.available()) {
    command = Serial.read();

    // if received 0 to 5 return AD converter value of that channel
    if (command >= '0' && command <= '5') {
      int channel_to_read = command - '0';
      send_channel(channel_to_read);
      Serial.println();
    }

    // if an a is received toggle the sampling of analogs
    if (command == 'a') {
      sample_analogs = !sample_analogs;
      Serial.print(">>Sampling analogs: ");
      if (sample_analogs) {
        Serial.println(" Enabled<<");
      } else {
        Serial.println(" Disabled<<");
      }
    }

    // if a ? is received send all channels tab delimited?
    if (command == '?') {
      send_all_channels();
    }

    // if command is i toggle sine wave streaming mode
    if (command == 'i') {
      generate_sine_wave = !generate_sine_wave;
      Serial.print(">>Generate sinewave: ");
      if (generate_sine_wave) {
        Serial.println(" Enabled<<");
      } else {
        Serial.println(" Disabled<<");
      }
    }


    // toggle streaming of analog values
    if (command == 's') {
      stream = !stream;
      if (stream) {
        Serial.println(">>Stream Enabled<<");
      } else {
        Serial.println(">>Stream Disabled<<");
      }
    }

    // change mode from volts to raw
    if (command == 'm') {
      mode = (mode - 1) * -1;
      Serial.print(">>Switched to mode: ");
      Serial.print(mode);
      Serial.println("<<");
    }

    // manually increment duty cycle
    if (command == 'd') {
      duty_cycle += 1;
    }
  }
}

void stream_analogs() {
  if (stream) {
    send_all_channels();
  }
}

void loop() {
  loops += 1;
  update_duty_cycle();
  update_analogs();
  process_serial_data();
  stream_analogs();
}
