#pragma once

#include "sound.h"
//#include "sound_command.h"
using AudioBuffer = int32_t[AUDIO_SAMPLES_PER_BUFFER];

class Patch {
public:
    virtual ~Patch() {}
    virtual void mix(AudioBuffer out_buffer, AudioBuffer in_buffer = 0) = 0;
    //virtual void command(SoundCommand c, int p1, int p2, int p3) = 0;
};

