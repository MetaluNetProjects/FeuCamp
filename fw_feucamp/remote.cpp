// Remote control

#include "fraise.h"
#include "string.h"
#include "sound/audiolayer.h"
#include "lamp.h"
#include "sound/main_patch.h"

#include <algorithm>

#define printf fraise_printf

extern AudioLayer audio;
MainPatch &patch = static_cast<MainPatch&>(audio.get_patch());

uint8_t volume;
uint8_t soufflevolume;
uint8_t gresillvolume;
uint8_t crepitvolume;
uint8_t intens;
uint8_t crepit;
uint8_t rouge;

static bool is_int(const char *str, int *result)
{
   char * p;
   *result = (int)strtol(str, &p, 10);
   return (*p == 0);
}

static void command_set_volume(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    volume = (uint8_t)val;
    audio.set_volume(volume);
}

static void command_set_soufflevolume(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    soufflevolume = (uint8_t)val;
    patch.wind.vol = (50000 * soufflevolume) / 256;
}

static void command_set_gresillvolume(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    gresillvolume = (uint8_t)val;
    Crackle::vol = (3000 * gresillvolume) / 256;
}

static void command_set_crepitvolume(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    crepitvolume = (uint8_t)val;
    Crack::vol = (30000 * crepitvolume) / 256;
}

static void command_set_intens(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    intens = val;
    char comm[] = {0, 0, intens, 10};
    Lamp::group_command(comm, sizeof(comm));
}

static void command_set_crepit(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    crepit = val;
}

static void command_set_rouge(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    rouge = val;
    char comm[] = {0, 3, rouge};
    Lamp::group_command(comm, sizeof(comm));
}

static void command_get(const char* command) {
    if(!strcmp(command, "volume")) printf("R volume %d\n", volume);
    else if(!strcmp(command, "soufflevolume")) printf("R soufflevolume %d\n", soufflevolume);
    else if(!strcmp(command, "gresillvolume")) printf("R gresillvolume %d\n", gresillvolume);
    else if(!strcmp(command, "crepitvolume")) printf("R crepitvolume %d\n", crepitvolume);
    else if(!strcmp(command, "intens")) printf("R intens %d\n", intens);
    else if(!strcmp(command, "crepit")) printf("R crepit %d\n", crepit);
    else if(!strcmp(command, "rouge")) printf("R rouge %d\n", rouge);
}

static void command_set(char* command) {
    char *token = strsep(&command, " ");
    if(!strcmp(token, "volume")) command_set_volume(command);
    else if(!strcmp(token, "soufflevolume")) command_set_soufflevolume(command);
    else if(!strcmp(token, "gresillvolume")) command_set_gresillvolume(command);
    else if(!strcmp(token, "crepitvolume")) command_set_crepitvolume(command);
    else if(!strcmp(token, "intens")) command_set_intens(command);
    else if(!strcmp(token, "crepit")) command_set_crepit(command);
    else if(!strcmp(token, "rouge")) command_set_rouge(command);
    command_get(token);
}

void remote_command(const char* com) {
    char command[256];
    strncpy(command, com, sizeof(command) - 1);
    char* commandp = command;
    char *token = strsep(&commandp, " ");
    if(!strcmp(token, "set")) command_set(commandp);
    else if(!strcmp(token, "get")) command_get(commandp);
}

inline uint8_t clip(uint8_t x, uint8_t l, uint8_t h) {
    return std::clamp(x, l, h);
}

void flash() {
    static const char comm[] = {0, 0, 0};
    Lamp::group_rnd_flash(comm, sizeof(comm));
}

void remote_update() {
    static absolute_time_t next_time = get_absolute_time();
    if(time_reached(next_time)) {
        next_time = make_timeout_time_ms(5);
        if(intens * crepit == 0) return;
        int intensC = clip(intens, 1, 50);
        int crepitC = clip(crepit, 1, 255);
        int rnd = 10000 / clip(intensC * crepitC / 256, 1, 100);
        if(random() % rnd == 0) {
            char comm[] = {3, 1};
            if(random() % 2) comm[1] = 2;
            audio.receivebytes(comm, sizeof(comm));
            flash();
        }
    }
}

void remote_init() {
    remote_command("set volume 250");
    remote_command("set soufflevolume 25");
    remote_command("set gresillvolume 25");
    remote_command("set crepitvolume 250");
    remote_command("set intens 50");
    remote_command("set crepit 100");
}
