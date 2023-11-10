#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"


#define PWM_MIN_VALUE 0
#define TOP 1000
#define CLOCK_DIVIDER 125
#define STEP 5
0
#define LED1 22
#define LED2 21
#define LED3 20
#define ENC_A 10
#define ENC_B 11
#define ENC_SW 12

typedef struct IO_PIN {
int pin_number;
bool input;
//bool invert;
} PinData; // input (pinNumber, 1,0) // output (pinNumber, S0,0)

typedef struct ROTARY_DATA{
    bool ROT_A;
    bool ROT_B;
    bool SW;
    bool flag;
} RotaryData;

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
    pwm_config_set_wrap(&config, TOP-1);
    // Initialize PWM with start set to false
    pwm_init(slice_num, &config,false);
    // Set level (CC) → duty cycle
    pwm_set_chan_level(slice_num, channel, 0);
    // Select PWM mode for your pin
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    // Start PWM
    pwm_set_enabled(slice_num, true);
}

static queue_t data_queue;
// call back
void encoder_callback (uint gpio, uint32_t events) {
    static RotaryData data;
    if (gpio == ENC_A)
    {
        if (gpio_get(ENC_B))
        { 
            data.ROT_A=0;  
            data.ROT_B=1;
            data.SW = 0;
            data.flag=0;
        }else{
            data.ROT_A=1;
            data.ROT_B=0;
            data.SW=0;
            data.flag=0;
        }
        queue_try_add(&data_queue, &data);
    }

    if (gpio == ENC_SW && data.flag==0) {
        data.ROT_A=0; 
        data.ROT_B=0;
        data.SW=1;
        data.flag=1;
        queue_try_add(&data_queue, &data);
    }
    else
    {
        data.flag=0;
    }
    
}

int main() {

    int pwm_level=0;
    bool flag = 0;
    bool isOn =0;

    // Initialize chosen serial port
    stdio_init_all();
    queue_init(&data_queue, sizeof(RotaryData), 1);
    uint slice_num1 = pwm_gpio_to_slice_num(LED1);
    uint channel1 = pwm_gpio_to_channel(LED1);
    configure_pwm(LED1, slice_num1, channel1);
    uint slice_num2 = pwm_gpio_to_slice_num(LED2);
    uint channel2 = pwm_gpio_to_channel(LED2);
    configure_pwm(LED2, slice_num2, channel2);
    uint slice_num3 = pwm_gpio_to_slice_num(LED3);
    uint channel3 = pwm_gpio_to_channel(LED3);
    configure_pwm(LED3, slice_num3, channel3);
  

    gpio_init (ENC_SW);     //Initialise a GPIO for (enabled I/O and set func to GPIO_FUNC_SIO)
    gpio_set_dir (ENC_SW,GPIO_IN);
    gpio_pull_up (ENC_SW);

    gpio_init (ENC_A);
    gpio_set_dir (ENC_A,GPIO_IN);
    gpio_disable_pulls (ENC_A);

    gpio_init (ENC_B);
    gpio_set_dir (ENC_B,GPIO_IN);
    gpio_disable_pulls (ENC_B);

    gpio_set_irq_enabled_with_callback (ENC_SW, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    // Set up interrupts for the rotary encoder, pass the pointer to encoder struct as user_data
    gpio_set_irq_enabled (ENC_A, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled (ENC_B, GPIO_IRQ_EDGE_FALL, true);


    // Loop forever
    while (true) {
        RotaryData data;
        while (queue_try_remove(&data_queue, &data)) {
            printf("ROT_A: %d, ROT_B: %d, SW: %d\r\n", data.ROT_A, data.ROT_B, data.SW);  
            if (pwm_level==0 && data.SW==1)
            {
                pwm_level=(TOP/2);            
                isOn =1;
                flag=1;
            }
            else
            {
                if (data.SW==1 && flag==0)
                {
                    isOn = !isOn;
                    printf("Toggle value %d\n", isOn); // gpio_put(led_pin, false); set led off  
                    flag=1;        
                }else {
                    flag=0;
                    //data.SW=0;
                }
            }  
            if (isOn==1)
            { 
                if (data.ROT_A==1)
                {
                    pwm_level-=STEP;
                }

                if (data.ROT_B==true)
                {
                    pwm_level+=STEP;
                }

                if (pwm_level<=0)
                {
                    pwm_level=0;
                }

                if (pwm_level>=1000)
                {
                    pwm_level=TOP;
                }
                printf("PWM_LEVEL:%d\r\n", pwm_level);
                
                pwm_set_chan_level(slice_num1, channel1, pwm_level); 
                pwm_set_chan_level(slice_num2, channel2, pwm_level); 
                pwm_set_chan_level(slice_num3, channel3, pwm_level);
            }
            else
            {
                pwm_set_chan_level(slice_num1, channel1, PWM_MIN_VALUE); 
                pwm_set_chan_level(slice_num2, channel2, PWM_MIN_VALUE); 
                pwm_set_chan_level(slice_num3, channel3, PWM_MIN_VALUE); 
            }
        }
        sleep_ms(50);
    }
}