// This requires the CapacitiveSensor library to be installed
// https://github.com/PaulStoffregen/CapacitiveSensor
// Include the library
#include "CapacitiveSensor.h"

// Include a file that contains frequencies for each note
#include "pitches.h"

// Output pin connected to the speaker
#define PWM_OUT_PIN 10

// Pin of the switch for changing the octave
#define OCTAVE_SWITCH_PIN A0

// Number of keys
#define NUM_KEYS 12

// The threshold to recognizing key press
#define KEY_PRESS_THRESHOLD 100

// Set capacitive sensor sensitivity
#define CAPACITIVE_SENSOR_SENSITIVITY 15

// Pointers for each key object
CapacitiveSensor *keys[NUM_KEYS];

// Times when each key is pressed (for detecting which is last pressed)
uint64_t timePressed[NUM_KEYS];

// Pins for piano keys
const uint8_t keyPins[] = {A3, 0, 1, 2, 3, 4, A5, A4, 5, 6, 7, 8};

// Common send pin for all keys
const uint8_t sendPin = 9;

void setup()
{
    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Create a key object
        keys[i] = new CapacitiveSensor(sendPin, keyPins[i]);

        // Turn off auto calibrate
        keys[i]->set_CS_AutocaL_Millis(0xFFFFFFFF);
    }
}

void loop()
{
    // Get selected octave
    int currentOctave = getOctave();

    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Check if the key is pressed
        if (keys[i]->capacitiveSensor(CAPACITIVE_SENSOR_SENSITIVITY) > KEY_PRESS_THRESHOLD)
        {
            // Remember the time when the key is pressed
            timePressed[i] = millis();
        }
    }

    // Find the last pressed key (the biggest time), and play this tone
    int lastPressedIndex = 0;
    for (int i = 1; i < NUM_KEYS; i++)
    {
        if (timePressed[i] > timePressed[lastPressedIndex])
        {
            lastPressedIndex = i;
        }
    }

    // Play the tone of the last pressed key
    tone(PWM_OUT_PIN, tones[currentOctave][lastPressedIndex]);

    // Stop the tone only if the all keys are released
    bool allKeysReleased = 1;

    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Check if the key is pressed
        if (keys[i]->capacitiveSensor(CAPACITIVE_SENSOR_SENSITIVITY) > KEY_PRESS_THRESHOLD)
        {
            // If the key is pressed, set the flag to 0, and don't check next
            allKeysReleased = 0;
            break;
        }
    }

    // If all of the keys are released
    if (allKeysReleased)
    {
        // Stop the tone
        noTone(PWM_OUT_PIN);
    }

    lastPressedKey = currentPressedKey;
}

// Read the current octave from the switch and return the selected octave
int getOctave()
{
    // Read the analog value
    int analogSwitchRead = analogRead(OCTAVE_SWITCH_PIN);

    // Return a value depending on the range it is in
    if (analogSwitchRead < 100) // The real value is around 0
        return 0;
    else if (analogSwitchRead > 240 && analogSwitchRead < 440) // The real value is around 340
        return 1;
    else if (analogSwitchRead > 590 && analogSwitchRead < 790) // The real value is around 690
        return 2;
    else if (analogSwitchRead > 900) // The real value is around 1023
        return 3;
}
