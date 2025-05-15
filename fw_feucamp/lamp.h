// pixel led "lamp"

#pragma once
#include "fraise.h"
#include <stdlib.h>
#include "pixel.h"
#include <vector>

#define MAX_LAMPS 200
#define MAX_GROUPS 20

#ifndef CLIP
#define CLIP(x, min, max) (MIN(MAX(x, min), max))
#endif

class Lamp {
  private:
    inline static Lamp* lamps[MAX_LAMPS] = {0};
    inline static std::vector<int> groups[MAX_GROUPS];
  protected:
    int id;
    int chan;
    Lamp(int _id, int _chan): id(_id), chan(_chan) { lamps[id] = this;}
    virtual void _compute();
    virtual void _command(const char *data, uint8_t len) {
        switch(data[0]) {
            case 200: // get chan
                printf("l lamp %d: chan %d\n", id, chan);
                break;
            default: ;
        }
    }
    inline void do_command(const char *data, uint8_t len) {
        _command(data, len);
        Lamp::_command(data, len);
    }
    inline static int period_ms = 10; // compute period
  public:
    static void compute() { 
        for(int i = 0; i < MAX_LAMPS; i++) { 
            if(lamps[i]) lamps[i]->_compute();
        }
    }
    static void command(const char *data, uint8_t len) {
        if(!len) return;
        unsigned int i = data[0];
        data++; len--;
        if(len && i < MAX_LAMPS && lamps[i]) {
            lamps[i]->do_command(data, len);
        }
    }
    static void clear_group(int g) {
        if(g >= MAX_GROUPS) return;
        groups[g].clear();
    }
    static void add_to_group(int g, int l) {
        if(g >= MAX_GROUPS) return;
        groups[g].push_back(l);
    }
    static void group_command(const char *data, uint8_t len) {
        if(len < 2) return;
        int g = data[0];
        if(g >= MAX_GROUPS) return;
        data++; len--;
        for(auto l: groups[g]) {
            if(lamps[l]) lamps[l]->do_command(data, len);
        }
    }
    static void group_rnd_flash(const char *data, uint8_t len) {
        if(len < 2) return;
        int g = data[0];
        if(g >= MAX_GROUPS) return;
        data++; len--;
        int l = random() % groups[g].size();
        char comm[] = {10, 0, 0};
        if(lamps[l]) lamps[l]->do_command(comm, sizeof(comm));
    }
    static void config(const char *data, uint8_t len) {
        if(!len) return;
        unsigned int com = data[0];
        data++; len--;
        switch(com) {
            case 0: // period_ms
                period_ms = data[0];
                break;
            case 1: // define group
                if(len > 0) {
                    int g = data[0];
                    data++; len--;
                    if(g >= MAX_GROUPS) break;
                    clear_group(g);
                    int i = 0;
                    while(len--) {
                        add_to_group(g, data[i++]);
                    }
                }
                break;
            /*case 2: // group command
                if(len > 1) {
                    int g = data[0];
                    if(g >= MAX_GROUPS) break;
                    data++; len--;
                    for(auto l: groups[g]) {
                        if(lamps[l]) lamps[l]->do_command(data, len);
                    }
                }
                break;*/
            default: ;
        }
    }
    static void set_period_ms(int ms) {period_ms = ms;}
};

/*class LampFade: public Lamp {
    float val = 0;
    unsigned char target = 0;
    float factor = 1.0; // ms
  public:
    LampFade(int id, int chan): Lamp(id, chan) {}
    void _compute(char *buf){
        val += ((float)target - val) * factor;
        buf[chan] = val;
    }
    void _command(const char *data, uint8_t len) {
        unsigned char command = data[0];
        unsigned int time_constant;
        switch(command) {
            case 0: // target
                target = data[1];
                break;
            case 1 : // time constant ms
                time_constant = data[1] * 256 + data[2];
                if(time_constant < 1) time_constant = 1;
                factor = ((float)period_ms) / time_constant;
                if(factor > 1.0) factor = 1.0;
                printf("factor: %f\n", factor);
                break;
            default: ;
        }
    }
};*/

inline int rnd(int scale) {
    return (int)((unsigned)random() % (unsigned)scale);
}

inline void filter(int &val, int input, int speed, int speed_scale) {
    val += ((input - val) * speed) / speed_scale;
}

inline void ffilter(float &val, int input, int speed, int speed_scale) {
    val += ((input - val) * speed) / (float)speed_scale;
}

class LampFire: public Lamp {
    int colorSpeedUp = 30, colorSpeedDown = 40;
    int colorMaxSpeedUp = 30, colorMaxSpeedDown = 40;
    int colorCountLow = 1, colorCountHigh = 1;
    int colorMaxCountLow = 1, colorMaxCountHigh = 1;
    int colorFiltFactor = 20; // percent
    int colorCount = 0;
    int colorFilt = 0, color = 0; // 0 - 65535
    int colorTarget = 30000;
    int colorRandom = 80; // percent

    int intensSpeed = 45; // 0 - 255
    int intensMax = 0; //255 * 16;
    int intensMin = 10;
    int intens = 0, intensFilt = 0; // 0 - 4095
    int gamma = 0;
    struct { unsigned char r, g, b, w; } colorLow = {255, 0, 0, 0}, colorHigh = {255, 180, 0, 0};
    float r, g, b, w;
    int man_speed = 50; // 0 - 255
    int flash_count;
    int flash_duration_ms = 700;
    enum {none, armed, done} flash_state = none;
    enum {low, up, high, down} col_state = up;
    unsigned char ncolors = 4;

