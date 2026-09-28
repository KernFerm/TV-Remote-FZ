/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <flipper_format/flipper_format.h>
#include <infrared.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool raw;
    InfraredMessage message;
    uint32_t* timings;
    size_t timings_count;
    uint32_t frequency;
    float duty_cycle;
} TvrIrSignal;

void tvr_ir_clear(TvrIrSignal* signal);
bool tvr_ir_read_next(TvrIrSignal* signal, FlipperFormat* ff, FuriString* name, bool* name_found);
bool tvr_ir_read_index(TvrIrSignal* signal, FlipperFormat* ff, size_t index);
bool tvr_ir_valid(const TvrIrSignal* signal);
void tvr_ir_transmit(const TvrIrSignal* signal);
bool tvr_ir_save(FlipperFormat* ff, const TvrIrSignal* signal, const char* name);
