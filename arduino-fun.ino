#define BITRATE 38400
#define channels 6
#define MODE_RAW 0
#define MODE_VOLTAGE 1
#define PIN_PWM_DAC 5

int samples[channels];
bool stream = false;
int mode = MODE_RAW;
uint8_t duty_cycle = 0;


void setup() {
  Serial.begin(BITRATE);
}

void sendAllChannels() {

  for (int i = 0; i < channels; i++) {
    if (i > 0) Serial.print(",");
    sendChannel(i);
  }

  Serial.println();
}

void sendChannel(int channel_number) {
    if (mode == MODE_VOLTAGE) {
      Serial.print(samples[channel_number] / 1023.0 * 5.0);
    }

    if (mode == MODE_RAW) {
      Serial.print(samples[channel_number]);
    }
}

void loop() {
  char command;

  analogWrite(PIN_PWM_DAC, duty_cycle);

  // sample analog to digital converters
  for (int i = 0; i < channels; i++) {
    samples[i] = analogRead(i);
  }

  // check if a command byte has arrived from serial
  if (Serial.available()) {
    command = Serial.read();

    // if received 0 to 5 return AD converter value of that channel
    if (command >= '0' && command <= '5') {
      int channel_to_read = command - '0';
      sendChannel(channel_to_read);
      Serial.println();
    }

    // if a ? is received send all channels tab delimited?
    if (command == '?') {
      sendAllChannels();
    }

    if (command == 's') {
      stream = !stream;
      if (stream) {
        Serial.println(">>Stream Enabled<<");
      } else {
        Serial.println(">>Stream Disabled<<");
      }
    }

    if (command == 'm') {
      mode = (mode - 1) * -1;
      Serial.print(">>Switched to mode: ");
      Serial.print(mode);
      Serial.println("<<");
    }

    if(command == 'd') {
      duty_cycle += 1;
      Serial.print(">>Duty Cycle Now: ");
      Serial.println(duty_cycle);
    }
  }

  if (stream) {
    sendAllChannels();
  }
}
