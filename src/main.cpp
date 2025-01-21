#include "mbed.h"

// Pin definitions
InterruptIn  ch1_in(p5);     // RC receiver channel input
PwmOut       motor_out(p21); // PWM output to motor driver

// Create a timer to measure the pulse width
Timer pulse_timer;

// Shared variables (volatile because they are updated in interrupts)
volatile float pulse_width_us = 1500.0f;  // default to 1500µs (neutral)
volatile bool  new_pulse     = false;

// Function prototypes
void rise_handler();
void fall_handler();

int main()
{
    // Attach the interrupt handlers for the rising and falling edges
    ch1_in.rise(&rise_handler);
    ch1_in.fall(&fall_handler);
    
    // Initialize the motor PWM period (e.g. 20ms, typical for servo signals, 
    // or shorter for a DC motor driver—adjust as appropriate)
    motor_out.period_ms(2);  // 2ms period → 500Hz frequency (example)

    while (true)
    {
        // Read the current pulse width in microseconds.
        float current_pulse = pulse_width_us; // copy to local

        // Map the pulse width [1000,2000] µs to [0,1] duty cycle
        // Safe-guard if outside normal range:
        if (current_pulse < 1000.0f) {
            current_pulse = 1000.0f;
        } else if (current_pulse > 2000.0f) {
            current_pulse = 2000.0f;
        }

        // Normalize to [0,1]
        float duty = (current_pulse - 1000.0f) / 1000.0f; 
        // 1000µs → duty=0.0, 2000µs → duty=1.0

        // Set the motor PWM duty cycle
        motor_out.write(duty);

        // Wait a small amount of time before next update 
        // (this is not critical if you want constant control)
        wait_us(20000); // 20ms → 50Hz update rate
    }
}

// Rising edge interrupt handler
void rise_handler()
{
    // Reset and start the timer to measure pulse width
    pulse_timer.reset();
    pulse_timer.start();
}

// Falling edge interrupt handler
void fall_handler()
{
    // Stop the timer and read the elapsed time in microseconds
    pulse_timer.stop();
    pulse_width_us = pulse_timer.read_us();
}
