#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define PWM_MAX_VALUE 125
#define PWM_MIN_VALUE 0
typedef struct IO_PIN {
int pin_number;
bool input;
//bool invert;
} PinData; // input (pinNumber, 1,0) // output (pinNumber, S0,0)

void InitialPin(const PinData  *pin){
    gpio_init(pin->pin_number);
    if (pin->input)
    {
        gpio_set_dir(pin->pin_number, GPIO_IN);
        gpio_pull_up(pin->pin_number);
    }
    else
    {
        gpio_set_dir(pin->pin_number, GPIO_OUT);
    }
}

bool isClicked(PinData  *pin){
    bool pressed;
    if (!gpio_get(pin->pin_number))
    {
        pressed=1;
    }
    else if (gpio_get(pin->pin_number))
    {
        pressed=0;
    }
    return pressed;
}



int main() {

    PinData sw0 = { 9, 1};
    PinData sw1 = { 8, 1};
    PinData sw2 = { 7, 1};
    PinData led = { 22, 0};
    bool sw1_flag = 0;

    uint pwm_level = 0;
    bool isOn =0;


    // Initialize Pins
    InitialPin(&sw0);
    InitialPin(&sw1);
    InitialPin(&sw2);
    InitialPin(&led);
    // Initialize chosen serial port
    stdio_init_all();

    gpio_set_function(led.pin_number, GPIO_FUNC_PWM);
 
    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(led.pin_number);
    pwm_set_wrap(slice_num, PWM_MAX_VALUE);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    // Loop forever
    while (true) {
        // use flag to check is sw1 pressed (on click) --> then change state of LED
        if (isClicked(&sw1) && sw1_flag==0)
        {
            sw1_flag=1;
            isOn = !isOn;
            printf("Toggle value %d\n", isOn); // gpio_put(led_pin, false); set led off          

        }else if(!isClicked(&sw1)){
            sw1_flag=0;
        }

        // turn on led
        if(isOn==1)
        {
            if (pwm_level==0)
            {
                pwm_level=(PWM_MAX_VALUE+1)/2;
            }
            pwm_set_chan_level(slice_num, PWM_CHAN_A, pwm_level);  
            // press sw2 ==> pwm level --1
            if (isClicked(&sw2))
            {
                pwm_level-=1;
                if (pwm_level<=PWM_MIN_VALUE)
                {
                    pwm_level=PWM_MIN_VALUE;
                }
                printf("PWM Level: %u\r\n", pwm_level);
            }
                // press sw0 ==> pwm level ++1
            if (isClicked(&sw0))
            {
                pwm_level+=1;
                if (pwm_level>=PWM_MAX_VALUE)
                {
                    pwm_level=PWM_MAX_VALUE;
                }
                printf("PWM Level: %u\r\n", pwm_level);
            }
        }
        // turn off led
        else
        {
            //gpio_put(led.pin_number, false);
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 0); 
        }

        sleep_ms(50);
    }
}