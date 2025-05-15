#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"
#include "patch.h"
#include "filter.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Noise {
public:
    void mix(int32_t *out_buffer, int volume) {
        if(volume == 0) volume = 1;
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) 
            out_buffer[i] = (random() % (volume * 2)) - volume;
    }
};
class Amp {
public:
    void mix(int32_t *out_buffer, int volume) {
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) 
            out_buffer[i] *= volume;
    }
};

class Wind {
public:
    int vol = 50000;
    Noise main_noise;
    Lop lp1{150}, lp2{150};
    Bandpass bp1{360, 2, 2};
    Noise vn_noise;
    Lop vn_lp1{20};
    int vn_vol1 = 6;
    Lop vn_lp2{4};
    int vn_vol2 = 4;
    Amp amp;
    AudioBuffer buf, buf2;
    void mix(int32_t *out_buffer) {
        main_noise.mix(buf, vol);
        lp1.filter(buf);
        lp2.filter(buf);
        bp1.filter(buf);

        vn_noise.mix(buf2, 32767);
        vn_lp1.filter(buf2);
        amp.mix(buf2, vn_vol1);
        vn_lp2.filter(buf2);
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            out_buffer[i] += (buf[i] * (10000 + vn_vol2 * buf2[i])) / 10000;
        }
    }
};

class Crackle {
public:
    Bandpass bp1, bp2, bp3;
    //Noise noise;
    //Enveloppe env;
    inline static int vol = 1000;
    inline static int rnd = 7000;
    Crackle(int f1, int f2, int f3): bp1(f1, 2, 1), bp2(f2, 2, 2), bp3(f3, 5, 15) {}
    void mix(int32_t *out_buffer) {
        AudioBuffer buf;//, buf2;
        if(rnd <= 0) rnd = 1;
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            if(random() % rnd == 0) {
                int v = random() % 2048;
                v = (v * v) / 2048;
                buf[i] = (vol * v) / 2048;
            } else buf[i] = 0;
        }
        //noise.mix(buf2, 23768);
        //env.mix_squ(buf, buf2);

        //bp1.setFQ(700 + (random() % 700), 2, 1);
        bp1.mix(buf);
        //bp2.setFQ(3000 + (random() % 3000), 2, 2);
        bp2.mix(buf);
        //bp3.setFQ(7000 + (random() % 3000), 5, 15);
        //bp3.mix(buf);
        bp3.mix(out_buffer, buf);
        /*for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            out_buffer[i] += buf[i];
        }*/
    }
};

class Crack {
public:
    Bandpass bp1, bp2, bp3;
    //AudioBuffer buf, buf2;
    Noise noise;
    Enveloppe env;
    bool cut = false;
    inline static int vol = 30000;
    inline static int rnd = 7000;
    Crack(int f1, int f2, int f3): bp1(f1, 1, 10), bp2(f2, 20, 1), bp3(f3, 15, 10) {}
    void mix(int32_t *out_buffer) {
        AudioBuffer buf = {0}, buf2;
        noise.mix(buf2, vol);
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            if((random() % 200) == 0) cut = ((random() % 100) < 90);
            if(cut) buf2[i] = 0;
        }
        env.mix_squ(buf, buf2);
        //bp1.setFQ(700 + (random() % 700), 2, 1);
        bp1.filter(buf2, buf);
        //bp2.setFQ(3000 + (random() % 3000), 2, 2);
        bp2.mix(buf2, buf);
        //bp3.setFQ(7000 + (random() % 3000), 5, 15);
        //bp3.mix(buf);
        bp3.mix(buf2, buf);
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            out_buffer[i] += buf2[i];
        }
    }
    void trig() {
        bp2.setFQ(900 + (random() % 300), 50, 2);
        env.start(0, 0, (random() % 40) + 0);
        cut = false;
    }
};

class MainPatch: public Patch {
public:
    Wind wind;
    Crackle ckl1{810, 3200, 9700}, ckl2{710, 3600, 5700};
    Crack ck1{230, 800, 8000}, ck2{280, 700, 9000};
    MainPatch() {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        wind.mix(out_buffer);
        ckl1.mix(out_buffer);
        ckl2.mix(out_buffer);
        ck1.mix(out_buffer);
        ck2.mix(out_buffer);
    }
};
