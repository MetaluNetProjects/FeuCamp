/*
Copyright (C) 2025 Antoine Rousseau
all material Copyright (c) 1997-1999 Miller Puckette.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "m_pd.h"
#include "math.h"
#include <stdlib.h>
#include <map>

#include "compat.h"
#include "sound.h"
#include "sound/audiolayer.h"
#include "sound/main_patch.h"

/* -------------------------- pico sdk compat --------------------------*/
/*absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset {
        .tv_sec = ms / 1000, .tv_usec = (ms % 1000) * 1000
    };
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, < );
}

absolute_time_t at_the_end_of_time { .tv_sec = (long)1e12, .tv_usec = 0};

absolute_time_t get_absolute_time() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}

absolute_time_t boot_time = get_absolute_time();

uint32_t to_ms_since_boot (absolute_time_t t) {
    struct timeval tv;
    timersub(&t, &boot_time, &tv);
    return tv.tv_sec * 1000U + tv.tv_usec / 1000U;
}

uint8_t fraise_get_uint8() {
    return 0;
}*/
/* -------------------------- emulator ------------------------------ */
static t_class *emulator_class;

typedef struct _emulator
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_msgout;
    AudioLayer *x_audio;
    MainPatch *x_patch;
} t_emulator;

t_emulator *instance;

static void *emulator_new(void)
{
    t_emulator *x = (t_emulator *)pd_new(emulator_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    instance = x;
    x->x_audio = new AudioLayer;
    x->x_patch = &dynamic_cast<MainPatch&>(x->x_audio->get_patch());
    return (x);
}

static void emulator_free(t_emulator *x)
{
    delete x->x_audio;
}

static void emulator_anything(t_emulator *x, t_symbol *s, int argc, t_atom *argv)
{
    /*if(s == gensym("sfx")) {
        static std::map<t_symbol*, int> commands;
        if(commands.empty()) {
            for(int i = 0; i < 256; i++) {
                const char *w = get_sound_command_string(i);
                if(w) commands[gensym(w)] = i;
            }
        }
        int com = 0;
        if(argc > 0) {
            if((&argv[0])->a_type == A_SYMBOL) {
                t_symbol *sym = atom_getsymbol(&argv[0]);
                if(commands.count(sym) == 0) {
                    pd_error(x, "%s: no such sfx command!", sym->s_name);
                    return;
                }
                com = commands[atom_getsymbol(&argv[0])];
            }
            else com = atom_getfloat(&argv[0]);
        }
        int p1 = argc > 1 ? atom_getfloat(&argv[1]) : 0;
        int p2 = argc > 2 ? atom_getfloat(&argv[2]) : 0;
        int p3 = argc > 3 ? atom_getfloat(&argv[3]) : 0;
        x->x_audio->command((SoundCommand)com, p1, p2, p3);
    }*/
    if(s == gensym("wind_vol")) {
        x->x_patch->wind.vol = atom_getfloat(&argv[0]);
    }
    else if(s == gensym("wind_vn_vol1")) {
        x->x_patch->wind.vn_vol1 = atom_getfloat(&argv[0]);
    }
    else if(s == gensym("wind_vn_lp1")) {
        x->x_patch->wind.vn_lp1.setFreq(atom_getfloat(&argv[0]));
    }
    else if(s == gensym("wind_vn_vol2")) {
        x->x_patch->wind.vn_vol2 = atom_getfloat(&argv[0]);
    }
    else if(s == gensym("wind_vn_lp2")) {
        x->x_patch->wind.vn_lp2.setFreq(atom_getfloat(&argv[0]));
    }
    else if(s == gensym("wind_bp")) {
        x->x_patch->wind.bp1.setFQ(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]), atom_getfloat(&argv[2]));
    }
    else if(s == gensym("crackle_vol")) {
        Crackle::vol = atom_getfloat(&argv[0]);
    }
    else if(s == gensym("crackle_rnd")) {
        Crackle::rnd = atom_getfloat(&argv[0]);
    }
    else if(s == gensym("crack1")) {
        x->x_patch->ck1.env.start(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]), atom_getfloat(&argv[2]));
    }
    else if(s == gensym("ck1")) {
        x->x_patch->ck1.trig();
    }
    else if(s == gensym("crack1_bp")) {
        int bp_num = CLIP((int)atom_getfloat(&argv[0]), 1, 3);
        Bandpass *bp;
        switch(bp_num) {
            case 1 : bp = &x->x_patch->ck1.bp1; break;
            case 2 : bp = &x->x_patch->ck1.bp2; break;
            case 3 : bp = &x->x_patch->ck1.bp3; break;
            default: return;
        }
        bp->setFQ(atom_getfloat(&argv[1]), atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
    }
    else if(s == gensym("crack2")) {
        x->x_patch->ck2.env.start(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]), atom_getfloat(&argv[2]));
    }
    else if(s == gensym("ck2")) {
        x->x_patch->ck2.trig();
    }
    else if(s == gensym("crack2_bp")) {
        int bp_num = CLIP((int)atom_getfloat(&argv[0]), 1, 3);
        Bandpass *bp;
        switch(bp_num) {
            case 1 : bp = &x->x_patch->ck2.bp1; break;
            case 2 : bp = &x->x_patch->ck2.bp2; break;
            case 3 : bp = &x->x_patch->ck2.bp3; break;
            default: return;
        }
        bp->setFQ(atom_getfloat(&argv[1]), atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
    }
}

static t_int *emulator_perform(t_int *w)
{
    t_emulator *x = (t_emulator *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    int i;
    int32_t intbuf[AUDIO_SAMPLES_PER_BUFFER] = {0};

    x->x_audio->mix(intbuf, 0);
    for (i = 0; i < n; i++)
    {
        *out++ = CLIP(intbuf[i] / 32768.0, -1.0, 1.0);
    }
    return (w + 4);
}

static void emulator_dsp(t_emulator *x, t_signal **sp)
{
    dsp_add(emulator_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/* -------------------------- setup ------------------------------ */

#ifdef __cplusplus
extern "C" {
#endif

void emulator_setup(void)
{
    emulator_class = class_new(gensym("emulator"), (t_newmethod)emulator_new,
                                 (t_method)emulator_free, sizeof(t_emulator), 0, A_NULL);
    class_addanything(emulator_class, emulator_anything);
    class_addmethod(emulator_class, (t_method)emulator_dsp, gensym("dsp"), A_NULL);
}

#ifdef __cplusplus
}
#endif

