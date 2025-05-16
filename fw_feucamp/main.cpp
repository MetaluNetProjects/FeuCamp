/**
 * Firelight generator on pixel-led string
 */

#define BOARD pico
#include "fraise.h"
#include "pixel.h"
#include "lamp.h"
#include "sound/audiolayer.h"
#include "remote.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint AUDIO_PWM_PIN = 4;

int ledPeriod = 250;

float cpuload = 0; // %
AudioLayer audio;

void setup() {
    pixel_setup();

    Lamp::set_period_ms(PIXEL_PERIOD_MS);

    // RGB: 
    Lamp::clear_group(0);
    for(int i = 0; i < 15; i++) {
        new LampFire(i, i, 3);
        Lamp::add_to_group(0, i);
    }
    audio.init(AUDIO_PWM_PIN);
    remote_init();
}

void loop(){
    static absolute_time_t nextLed;
    static bool led = false;

    /*if(dmx.transfer_finished()) {
        frame_duration_us = absolute_time_diff_us(fill_frame_start_time, get_absolute_time());
        dmx.transfer_frame(dmxBuf_now, DMX_CHAN_COUNT);
        float load = (100.0 * fill_frame_duration_us) / (float(frame_duration_us));
        cpuload += (load - cpuload) * 0.1;
        dmxBuf_current_index = 1 - dmxBuf_current_index;
        fill_frame();
    }*/
    if(pixel_update()) Lamp::compute();
    if(time_reached(nextLed)) {
        gpio_put(LED_PIN, led = !led);
        nextLed = make_timeout_time_ms(ledPeriod);
    }
    remote_update();
}

void fraise_receivebytes(const char *data, uint8_t len){
    char com = fraise_get_uint8();
    switch(com) {
        case 1: ledPeriod = (int)data[1] * 10; break;
        case 10: audio.receivebytes(data + 1, len - 1); break;
        case 100: Lamp::command(data + 1, len - 1); break;
        case 101: Lamp::config(data + 1, len - 1); break;
        case 102: Lamp::group_command(data + 1, len - 1); break;
        case 103: Lamp::group_rnd_flash(data + 1, len - 1); break;
    /*default: {
            printf("rcvd ");
            for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
            putchar('\n');
        }*/
    }
}

void fraise_receivechars(const char *data, uint8_t len){
    if(data[0] == 'E') { // Echo
        printf("E%s\n", data + 1);
    }
    else if(data[0] == 'L') { // get load
        printf("L %f\n", cpuload);
    }
    else if(data[0] == 'R') { // Remote command
        remote_command(data + 1);
    }
}