    void recalc_params() {
        #define rndmix(val, percent) (rnd(val * percent) + val * (100 - percent))

        colorSpeedUp = rndmix(colorMaxSpeedUp, colorRandom); // colorSpeedUp ~= 100*colorMaxSpeedUp
        if(colorSpeedUp < 200) colorSpeedUp = 200;

        colorSpeedDown = rndmix(colorMaxSpeedDown, colorRandom);
        if(colorSpeedDown < 200) colorSpeedDown = 200;

        colorTarget = rndmix(65536, colorRandom) / 100;
        colorCountLow = rndmix(colorMaxCountLow, colorRandom) / 100;
        colorCountHigh = rndmix(colorMaxCountHigh, colorRandom) / 100;
    }

    inline void setbuf(int chan, unsigned char r, unsigned char g, unsigned char b, unsigned char w) {
        set_pixel(chan, r, g, b);
    }

    void _compute() override {
        if(flash_state != none) {
            if(flash_state == armed) {
                if(flash_count == 0) {
                    int r = random() % 256;
                    int g = random() % 128;
                    int b = MIN(r, g) / 8;
                    setbuf(chan, r, g, b, r);
                    flash_count = flash_duration_ms / period_ms;
                    flash_state = done;
                    return;
                }
                else flash_count--;
            }
            else if(flash_state == done) {
                float v = ((float)flash_count * period_ms) / flash_duration_ms;
                v = v * v * 255;
                v = CLIP(v, 0, 255);
                setbuf(chan, v, v, v / 8, v);
                if(flash_count == 0) {
                    flash_state = none;
                    setbuf(chan, 0, 0, 0, 0);
                }
                else flash_count--;
                return;
            }
            setbuf(chan, 0, 0, 0, 0);
            intens = intensFilt = 0;
            return;
        }
        switch(col_state) {
            case up:
                color += colorSpeedUp;
                if(color > colorTarget) {
                    if(color > 65535) color = 65535;
                    recalc_params();
                    col_state = high;
                    colorCount = 0;
                }
                break;
            case high:
                if(colorCount >= colorCountHigh) {
                    col_state = down;
                } else colorCount++;
                break;
            case down:
                color -= colorSpeedDown;
                if(color < 0) {
                    color = 0;
                    col_state = low;
                    colorCount = 0;
                }
                break;
            case low:
                if(colorCount >= colorCountLow) {
                    col_state = up;
                } else colorCount++;
                break;
            default: ;
        }
        colorFilt += (color - colorFilt) * colorFiltFactor / 100;
        int col_gamma = colorFilt / (gamma * (1.0 - colorFilt / 65536.0) + 1.0);

        int intens_rnd = intensMax;
        if(intensMax > intensMin) intens_rnd = (1.5 * rnd(intensMax - intensMin)) + intensMin;
        intens += ((intens_rnd - intens) * intensSpeed) / 256;
        if(intens < 0) intens = 0;
        if(intens > 4095) intens = 4095;
        intensFilt += ((intens - intensFilt) * intensSpeed) / 256;

        #define MIX(c) ((((65535 - col_gamma) * colorLow.c + col_gamma * colorHigh.c) / 256) * intensFilt) / (4096 * 256);
        #define MIXCLIP(c) do{c = MIX(c); c = CLIP(c, 0, 255);} while(0)
        MIXCLIP(r);
        MIXCLIP(g);
        MIXCLIP(b);
        MIXCLIP(w);
        setbuf(chan, r, g, b, w);
    }
    void _command(const char *data, uint8_t len) override {
        unsigned char command = data[0];
        switch(command) {
            case 0: // intens
                intensMax = data[1] * 16;
                if(len > 2) intensMin = data[2] * 16;
                break;
            case 1: // config
                //if(len < 8) break;
                colorMaxSpeedUp     = data[1];
                colorMaxSpeedDown   = data[2];
                colorMaxCountLow    = data[3];
                colorMaxCountHigh   = data[4];
                colorRandom         = data[5];
                colorFiltFactor     = 100 - data[6];
                colorFiltFactor = CLIP(colorFiltFactor, 1, 100);
                intensSpeed         = data[7];
                if(intensSpeed < 2) intensSpeed = 2;
                break;
            case 2: // colors
                if(len < 9) break;
                colorLow.r  = data[1];
                colorLow.g  = data[2];
                colorLow.b  = data[3];
                colorLow.w  = data[4];
                colorHigh.r = data[5];
                colorHigh.g = data[6];
                colorHigh.b = data[7];
                colorHigh.w = data[8];
                break;
            case 3: // gamma
                gamma = data[1] / 8;
                break;
            case 10: //flash
                flash_state = armed;
                flash_count = rnd((data[1] * 10) / period_ms);  // random delay (x 10ms)
                flash_duration_ms = data[2] * 10;               // duration (x 10ms)
                break;
            case 100: // stats
                printf("l lamp %d intensFilt:%d\n", id, intensFilt);
                break;
            default: ;
        }
    }

  public:
    LampFire(int id, int chan, unsigned char ncolors): Lamp(id, chan), ncolors(ncolors) { ncolors = CLIP(ncolors, 1, 4);}
};
