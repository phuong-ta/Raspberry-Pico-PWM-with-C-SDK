#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

typedef struct IO_PIN {
int pin_number;
bool input;
//bool invert;
} PinData; // input (pinNumber, 1,0) // output (pinNumber, 0,0)

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

    // Loop forever
    while (true) {

        if (isClicked(&sw1) && sw1_flag==0)
        {
            sw1_flag=1;
            isOn = !isOn;
            printf("Toggle value %d\n", isOn); // gpio_put(led_pin, false); set led off
            if(isOn==1)
            {
                
                if (pwm_level==0)
                {
                    pwm_level=64;
                }
                
                // press sw2 ==> pwm level --1
                if (isClicked(&sw2))
                {
                    pwm_level-=1;
                    if (pwm_level<=0)
                    {
                        pwm_level=0;
                    }
                    printf("PWM Level: %u\r\n", pwm_level);
                }
                // press sw0 ==> pwm level ++1
                if (isClicked(&sw0))
                {
                    pwm_level+=1;
                    if (pwm_level>=125)
                    {
                        pwm_level=125;
                    }
                    printf("PWM Level: %u\r\n", pwm_level);
                }
            }
            else
            {
                gpio_put(led.pin_number, false);
            }
            

        }else if(!isClicked(&sw1)){
            sw1_flag=0;
        }


        sleep_ms(100);
    }
}