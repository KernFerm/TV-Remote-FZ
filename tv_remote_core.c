/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "tv_remote_core.h"

#include <ctype.h>
#include <string.h>

typedef struct {
    const char* key;
    TvrAction action;
} TvrAlias;

static const TvrAlias aliases[] = {
    {"power", TvrActionPower},
    {"poweronoff", TvrActionPower},
    {"onoff", TvrActionPower},
    {"mute", TvrActionMute},
    {"volumemute", TvrActionMute},
    {"input", TvrActionInput},
    {"source", TvrActionInput},
    {"inputsource", TvrActionInput},
    {"volup", TvrActionVolumeUp},
    {"volumeup", TvrActionVolumeUp},
    {"volumeplus", TvrActionVolumeUp},
    {"voldown", TvrActionVolumeDown},
    {"volumedown", TvrActionVolumeDown},
    {"volumeminus", TvrActionVolumeDown},
    {"chup", TvrActionChannelUp},
    {"chnext", TvrActionChannelUp},
    {"channelup", TvrActionChannelUp},
    {"channelplus", TvrActionChannelUp},
    {"chdown", TvrActionChannelDown},
    {"chprev", TvrActionChannelDown},
    {"channeldown", TvrActionChannelDown},
    {"channelminus", TvrActionChannelDown},
    {"last", TvrActionPreviousChannel},
    {"previouschannel", TvrActionPreviousChannel},
    {"prevchannel", TvrActionPreviousChannel},
    {"recall", TvrActionPreviousChannel},
    {"guide", TvrActionGuide},
    {"epg", TvrActionGuide},
    {"info", TvrActionInfo},
    {"display", TvrActionInfo},
    {"favorite", TvrActionFavorite},
    {"favorites", TvrActionFavorite},
    {"fav", TvrActionFavorite},
    {"tv", TvrActionTv},
    {"closedcaption", TvrActionCc},
    {"closedcaptions", TvrActionCc},
    {"cc", TvrActionCc},
    {"subtitle", TvrActionSubtitle},
    {"subtitles", TvrActionSubtitle},
    {"audio", TvrActionAudio},
    {"up", TvrActionUp},
    {"navup", TvrActionUp},
    {"cursorup", TvrActionUp},
    {"down", TvrActionDown},
    {"navdown", TvrActionDown},
    {"cursordown", TvrActionDown},
    {"left", TvrActionLeft},
    {"navleft", TvrActionLeft},
    {"cursorleft", TvrActionLeft},
    {"right", TvrActionRight},
    {"navright", TvrActionRight},
    {"cursorright", TvrActionRight},
    {"ok", TvrActionOk},
    {"enter", TvrActionOk},
    {"select", TvrActionOk},
    {"confirm", TvrActionOk},
    {"home", TvrActionHome},
    {"smarthub", TvrActionHome},
    {"menu", TvrActionMenu},
    {"back", TvrActionBack},
    {"return", TvrActionBack},
    {"exit", TvrActionExit},
    {"0", TvrActionNum0},
    {"num0", TvrActionNum0},
    {"number0", TvrActionNum0},
    {"digit0", TvrActionNum0},
    {"1", TvrActionNum1},
    {"num1", TvrActionNum1},
    {"number1", TvrActionNum1},
    {"digit1", TvrActionNum1},
    {"2", TvrActionNum2},
    {"num2", TvrActionNum2},
    {"number2", TvrActionNum2},
    {"digit2", TvrActionNum2},
    {"3", TvrActionNum3},
    {"num3", TvrActionNum3},
    {"number3", TvrActionNum3},
    {"digit3", TvrActionNum3},
    {"4", TvrActionNum4},
    {"num4", TvrActionNum4},
    {"number4", TvrActionNum4},
    {"digit4", TvrActionNum4},
    {"5", TvrActionNum5},
    {"num5", TvrActionNum5},
    {"number5", TvrActionNum5},
    {"digit5", TvrActionNum5},
    {"6", TvrActionNum6},
    {"num6", TvrActionNum6},
    {"number6", TvrActionNum6},
    {"digit6", TvrActionNum6},
    {"7", TvrActionNum7},
    {"num7", TvrActionNum7},
    {"number7", TvrActionNum7},
    {"digit7", TvrActionNum7},
    {"8", TvrActionNum8},
    {"num8", TvrActionNum8},
    {"number8", TvrActionNum8},
    {"digit8", TvrActionNum8},
    {"9", TvrActionNum9},
    {"num9", TvrActionNum9},
    {"number9", TvrActionNum9},
    {"digit9", TvrActionNum9},
    {"dash", TvrActionDash},
    {"hyphen", TvrActionDash},
    {"play", TvrActionPlay},
    {"pause", TvrActionPause},
    {"stop", TvrActionStop},
    {"rewind", TvrActionRewind},
    {"rev", TvrActionRewind},
    {"fastforward", TvrActionFastForward},
    {"forward", TvrActionFastForward},
    {"ff", TvrActionFastForward},
    {"previous", TvrActionPrevious},
    {"prev", TvrActionPrevious},
    {"skipback", TvrActionPrevious},
    {"next", TvrActionNext},
    {"skipforward", TvrActionNext},
    {"record", TvrActionRecord},
    {"rec", TvrActionRecord},
    {"settings", TvrActionSettings},
    {"setting", TvrActionSettings},
    {"setup", TvrActionSettings},
};

