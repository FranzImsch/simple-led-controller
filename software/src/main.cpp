#include <Arduino.h>

// Code made for "Simplest LED Controller" https://frz.icu/led-ctrl

// nRF52 Arduino implementation does not support multiple pins for
// one PWM unit, so a maximum of 3 channels can be controlled at a
// time with this sketch.
//
// One possible solution to this would be to use one channel as a
// digital output only (meaning just on or off capability).

// ************* User Configuration ************* //

// #define MY_DEBUG

#define MY_RADIO_NRF5_ESB
// #define MY_REPEATER_FEATURE

#define SN "Simplest-LED-Controller"
#define SV "1.0"

#define numChannels 4 // Select amount of populated mosfets.

#define blinkLED // Makes the on board LED blink whenever a command is received

#define gammaCorrection

// ********************************************** //

const byte CHILD_ID[5] = {0, 1, 2, 3, 4};

#define LIGHT_OFF 0
#define LIGHT_ON 1

#include <MySensors.h>

// Sketch is made for a max. of 4 channels. Index 0 is not used
// to assign index 1 to ch1 …
int16_t last_state[5] = {0, LIGHT_ON, LIGHT_ON, LIGHT_ON, LIGHT_ON};
int16_t last_dim[5] = {0, 0, 0, 0, 0};

MyMessage light_msg[5] = {MyMessage(), MyMessage(CHILD_ID[1], V_STATUS), MyMessage(CHILD_ID[2], V_STATUS), MyMessage(CHILD_ID[3], V_STATUS), MyMessage(CHILD_ID[4], V_STATUS)};
MyMessage dimmer_msg[5] = {MyMessage(), MyMessage(CHILD_ID[1], V_PERCENTAGE), MyMessage(CHILD_ID[2], V_PERCENTAGE), MyMessage(CHILD_ID[3], V_PERCENTAGE), MyMessage(CHILD_ID[4], V_PERCENTAGE)};

const byte channelToPin[5] = {0, 5, 4, 1, 0}; // Assigning Ch 1 to pin 5, ...
const byte LED_ONBOARD[3] = {0, 6, 8};
#ifdef gammaCorrection
extern const uint8_t gamma8[];
#endif

void presentation()
{
  // Send the sketch version information to the gateway
  sendSketchInfo(SN, SV);

  for (byte a = 1; a <= numChannels; a++)
  {
    present(CHILD_ID[a], S_DIMMER);
    wait(5);
  }
}

void update_light()
{
  for (byte b = 1; b <= numChannels; b++)
  {
    if (last_state[b] == LIGHT_OFF)
    {
      Serial.println("Light state: OFF");
      analogWrite(channelToPin[b], 0);
    }
    else
    {
      Serial.print("Light state: ON, Level: ");
      Serial.println(last_dim[b]);
#ifdef gammaCorrection
      int value = (last_dim[b]/100.00)*255.00;
      analogWrite(channelToPin[b], pgm_read_byte(&gamma8[value]));
#else
      analogWrite(channelToPin[b], (last_dim[b] / 100.00) * 255.00);
#endif
    }
  }
}

void blinkOnboardLED(byte LED, bool makeOn)
{
#ifdef blinkLED
  if (makeOn)
  {
    digitalWrite(LED_ONBOARD[LED], HIGH);
  }
  else if (!makeOn)
  {
    digitalWrite(LED_ONBOARD[LED], LOW);
  }
#endif
}

void send_dimmer_message()
{
  blinkOnboardLED(2, true);
  for (byte c = 1; c <= numChannels; c++)
  {
    send(dimmer_msg[c].set(last_dim[c]));
    delay(50);
  }
  blinkOnboardLED(2, false);
}

void send_status_message()
{
  blinkOnboardLED(2, true);
  for (byte d = 1; d <= numChannels; d++)
  {
    if (last_state[d] == LIGHT_OFF)
    {
      send(light_msg[d].set((int16_t)0));
    }
    else
    {
      send(light_msg[d].set((int16_t)1));
    }
  }
  blinkOnboardLED(2, false);
}

void receive(const MyMessage &message)
{
  blinkOnboardLED(1, true);
  //When receiving a V_STATUS command, switch the light between OFF
  //and the last received dimmer value
  if (message.type == V_STATUS)
  {
    Serial.println("V_STATUS command received...");

    int lstate = message.getInt();

    if ((lstate < 0) || (lstate > 1))
    {
      Serial.println("V_STATUS data invalid (should be 0/1)");
      return;
    }

    byte e = message.sensor;
    last_state[e] = lstate;

    if ((last_state[e] == LIGHT_ON) && (last_dim[e] == 0))
    {
      last_dim[e] = 100;
    }

    //Update constroller status
    send_status_message();
  }
  else if (message.type == V_PERCENTAGE)
  {
    Serial.println("V_PERCENTAGE command received...");
    int dim_value = constrain(message.getInt(), 0, 100);
    if (dim_value == 0)
    {
      byte e = message.sensor;
      last_state[e] = LIGHT_OFF;

      //Update constroller with dimmer value & status
      send_dimmer_message();
      send_status_message();
    }
    else
    {
      byte e = message.sensor;
      last_state[e] = LIGHT_ON;
      last_dim[e] = dim_value;

      //Update constroller with dimmer value
      send_dimmer_message();
    }
  }
  else
  {
    Serial.println("Invalid command received...");
    return;
  }

  blinkOnboardLED(1, false);

  //Here you set the actual light state/level
  update_light();
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_ONBOARD[1], OUTPUT);
  pinMode(LED_ONBOARD[2], OUTPUT);
  delay(1000);
  update_light();
  Serial.println("Node ready to receive messages...");
}

void loop()
{
  //In MySensors2.x, first message must come from within loop()
  static bool first_message_sent = false;
  if (first_message_sent == false)
  {
    Serial.println("Sending initial state...");
    send_dimmer_message();
    send_status_message();
    first_message_sent = true;
  }
}

#ifdef gammaCorrection
const uint8_t PROGMEM gamma8[] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
#endif