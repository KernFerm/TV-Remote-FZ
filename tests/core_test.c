/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "../tv_remote_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(tvr_normalize("Vol_up") == TvrActionVolumeUp);
    assert(tvr_normalize("VolumeUp") == TvrActionVolumeUp);
    assert(tvr_normalize("VOL+") == TvrActionVolumeUp);
    assert(tvr_normalize("volume-down") == TvrActionVolumeDown);
    assert(tvr_normalize("Vol_dn") == TvrActionVolumeDown);
    assert(tvr_normalize("CH+") == TvrActionChannelUp);
    assert(tvr_normalize("Input / Source") == TvrActionInput);
    assert(tvr_normalize("HDMI 3") == TvrActionHdmi3);
    assert(tvr_normalize("Input_av") == TvrActionAv);
    assert(tvr_normalize("Component 2") == TvrActionComponent2);
    assert(tvr_normalize("Enter") == TvrActionOk);
    assert(tvr_normalize("Num_enter") == TvrActionNumEnter);
    assert(tvr_normalize("RETURN") == TvrActionBack);
    assert(tvr_normalize("Fast Forward") == TvrActionFastForward);
    assert(tvr_normalize("digit_9") == TvrActionNum9);
    assert(tvr_normalize("Settings") == TvrActionSettings);
    assert(tvr_normalize("Netflix") == TvrActionOther);
    assert(tvr_action_page(TvrActionPower) == TvrPageQuick);
    assert(tvr_action_page(TvrActionGuide) == TvrPageChannel);
    assert(tvr_action_page(TvrActionLeft) == TvrPageNavigation);
    assert(tvr_action_page(TvrActionNum0) == TvrPageNumbers);
    assert(tvr_action_page(TvrActionRecord) == TvrPageMedia);
    assert(tvr_action_page(TvrActionHdmi1) == TvrPageInputs);
    assert(tvr_action_page(TvrActionOther) == TvrPageExtra);
    assert(tvr_action_repeats(TvrActionVolumeUp));
    assert(tvr_action_repeats(TvrActionChannelDown));
    assert(!tvr_action_repeats(TvrActionPower));
    char label[8];
    tvr_safe_label("Long television name", label, sizeof(label));
    assert(strcmp(label, "Long te") == 0);
    const char odd[] = {'A', 1, 'B', 0};
    tvr_safe_label(odd, label, sizeof(label));
    assert(strcmp(label, "A?B") == 0);
    puts("TV Remote core tests: PASS");
    return 0;
}
