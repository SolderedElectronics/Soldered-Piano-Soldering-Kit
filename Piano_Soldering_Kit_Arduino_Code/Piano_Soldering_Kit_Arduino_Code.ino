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

// Srpeggio settings
#define ARPEGGIO_SPEED_POT_PIN A2
#define ARPEGGIO_MIN_DELAY     0
#define ARPEGGIO_MAX_DELAY     60

// Pin for the note selector
#define MULTI_TONE_SW_PIN 11

// Number of keys
#define NUM_KEYS 12

// The threshold to recognizing key press
#define KEY_PRESS_THRESHOLD 100

// Set capacitive sensor sensitivity
#define CAPACITIVE_SENSOR_SENSITIVITY 15

// Pins for piano keys
const uint8_t keyPins[] = {A3, 0, 1, 2, 3, 4, A5, A4, 5, 6, 7, 8};

// Common send pin for all keys
const uint8_t sendPin = 9;

// Pointers for each key object
CapacitiveSensor *keys[NUM_KEYS];

// Struct for information about each key
struct keyInformation
{
    uint32_t timePressed; // Time when the key is pressed
    bool pressed;         // Is the key pressed
    byte keyNum;          // Number of the key, needed because of sorting
    bool timeSaved;       // Flag for saving time when the key is pressed
} key[NUM_KEYS];          // Make 12 structs of this type

// Pointer for each key so we can sort without losing keys order
struct keyInformation *pointersToKeys[NUM_KEYS];

void setup()
{
    // Go through each key and make an object
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Create a key object
        keys[i] = new CapacitiveSensor(sendPin, keyPins[i]);

        // Turn off auto calibrate
        keys[i]->set_CS_AutocaL_Millis(0xFFFFFFFF);
    }

    // Set multi-tone switch pin as input
    pinMode(MULTI_TONE_SW_PIN, INPUT_PULLUP);
}

void loop()
{
    // Get selected octave
    int currentOctave = getOctave();

    if (digitalRead(MULTI_TONE_SW_PIN) == HIGH)
    {
        // Play only one key which is last pressed
        playOneKey(currentOctave);
    }
    else
    {
        // Play the arpeggio effect if the multi keys are pressed
        playArpeggio(currentOctave);
    }

    // Stop the tone only if the all keys are released
    stopTone();
}

// Read the current octave from the switch and return the selected octave
int getOctave()
{
    // Read the analog value and get average
    int analogSwitchRead = 0;
    int numSamples = 3;

    // Read numSamples times
    for (int i = 0; i < numSamples; i++)
    {
        analogSwitchRead += analogRead(OCTAVE_SWITCH_PIN);
    }

    // Get average
    analogSwitchRead /= numSamples;

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

// Stop the tone only if the all keys are released
void stopTone()
{
    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Check if the key is pressed
        if (keys[i]->capacitiveSensor(CAPACITIVE_SENSOR_SENSITIVITY) > KEY_PRESS_THRESHOLD)
        {
            // If yes, return from function
            return;
        }
    }

    // If all keys are released, it will go to this part of the code and stop the tone
    noTone(PWM_OUT_PIN);
}

// Play the arpeggio effect if the multi keys are pressed
void playArpeggio(int octave)
{
    // Read pot and map the value to get arpeggio delay
    int arpeggioDelay = map(analogRead(ARPEGGIO_SPEED_POT_PIN), 0, 1023, ARPEGGIO_MIN_DELAY, ARPEGGIO_MAX_DELAY);

    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        // Check if the key is pressed
        if (keys[i]->capacitiveSensor(CAPACITIVE_SENSOR_SENSITIVITY) > KEY_PRESS_THRESHOLD)
        {
            tone(PWM_OUT_PIN, tones[octave][i]);
            delay(arpeggioDelay);
        }
    }
}

// This function reads if the key is pressed and if yes, remember when it's pressed
void refreshKeyTimes()
{
    // Go through each key
    for (int i = 0; i < NUM_KEYS; i++)
    {
        key[i].keyNum = i;

        uint32_t time;

        // Check if the key is pressed
        if (keys[i]->capacitiveSensor(CAPACITIVE_SENSOR_SENSITIVITY) > KEY_PRESS_THRESHOLD)
        {
            // Remember the time when it's pressed
            time = millis();

            // Set presset flag
            key[i].pressed = 1;
        }
        else
        {
            // If the key is not pressed, set the flag for pressed to 0, and reset the save time flag to store the time
            // the next time when it will be pressed again
            key[i].pressed = 0;
            key[i].timeSaved = 0;
        }

        // If the key is pressed and time is not saved, save time for this key
        if (key[i].pressed == 1 && key[i].timeSaved == 0)
        {
            key[i].timePressed = time;
            key[i].timeSaved = 1;
        }
    }
}

// Sort the temporary array of the pointers to the keys in order to the time
// The last pressed key will be first in this array after sorting
void sortKeys()
{
    // First set pointers to point to the key on the same index
    for (int i = 0; i < NUM_KEYS; i++)
    {
        pointersToKeys[i] = &key[i];
    }

    // Sort keys so that pointers to keys are sorted
    for (int i = 0; i < NUM_KEYS - 1; i++)
    {
        for (int j = i + 1; j < NUM_KEYS; j++)
        {
            if (pointersToKeys[j]->timePressed >= pointersToKeys[i]->timePressed)
            {
                // Swap pointers
                struct keyInformation *temp = pointersToKeys[i];
                pointersToKeys[i] = pointersToKeys[j];
                pointersToKeys[j] = temp;
            }
        }
    }
}

// Play only one key which is last pressed
void playOneKey(int octave)
{
    refreshKeyTimes();
    sortKeys();

    for (int i = 0; i < NUM_KEYS; i++)
    {
        if (pointersToKeys[i]->pressed == 1)
        {
            tone(PWM_OUT_PIN, tones[octave][pointersToKeys[i]->keyNum]);
            break;
        }
    }
}
