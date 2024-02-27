#include <avr/io.h>
#include <util/delay.h>

#define SPEAKER_PORT PORTC
#define SPEAKER_DDR DDRC
#define SPEAKER_PIN PC0

// initializes the speaker
void speakerInit()
{
    SPEAKER_DDR |= (1 << SPEAKER_PIN);
}

void playToneLow()
{
    unsigned int duration = 1500;
    unsigned int frequency = 2000;
    unsigned int i;
    unsigned long delay = F_CPU / frequency / 2; // Calculate the delay value

    unsigned long num_cycles = frequency * duration / 1000; // Calculate the number of cycles

    for (i = 0; i < num_cycles; i++)
    {
        SPEAKER_PORT |= (1 << SPEAKER_PIN);  // Set PD5 high to turn on the buzzer
        _delay_us(delay);                    // Delay for half the period
        SPEAKER_PORT &= ~(1 << SPEAKER_PIN); // Set PD5 low to turn off the buzzer
        _delay_us(delay);                    // Delay for half the period
    }
}

void playToneHigh()
{
    unsigned int duration = 200;
    unsigned int frequency = 12000;
    unsigned int i;
    unsigned long delay = F_CPU / frequency / 2; // Calculate the delay value

    unsigned long num_cycles = frequency * duration / 1000; // Calculate the number of cycles

    for (i = 0; i < num_cycles; i++)
    {
        SPEAKER_PORT |= (1 << SPEAKER_PIN);  // Set PD5 high to turn on the buzzer
        _delay_us(delay);                    // Delay for half the period
        SPEAKER_PORT &= ~(1 << SPEAKER_PIN); // Set PD5 low to turn off the buzzer
        _delay_us(delay);                    // Delay for half the period
    }
}

void playToneHigher()
{
    unsigned int duration = 200;
    unsigned int frequency = 18000;
    unsigned int i;
    unsigned long delay = F_CPU / frequency / 2; // Calculate the delay value

    unsigned long num_cycles = frequency * duration / 1000; // Calculate the number of cycles

    for (i = 0; i < num_cycles; i++)
    {
        SPEAKER_PORT |= (1 << SPEAKER_PIN);  // Set PD5 high to turn on the buzzer
        _delay_us(delay);                    // Delay for half the period
        SPEAKER_PORT &= ~(1 << SPEAKER_PIN); // Set PD5 low to turn off the buzzer
        _delay_us(delay);                    // Delay for half the period
    }
}