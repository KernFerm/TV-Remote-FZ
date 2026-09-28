/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TVR_MAX_COMMANDS 224U
#define TVR_NAME_MAX     48U

typedef enum {
    TvrActionOther,
    TvrActionPower,
    TvrActionMute,
    TvrActionInput,
    TvrActionHdmi,
    TvrActionHdmi1,
    TvrActionHdmi2,
    TvrActionHdmi3,
    TvrActionHdmi4,
    TvrActionAv,
    TvrActionComponent,
    TvrActionComponent1,
    TvrActionComponent2,
    TvrActionComponent3,
    TvrActionVolumeUp,
    TvrActionVolumeDown,
    TvrActionChannelUp,
    TvrActionChannelDown,
    TvrActionPreviousChannel,
    TvrActionGuide,
    TvrActionInfo,
    TvrActionFavorite,
    TvrActionTv,
    TvrActionCc,
    TvrActionSubtitle,
    TvrActionAudio,
    TvrActionUp,
    TvrActionDown,
    TvrActionLeft,
    TvrActionRight,
    TvrActionOk,
    TvrActionHome,
    TvrActionMenu,
    TvrActionBack,
    TvrActionExit,
    TvrActionNum0,
    TvrActionNum1,
    TvrActionNum2,
    TvrActionNum3,
    TvrActionNum4,
    TvrActionNum5,
    TvrActionNum6,
    TvrActionNum7,
    TvrActionNum8,
    TvrActionNum9,
    TvrActionDash,
    TvrActionNumEnter,
    TvrActionPlay,
    TvrActionPause,
    TvrActionStop,
    TvrActionRewind,
    TvrActionFastForward,
    TvrActionPrevious,
    TvrActionNext,
    TvrActionRecord,
    TvrActionSettings,
    TvrActionCount,
} TvrAction;

typedef enum {
    TvrPageQuick,
    TvrPageChannel,
    TvrPageNavigation,
    TvrPageNumbers,
    TvrPageMedia,
    TvrPageInputs,
    TvrPageExtra,
    TvrPageCount,
} TvrPage;

typedef struct {
    char name[TVR_NAME_MAX];
    TvrAction action;
    uint16_t file_index;
    bool raw;
} TvrCommand;

TvrAction tvr_normalize(const char* name);
TvrPage tvr_action_page(TvrAction action);
const char* tvr_action_label(TvrAction action);
bool tvr_action_repeats(TvrAction action);
void tvr_safe_label(const char* input, char* output, size_t output_size);
