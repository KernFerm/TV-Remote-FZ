/* SPDX-License-Identifier: GPL-3.0-or-later */
/* File format behavior is compatible with Flipper firmware's GPL infrared_signal implementation. */
#include "tv_remote_signal.h"

#include <infrared/worker/infrared_transmit.h>
#include <infrared/worker/infrared_worker.h>
#include <stdlib.h>
#include <string.h>

void tvr_ir_clear(TvrIrSignal* signal) {
    if(!signal) return;
    free(signal->timings);
    memset(signal, 0, sizeof(*signal));
    signal->message.protocol = InfraredProtocolUnknown;
}

static bool tvr_ir_read_body(TvrIrSignal* signal, FlipperFormat* ff) {
    FuriString* type = furi_string_alloc();
    if(!type) return false;
    bool ok = flipper_format_read_string(ff, "type", type);
    if(ok && furi_string_equal_str(type, "parsed")) {
        FuriString* protocol = furi_string_alloc();
        InfraredMessage message = {
            .protocol = InfraredProtocolUnknown, .address = 0U, .command = 0U};
        ok = protocol && flipper_format_read_string(ff, "protocol", protocol);
        if(ok) message.protocol = infrared_get_protocol_by_name(furi_string_get_cstr(protocol));
        if(ok) ok = infrared_is_protocol_valid(message.protocol);
        if(ok)
            ok = flipper_format_read_hex(
                ff, "address", (uint8_t*)&message.address, sizeof(message.address));
        if(ok)
            ok = flipper_format_read_hex(
                ff, "command", (uint8_t*)&message.command, sizeof(message.command));
        if(ok) {
            uint8_t address_bits = infrared_get_protocol_address_length(message.protocol);
            uint8_t command_bits = infrared_get_protocol_command_length(message.protocol);
            uint32_t address_mask = address_bits >= 32U ? UINT32_MAX :
                                                          ((1UL << address_bits) - 1UL);
            uint32_t command_mask = command_bits >= 32U ? UINT32_MAX :
                                                          ((1UL << command_bits) - 1UL);
            ok = message.address == (message.address & address_mask) &&
                 message.command == (message.command & command_mask);
        }
        if(ok) {
            tvr_ir_clear(signal);
            signal->raw = false;
            signal->message = message;
        }
        if(protocol) furi_string_free(protocol);
    } else if(ok && furi_string_equal_str(type, "raw")) {
        uint32_t frequency = 0U, count = 0U;
        float duty = 0.0f;
        ok = flipper_format_read_uint32(ff, "frequency", &frequency, 1U) &&
             flipper_format_read_float(ff, "duty_cycle", &duty, 1U) &&
             flipper_format_get_value_count(ff, "data", &count);
        if(ok)
            ok = count > 0U && count <= MAX_TIMINGS_AMOUNT &&
                 frequency >= INFRARED_MIN_FREQUENCY && frequency <= INFRARED_MAX_FREQUENCY &&
                 duty > 0.0f && duty <= 1.0f;
        uint32_t* timings = ok ? malloc(sizeof(uint32_t) * count) : NULL;
        if(ok) ok = timings && flipper_format_read_uint32(ff, "data", timings, count);
        if(ok) {
            tvr_ir_clear(signal);
            signal->raw = true;
            signal->timings = timings;
            signal->timings_count = count;
            signal->frequency = frequency;
            signal->duty_cycle = duty;
        } else
            free(timings);
    } else
        ok = false;
    furi_string_free(type);
    return ok;
}

bool tvr_ir_read_next(TvrIrSignal* signal, FlipperFormat* ff, FuriString* name, bool* name_found) {
    *name_found = flipper_format_read_string(ff, "name", name);
    if(!*name_found) return false;
    return tvr_ir_read_body(signal, ff);
}

bool tvr_ir_read_index(TvrIrSignal* signal, FlipperFormat* ff, size_t index) {
    FuriString* name = furi_string_alloc();
    if(!name) return false;
    bool ok = false;
    for(size_t i = 0U; i <= index; ++i) {
        if(!flipper_format_read_string(ff, "name", name)) break;
        if(i == index) {
            ok = tvr_ir_read_body(signal, ff);
            break;
        }
    }
    furi_string_free(name);
    return ok;
}

bool tvr_ir_valid(const TvrIrSignal* signal) {
    if(!signal) return false;
    return signal->raw ? signal->timings && signal->timings_count > 0U :
                         infrared_is_protocol_valid(signal->message.protocol);
}

void tvr_ir_transmit(const TvrIrSignal* signal) {
    if(signal->raw)
        infrared_send_raw_ext(
            signal->timings, signal->timings_count, true, signal->frequency, signal->duty_cycle);
    else
        infrared_send(&signal->message, 1);
}

bool tvr_ir_save(FlipperFormat* ff, const TvrIrSignal* signal, const char* name) {
    if(!ff || !signal || !name || !name[0] || !tvr_ir_valid(signal)) return false;
    bool ok = flipper_format_write_comment_cstr(ff, "") &&
              flipper_format_write_string_cstr(ff, "name", name);
    if(signal->raw) {
        ok = ok && flipper_format_write_string_cstr(ff, "type", "raw") &&
             flipper_format_write_uint32(ff, "frequency", &signal->frequency, 1U) &&
             flipper_format_write_float(ff, "duty_cycle", &signal->duty_cycle, 1U) &&
             flipper_format_write_uint32(ff, "data", signal->timings, signal->timings_count);
    } else {
        const char* protocol = infrared_get_protocol_name(signal->message.protocol);
        ok = ok && protocol && flipper_format_write_string_cstr(ff, "type", "parsed") &&
             flipper_format_write_string_cstr(ff, "protocol", protocol) &&
             flipper_format_write_hex(
                 ff, "address", (const uint8_t*)&signal->message.address, 4U) &&
             flipper_format_write_hex(ff, "command", (const uint8_t*)&signal->message.command, 4U);
    }
    return ok;
}
