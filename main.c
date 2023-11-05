#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define PWM_MIN_VALUE 0
#define TOP 1000
#define CLOCK_DIVIDER 125
#define STEP 20
#define LED1 22
#define LED2 21
#define LED3 20
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

void configure_pwm(uint gpio, uint slice_num, uint channel) {

   // Default PWM configuration
    pwm_config config = pwm_get_default_config();
    // Stop PWM
    pwm_set_enabled(slice_num, false);
    // Set clock divider
    pwm_config_set_clkdiv_int(&config, CLOCK_DIVIDER);
    // Set wrap (TOP)
    pwm_config_set_wrap(&config, TOP);
    // Initialize PWM with start set to false
    pwm_init(slice_num, &config,false);
    // Set level (CC) → duty cycle
    pwm_set_chan_level(slice_num, channel, (TOP/2));
    // Select PWM mode for your pin
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    // Start PWM
    pwm_set_enabled(slice_num, true);
}


int main() {

    PinData sw0 = { 9, 1};
    PinData sw1 = { 8, 1};
    PinData sw2 = { 7, 1};
    bool sw1_flag = 0;

    int pwm_level = PWM_MIN_VALUE;
    bool isOn =0;

    // Initialize Pins
    InitialPin(&sw0);
    InitialPin(&sw1);
    InitialPin(&sw2);

    // Initialize chosen serial port
    stdio_init_all();

    uint slice_num1 = pwm_gpio_to_slice_num(LED1);
    uint channel1 = pwm_gpio_to_channel(LED1);
    uint slice_num2 = pwm_gpio_to_slice_num(LED2);
    uint channel2 = pwm_gpio_to_channel(LED2);
    uint slice_num3 = pwm_gpio_to_slice_num(LED3);
    uint channel3 = pwm_gpio_to_channel(LED3);
    configure_pwm(LED1, slice_num1, channel1);
    configure_pwm(LED2, slice_num2, channel2);
    configure_pwm(LED3, slice_num3, channel3);
    // Loop forever
    while (true) {
        if (pwm_level==0 && isClicked(&sw1))
        {
            pwm_level=(TOP/2);            
            isOn =1;
            sw1_flag=1;
        }else{
            // use flag to check is sw1 pressed (on click) --> then change state of LED
            if (isClicked(&sw1) && sw1_flag==0)
            {
                sw1_flag=1;
                isOn = !isOn;
                printf("Toggle value %d\n", isOn); // gpio_put(led_pin, false); set led off          

            }else if(!isClicked(&sw1)){
                sw1_flag=0;
            }
        }

            // turn on led
            if(isOn==1)
            {
                pwm_set_chan_level(slice_num1, channel1, pwm_level); 
                pwm_set_chan_level(slice_num2, channel2, pwm_level); 
                pwm_set_chan_level(slice_num3, channel3, pwm_level); 
                // press sw2 ==> pwm level --1
                if (isClicked(&sw2))
                {
                    pwm_level-=STEP;
                    if (pwm_level<=PWM_MIN_VALUE)
                    {
                        pwm_level=PWM_MIN_VALUE;
                    }
                    printf("PWM Level: %u\r\n", pwm_level);
                }
                    // press sw0 ==> pwm level ++s1
                if (isClicked(&sw0))
                {
                    pwm_level+=STEP;
                    if (pwm_level>=TOP)
                    {
                        pwm_level=TOP;
                    }
                    printf("PWM Level: %u\r\n", pwm_level);
                }
            }
            // turn off led
            else
            {
                pwm_set_chan_level(slice_num1, channel1, PWM_MIN_VALUE);
                pwm_set_chan_level(slice_num2, channel2, PWM_MIN_VALUE);
                pwm_set_chan_level(slice_num3, channel3, PWM_MIN_VALUE);   
            }

        sleep_ms(50);
    }
}