static void normalize_key(const char* name, char* key, size_t size) {
    size_t out = 0U;
    if(!size) return;
    for(; name && *name && out + 1U < size; ++name) {
        unsigned char c = (unsigned char)*name;
        if(isalnum(c))
            key[out++] = (char)tolower(c);
        else if(c == '+') {
            const char* word = "plus";
            while(*word && out + 1U < size)
                key[out++] = *word++;
        } else if(c == '-') {
            const char* word = "minus";
            while(*word && out + 1U < size)
                key[out++] = *word++;
        }
    }
    key[out] = '\0';
}

TvrAction tvr_normalize(const char* name) {
    char key[64];
    normalize_key(name, key, sizeof(key));
    for(size_t i = 0; i < sizeof(aliases) / sizeof(aliases[0]); ++i)
        if(strcmp(key, aliases[i].key) == 0) return aliases[i].action;
    return TvrActionOther;
}

TvrPage tvr_action_page(TvrAction action) {
    if(action >= TvrActionNum0 && action <= TvrActionDash) return TvrPageNumbers;
    if(action >= TvrActionPlay && action <= TvrActionRecord) return TvrPageMedia;
    if(action >= TvrActionUp && action <= TvrActionExit) return TvrPageNavigation;
    if(action >= TvrActionPreviousChannel && action <= TvrActionAudio) return TvrPageChannel;
    if(action >= TvrActionPower && action <= TvrActionChannelDown) return TvrPageQuick;
    return TvrPageExtra;
}

const char* tvr_action_label(TvrAction a) {
    static const char* const labels[] = {
        "EXTRA", "PWR",   "MUTE", "INPUT", "VOL+", "VOL-", "CH+",   "CH-", "LAST",
        "GUIDE", "INFO",  "FAV",  "TV",    "CC",   "SUB",  "AUDIO", "UP",  "DOWN",
        "LEFT",  "RIGHT", "OK",   "HOME",  "MENU", "BACK", "EXIT",  "0",   "1",
        "2",     "3",     "4",    "5",     "6",    "7",    "8",     "9",   "-",
        "PLAY",  "PAUSE", "STOP", "REW",   "FF",   "PREV", "NEXT",  "REC", "SET"};
    return a < TvrActionCount ? labels[a] : "?";
}

bool tvr_action_repeats(TvrAction action) {
    return action == TvrActionVolumeUp || action == TvrActionVolumeDown ||
           action == TvrActionChannelUp || action == TvrActionChannelDown;
}

void tvr_safe_label(const char* input, char* output, size_t output_size) {
    if(!output_size) return;
    size_t i = 0U;
    while(input && *input && i + 1U < output_size) {
        unsigned char c = (unsigned char)*input++;
        output[i++] = (c >= 32U && c <= 126U) ? (char)c : '?';
    }
    output[i] = '\0';
}
