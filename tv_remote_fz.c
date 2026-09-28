/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "tv_remote_core.h"
#include "tv_remote_icons.h"
#include "tv_remote_signal.h"

#include <dialogs/dialogs.h>
#include <flipper_format/flipper_format.h>
#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <infrared/worker/infrared_worker.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <stream/stream.h>
#include <toolbox/path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Exported in target-f7 API 87.1; omitted by the public umbrella header. */
Stream* flipper_format_get_raw_stream(FlipperFormat* flipper_format);

#define TVR_VERSION        "1.0.2"
#define TVR_MAX_TVS        8U
#define TVR_PATH_MAX       384U
#define TVR_CONFIG         APP_DATA_PATH("tvs.cfg")
#define TVR_CONFIG_TMP     APP_DATA_PATH("tvs.cfg.partial")
#define TVR_CONFIG_BAK     APP_DATA_PATH("tvs.cfg.backup")
#define TVR_DB_PATH        APP_ASSETS_PATH("tv_catalog.bin")
#define TVR_DB_MAGIC       "TVRDB1\0\0"
#define TVR_DB_HEADER_SIZE 32U
#define TVR_DB_BRAND_SIZE  64U
#define TVR_DB_MODEL_SIZE  80U

typedef struct {
    uint32_t brand_count;
    uint32_t model_count;
    uint32_t brand_offset;
    uint32_t model_offset;
    uint32_t payload_offset;
} TvrDbHeader;

typedef struct {
    char name[52];
    uint32_t first_model;
    uint32_t model_count;
} TvrDbBrand;

typedef struct {
    char name[64];
    uint32_t payload_offset;
    uint32_t payload_size;
    uint32_t id;
} TvrDbModel;

typedef enum {
    TvrViewMain,
    TvrViewGroups,
    TvrViewBrands,
    TvrViewModels,
    TvrViewTvs,
    TvrViewRemote,
    TvrViewManage,
    TvrViewCommands,
    TvrViewSettings,
    TvrViewText,
    TvrViewInput,
} TvrViewId;

typedef struct {
    char friendly[TVR_NAME_MAX];
    char path[TVR_PATH_MAX];
} TvrSavedTv;

typedef struct TvrApp TvrApp;
typedef struct {
    TvrApp* app;
    uint32_t revision;
} TvrRemoteModel;

struct TvrApp {
    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    ViewDispatcher* dispatcher;
    Submenu* main_menu;
    Submenu* group_menu;
    Submenu* brand_menu;
    Submenu* model_menu;
    Submenu* tv_menu;
    Submenu* manage_menu;
    Submenu* command_menu;
    VariableItemList* settings;
    Widget* widget;
    TextInput* input;
    View* remote_view;
    TvrIrSignal signal;
    InfraredWorker* ir_worker;
    FuriString* browser_path;
    FuriString* text;
    TvrSavedTv tvs[TVR_MAX_TVS];
    uint8_t tv_count;
    int8_t favorite;
    int8_t active_tv;
    TvrCommand commands[TVR_MAX_COMMANDS];
    uint16_t command_count;
    uint16_t raw_count;
    uint16_t parsed_count;
    uint16_t visible[TVR_MAX_COMMANDS];
    uint16_t visible_count;
    uint16_t selected;
    TvrPage page;
    bool direct_launch;
    bool haptic;
    bool transmitting;
    bool sent_visible;
    bool nav_extras;
    uint32_t sent_tick;
    TvrViewId current;
    TvrViewId text_back;
    char edit_name[TVR_NAME_MAX];
    TvrDbHeader db_header;
    TvrDbBrand db_brand;
    char db_group;
    bool views_added;
};

enum {
    MainFavorite = 1U,
    MainTvs,
    MainFind,
    MainAdd,
    MainSettings,
    MainAbout
};
enum {
    ManageInfo = 1U,
    ManageRename,
    ManageFavorite,
    ManageCommands,
    ManageRemove
};

static void tvr_main_selected(void* context, uint32_t index);

static void tvr_switch(TvrApp* app, TvrViewId view) {
    app->current = view;
    view_dispatcher_switch_to_view(app->dispatcher, view);
}

static void tvr_show(TvrApp* app, const char* title, const char* body, TvrViewId back) {
    widget_reset(app->widget);
    furi_string_printf(app->text, "\e#%s\n%s", title, body);
    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, furi_string_get_cstr(app->text));
    app->text_back = back;
    tvr_switch(app, TvrViewText);
}

static bool tvr_write_all(File* file, const char* data, size_t size) {
    return storage_file_write(file, data, size) == size;
}

static uint32_t tvr_u32le(const uint8_t* value) {
    return (uint32_t)value[0] | ((uint32_t)value[1] << 8U) | ((uint32_t)value[2] << 16U) |
           ((uint32_t)value[3] << 24U);
}

static bool tvr_read_exact(File* file, void* data, size_t size) {
    return storage_file_read(file, data, size) == size && storage_file_get_error(file) == FSE_OK;
}

static bool tvr_db_open(TvrApp* app, File* file) {
    uint8_t data[TVR_DB_HEADER_SIZE];
    if(!storage_file_open(file, TVR_DB_PATH, FSAM_READ, FSOM_OPEN_EXISTING) ||
       !tvr_read_exact(file, data, sizeof(data)) || memcmp(data, TVR_DB_MAGIC, 8U) != 0 ||
       tvr_u32le(data + 8U) != 1U)
        return false;
    app->db_header.brand_count = tvr_u32le(data + 12U);
    app->db_header.model_count = tvr_u32le(data + 16U);
    app->db_header.brand_offset = tvr_u32le(data + 20U);
    app->db_header.model_offset = tvr_u32le(data + 24U);
    app->db_header.payload_offset = tvr_u32le(data + 28U);
    uint64_t size = storage_file_size(file);
    uint64_t brands_end = (uint64_t)app->db_header.brand_offset +
                          (uint64_t)app->db_header.brand_count * TVR_DB_BRAND_SIZE;
    uint64_t models_end = (uint64_t)app->db_header.model_offset +
                          (uint64_t)app->db_header.model_count * TVR_DB_MODEL_SIZE;
    return app->db_header.brand_count > 0U && app->db_header.brand_count <= 4096U &&
           app->db_header.model_count > 0U && app->db_header.model_count <= 100000U &&
           app->db_header.brand_offset == TVR_DB_HEADER_SIZE &&
           app->db_header.model_offset == brands_end &&
           app->db_header.payload_offset == models_end && models_end <= size;
}

static bool tvr_db_read_brand(TvrApp* app, File* file, uint32_t index, TvrDbBrand* brand) {
    if(index >= app->db_header.brand_count) return false;
    uint8_t data[TVR_DB_BRAND_SIZE];
    uint32_t offset = app->db_header.brand_offset + index * TVR_DB_BRAND_SIZE;
    if(!storage_file_seek(file, offset, true) || !tvr_read_exact(file, data, sizeof(data)))
        return false;
    memcpy(brand->name, data, sizeof(brand->name));
    brand->name[sizeof(brand->name) - 1U] = '\0';
    brand->first_model = tvr_u32le(data + 52U);
    brand->model_count = tvr_u32le(data + 56U);
    return brand->name[0] && brand->first_model <= app->db_header.model_count &&
           brand->model_count <= app->db_header.model_count - brand->first_model;
}

static bool tvr_db_read_model(TvrApp* app, File* file, uint32_t index, TvrDbModel* model) {
    if(index >= app->db_header.model_count) return false;
    uint8_t data[TVR_DB_MODEL_SIZE];
    uint32_t offset = app->db_header.model_offset + index * TVR_DB_MODEL_SIZE;
    if(!storage_file_seek(file, offset, true) || !tvr_read_exact(file, data, sizeof(data)))
        return false;
    memcpy(model->name, data, sizeof(model->name));
    model->name[sizeof(model->name) - 1U] = '\0';
    model->payload_offset = tvr_u32le(data + 64U);
    model->payload_size = tvr_u32le(data + 68U);
    model->id = tvr_u32le(data + 72U);
    uint64_t end = (uint64_t)model->payload_offset + model->payload_size;
    return model->name[0] && model->payload_size > 0U &&
           model->payload_offset >= app->db_header.payload_offset &&
           end <= storage_file_size(file);
}

static bool tvr_save_config(TvrApp* app) {
    if(!storage_simply_mkdir(app->storage, APP_DATA_PATH(""))) return false;
    File* file = storage_file_alloc(app->storage);
    if(!file) return false;
    bool ok = storage_file_open(file, TVR_CONFIG_TMP, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    char line[TVR_PATH_MAX + TVR_NAME_MAX + 32U];
    if(ok) {
        int n = snprintf(
            line,
            sizeof(line),
            "TVRFZ1\nD\t%u\nH\t%u\nF\t%d\n",
            app->direct_launch ? 1U : 0U,
            app->haptic ? 1U : 0U,
            (int)app->favorite);
        ok = n > 0 && (size_t)n < sizeof(line) && tvr_write_all(file, line, (size_t)n);
    }
    for(uint8_t i = 0; ok && i < app->tv_count; ++i) {
        int n =
            snprintf(line, sizeof(line), "T\t%s\t%s\n", app->tvs[i].friendly, app->tvs[i].path);
        ok = n > 0 && (size_t)n < sizeof(line) && tvr_write_all(file, line, (size_t)n);
    }
    if(ok) ok = storage_file_sync(file);
    if(storage_file_is_open(file)) storage_file_close(file);
    storage_file_free(file);
    if(!ok) {
        storage_common_remove(app->storage, TVR_CONFIG_TMP);
        return false;
    }
    bool had = storage_file_exists(app->storage, TVR_CONFIG);
    if(had) {
        storage_common_remove(app->storage, TVR_CONFIG_BAK);
        if(storage_common_rename(app->storage, TVR_CONFIG, TVR_CONFIG_BAK) != FSE_OK) {
            storage_common_remove(app->storage, TVR_CONFIG_TMP);
            return false;
        }
    }
    if(storage_common_rename(app->storage, TVR_CONFIG_TMP, TVR_CONFIG) != FSE_OK) {
        if(had) storage_common_rename(app->storage, TVR_CONFIG_BAK, TVR_CONFIG);
        return false;
    }
    if(had) storage_common_remove(app->storage, TVR_CONFIG_BAK);
    return true;
}

static void tvr_sanitize_field(char* text) {
    for(; text && *text; ++text)
        if(*text == '\t' || *text == '\r' || *text == '\n') *text = ' ';
}

static void tvr_load_config(TvrApp* app) {
    app->favorite = -1;
    app->haptic = true;
    File* file = storage_file_alloc(app->storage);
    if(!file || !storage_file_open(file, TVR_CONFIG, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file) storage_file_free(file);
        return;
    }
    uint64_t size64 = storage_file_size(file);
    if(size64 == 0U || size64 > 4095U) {
        storage_file_close(file);
        storage_file_free(file);
        return;
    }
    size_t size = (size_t)size64;
    char* data = malloc(size + 1U);
    if(!data) {
        storage_file_close(file);
        storage_file_free(file);
        return;
    }
    bool ok = storage_file_read(file, data, size) == size &&
              storage_file_get_error(file) == FSE_OK;
    storage_file_close(file);
    storage_file_free(file);
    if(!ok) {
        free(data);
        return;
    }
    data[size] = '\0';
    char* cursor = data;
    char* line = cursor;
    char* newline = strchr(cursor, '\n');
    if(newline) {
        *newline = '\0';
        cursor = newline + 1U;
    } else
        cursor = NULL;
    if(!line || strcmp(line, "TVRFZ1") != 0) {
        free(data);
        return;
    }
    while(cursor && *cursor) {
        line = cursor;
        newline = strchr(cursor, '\n');
        if(newline) {
            *newline = '\0';
            cursor = newline + 1U;
        } else
            cursor = NULL;
        if(!strncmp(line, "D\t", 2U))
            app->direct_launch = line[2] == '1';
        else if(!strncmp(line, "H\t", 2U))
            app->haptic = line[2] != '0';
        else if(!strncmp(line, "F\t", 2U))
            app->favorite = (int8_t)strtol(line + 2U, NULL, 10);
        else if(!strncmp(line, "T\t", 2U) && app->tv_count < TVR_MAX_TVS) {
            char* separator = strchr(line + 2U, '\t');
            if(!separator) continue;
            *separator = '\0';
            strlcpy(app->tvs[app->tv_count].friendly, line + 2U, TVR_NAME_MAX);
            strlcpy(app->tvs[app->tv_count].path, separator + 1U, TVR_PATH_MAX);
            if(app->tvs[app->tv_count].friendly[0] && app->tvs[app->tv_count].path[0])
                app->tv_count++;
        }
    }
    if(app->favorite < 0 || app->favorite >= (int8_t)app->tv_count) app->favorite = -1;
    free(data);
}

static void tvr_rebuild_main(TvrApp* app) {
    submenu_reset(app->main_menu);
    submenu_set_header(app->main_menu, "TV Remote FZ " TVR_VERSION);
    if(app->favorite >= 0)
        submenu_add_item(app->main_menu, "Open Favorite", MainFavorite, tvr_main_selected, app);
    submenu_add_item(app->main_menu, "Set Up TV", MainFind, tvr_main_selected, app);
    submenu_add_item(app->main_menu, "My TVs", MainTvs, tvr_main_selected, app);
    submenu_add_item(app->main_menu, "Add Existing .ir", MainAdd, tvr_main_selected, app);
    submenu_add_item(app->main_menu, "Settings", MainSettings, tvr_main_selected, app);
    submenu_add_item(app->main_menu, "About", MainAbout, tvr_main_selected, app);
}

static bool tvr_read_remote(TvrApp* app, int8_t tv_index) {
    if(tv_index < 0 || tv_index >= (int8_t)app->tv_count) return false;
    FlipperFormat* ff = flipper_format_buffered_file_alloc(app->storage);
    FuriString* header = furi_string_alloc();
    FuriString* name = furi_string_alloc();
    if(!ff || !header || !name) {
        if(ff) flipper_format_free(ff);
        if(header) furi_string_free(header);
        if(name) furi_string_free(name);
        return false;
    }
    bool ok = flipper_format_buffered_file_open_existing(ff, app->tvs[tv_index].path);
    uint32_t version = 0U;
    if(ok)
        ok = flipper_format_read_header(ff, header, &version) &&
             furi_string_equal_str(header, "IR signals file") && version == 1U;
    app->command_count = app->raw_count = app->parsed_count = 0U;
    while(ok && app->command_count < TVR_MAX_COMMANDS) {
        bool name_found = false;
        if(!tvr_ir_read_next(&app->signal, ff, name, &name_found)) {
            if(name_found || !stream_eof(flipper_format_get_raw_stream(ff))) ok = false;
            break;
        }
        TvrCommand* command = &app->commands[app->command_count];
        tvr_safe_label(furi_string_get_cstr(name), command->name, sizeof(command->name));
        if(!command->name[0]) strlcpy(command->name, "Unnamed", sizeof(command->name));
        command->action = tvr_normalize(command->name);
        command->file_index = app->command_count;
        command->raw = app->signal.raw;
        if(command->raw)
            app->raw_count++;
        else
            app->parsed_count++;
        app->command_count++;
    }
    if(ok && app->command_count == TVR_MAX_COMMANDS &&
       flipper_format_read_string(ff, "name", name))
        ok = false;
    ok = ok && app->command_count > 0U;
    flipper_format_free(ff);
    furi_string_free(name);
    furi_string_free(header);
    if(ok) app->active_tv = tv_index;
    return ok;
}

static uint16_t tvr_find_action(const TvrApp* app, TvrAction action) {
    for(uint16_t i = 0; i < app->command_count; ++i)
        if(app->commands[i].action == action) return i;
    return UINT16_MAX;
}

static bool tvr_page_has_commands(const TvrApp* app, TvrPage page) {
    for(uint16_t i = 0; i < app->command_count; ++i)
        if(tvr_action_page(app->commands[i].action) == page) return true;
    return false;
}

static void tvr_build_visible(TvrApp* app) {
    app->visible_count = 0U;
    if(app->page == TvrPageQuick || app->page == TvrPageNumbers || app->page == TvrPageMedia ||
       (app->page == TvrPageNavigation && app->nav_extras)) {
        static const TvrAction quick_order[] = {
            TvrActionPower,
            TvrActionMute,
            TvrActionInput,
            TvrActionVolumeUp,
            TvrActionVolumeDown,
            TvrActionChannelUp,
            TvrActionChannelDown,
        };
        static const TvrAction number_order[] = {
            TvrActionNum1,
            TvrActionNum2,
            TvrActionNum3,
            TvrActionNum4,
            TvrActionNum5,
            TvrActionNum6,
            TvrActionNum7,
            TvrActionNum8,
            TvrActionNum9,
            TvrActionNum0,
            TvrActionDash,
        };
        static const TvrAction media_order[] = {
            TvrActionPlay,
            TvrActionPause,
            TvrActionStop,
            TvrActionRewind,
            TvrActionFastForward,
            TvrActionPrevious,
            TvrActionNext,
            TvrActionRecord,
        };
        static const TvrAction navigation_extra_order[] = {
            TvrActionHome,
            TvrActionMenu,
            TvrActionBack,
            TvrActionExit,
        };
        const TvrAction* order = quick_order;
        size_t order_count = COUNT_OF(quick_order);
        if(app->page == TvrPageNumbers) {
            order = number_order;
            order_count = COUNT_OF(number_order);
        } else if(app->page == TvrPageMedia) {
            order = media_order;
            order_count = COUNT_OF(media_order);
        } else if(app->page == TvrPageNavigation) {
            order = navigation_extra_order;
            order_count = COUNT_OF(navigation_extra_order);
        }
        for(size_t n = 0; n < order_count; ++n) {
            uint16_t found = tvr_find_action(app, order[n]);
            if(found != UINT16_MAX) app->visible[app->visible_count++] = found;
        }
    } else {
        for(uint16_t i = 0; i < app->command_count; ++i)
            if(tvr_action_page(app->commands[i].action) == app->page)
                app->visible[app->visible_count++] = i;
    }
    if(app->selected >= app->visible_count)
        app->selected = app->visible_count ? app->visible_count - 1U : 0U;
}

static void tvr_change_page(TvrApp* app, int direction) {
    TvrPage page = app->page;
    for(uint8_t tries = 0; tries < TvrPageCount; ++tries) {
        page = (TvrPage)((page + TvrPageCount + direction) % TvrPageCount);
        if(tvr_page_has_commands(app, page)) {
            app->page = page;
            app->nav_extras = false;
            app->selected = 0U;
            tvr_build_visible(app);
            break;
        }
    }
}

static bool tvr_load_signal(TvrApp* app, uint16_t command_index) {
    if(app->active_tv < 0 || command_index >= app->command_count) return false;
    FlipperFormat* ff = flipper_format_buffered_file_alloc(app->storage);
    if(!ff) return false;
    bool opened = flipper_format_buffered_file_open_existing(ff, app->tvs[app->active_tv].path);
    bool loaded = opened &&
                  tvr_ir_read_index(&app->signal, ff, app->commands[command_index].file_index);
    flipper_format_free(ff);
    return loaded && tvr_ir_valid(&app->signal);
}

static void tvr_feedback(TvrApp* app) {
    app->sent_visible = true;
    app->sent_tick = furi_get_tick();
    notification_message(app->notifications, &sequence_blink_blue_10);
    if(app->haptic) notification_message(app->notifications, &sequence_single_vibro);
    view_commit_model(app->remote_view, true);
}

static bool tvr_send_once(TvrApp* app, uint16_t command_index) {
    if(!tvr_load_signal(app, command_index)) return false;
    tvr_ir_transmit(&app->signal);
    tvr_feedback(app);
    return true;
}

static bool tvr_start_repeat(TvrApp* app, uint16_t command_index) {
    if(app->transmitting || !tvr_load_signal(app, command_index)) return false;
    if(app->signal.raw)
        infrared_worker_set_raw_signal(
            app->ir_worker,
            app->signal.timings,
            app->signal.timings_count,
            app->signal.frequency,
            app->signal.duty_cycle);
    else
        infrared_worker_set_decoded_signal(app->ir_worker, &app->signal.message);
    infrared_worker_tx_set_get_signal_callback(
        app->ir_worker, infrared_worker_tx_get_signal_steady_callback, app);
    infrared_worker_tx_start(app->ir_worker);
    app->transmitting = true;
    tvr_feedback(app);
    return true;
}

static void tvr_stop_repeat(TvrApp* app) {
    if(!app->transmitting) return;
    infrared_worker_tx_stop(app->ir_worker);
    infrared_worker_tx_set_get_signal_callback(app->ir_worker, NULL, NULL);
    app->transmitting = false;
}

static void tvr_draw_button(
    Canvas* canvas,
    int x,
    int y,
    int w,
    int h,
    const char* label,
    bool selected,
    TvrAction action) {
    if(selected) {
        canvas_draw_rbox(canvas, x, y, w, h, 2);
        canvas_set_color(canvas, ColorWhite);
    } else
        canvas_draw_rframe(canvas, x, y, w, h, 2);
    canvas_set_font(canvas, FontKeyboard);
    if(tvr_icon_available(action)) {
        tvr_icon_draw(canvas, action, x + 7, y + h / 2);
        canvas_draw_str_aligned(
            canvas, x + w / 2 + 5, y + h / 2 + 3, AlignCenter, AlignCenter, label);
    } else
        canvas_draw_str_aligned(canvas, x + w / 2, y + h / 2 + 3, AlignCenter, AlignCenter, label);
    canvas_set_color(canvas, ColorBlack);
}

static void tvr_draw_header(Canvas* canvas, TvrApp* app) {
    char title[22];
    char page[12];
    const char* source = app->active_tv >= 0 ? app->tvs[app->active_tv].friendly : "No TV";
    snprintf(title, sizeof(title), "%.17s", source);
    snprintf(
        page,
        sizeof(page),
        "%s%u/6",
        app->active_tv == app->favorite ? "*" : "",
        (unsigned)app->page + 1U);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 1, 8, title);
    canvas_draw_str_aligned(canvas, 127, 8, AlignRight, AlignBottom, page);
    canvas_draw_line(canvas, 0, 10, 127, 10);
}

static void tvr_remote_draw(Canvas* canvas, void* context) {
    TvrRemoteModel* model = context;
    TvrApp* app = model->app;
    canvas_clear(canvas);
    tvr_draw_header(canvas, app);
    if(app->page == TvrPageNavigation && !app->nav_extras) {
        uint16_t up = tvr_find_action(app, TvrActionUp),
                 down = tvr_find_action(app, TvrActionDown),
                 left = tvr_find_action(app, TvrActionLeft),
                 right = tvr_find_action(app, TvrActionRight),
                 ok = tvr_find_action(app, TvrActionOk);
        canvas_set_font(canvas, FontPrimary);
        if(up != UINT16_MAX)
            canvas_draw_str_aligned(canvas, 64, 23, AlignCenter, AlignCenter, "^");
        if(left != UINT16_MAX)
            canvas_draw_str_aligned(canvas, 40, 39, AlignCenter, AlignCenter, "<");
        if(ok != UINT16_MAX) tvr_draw_button(canvas, 52, 29, 24, 19, "OK", false, TvrActionOk);
        if(right != UINT16_MAX)
            canvas_draw_str_aligned(canvas, 88, 39, AlignCenter, AlignCenter, ">");
        if(down != UINT16_MAX)
            canvas_draw_str_aligned(canvas, 64, 56, AlignCenter, AlignCenter, "v");
        canvas_set_font(canvas, FontKeyboard);
        canvas_draw_str(canvas, 1, 62, "D-pad=TV  hold OK=more");
    } else {
        uint16_t shown = app->visible_count > 12U ? 12U : app->visible_count;
        if(!shown) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas, 64, 36, AlignCenter, AlignCenter, "No commands on page");
        }
        for(uint16_t pos = 0; pos < shown; ++pos) {
            uint16_t offset = app->selected >= 12U ? (app->selected / 12U) * 12U : 0U;
            uint16_t slot = offset + pos;
            if(slot >= app->visible_count) break;
            uint16_t index = app->visible[slot];
            TvrCommand* command = &app->commands[index];
            uint8_t columns = app->page == TvrPageNavigation ? 2U : 3U;
            int col = pos % columns, row = pos / columns;
            int width = app->page == TvrPageNavigation ? 61 : 40;
            int height = app->page == TvrPageNavigation ? 17 : 11;
            int x = app->page == TvrPageNavigation ? 1 + col * 63 : 1 + col * 43;
            int y = app->page == TvrPageNavigation ? 17 + row * 20 : 13 + row * 13;
            tvr_draw_button(
                canvas,
                x,
                y,
                width,
                height,
                command->action == TvrActionOther ? command->name :
                                                    tvr_action_label(command->action),
                slot == app->selected,
                command->action);
        }
        if(app->visible_count > 12U) {
            char count[16];
            snprintf(
                count,
                sizeof(count),
                "%u/%u",
                (unsigned)app->selected + 1U,
                (unsigned)app->visible_count);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(canvas, 127, 63, AlignRight, AlignBottom, count);
        }
    }
    if(app->sent_visible) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 95, 0, 33, 10);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 8, AlignRight, AlignBottom, "TX Sent");
    }
}

static uint16_t tvr_selected_command(const TvrApp* app) {
    return app->visible_count && app->selected < app->visible_count ? app->visible[app->selected] :
                                                                      UINT16_MAX;
}

static bool tvr_remote_input(InputEvent* event, void* context) {
    TvrApp* app = context;
    if(event->type == InputTypeLong &&
       (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        tvr_stop_repeat(app);
        tvr_change_page(app, event->key == InputKeyRight ? 1 : -1);
        view_commit_model(app->remote_view, true);
        return true;
    }
    if(event->type == InputTypeLong && event->key == InputKeyOk) {
        tvr_stop_repeat(app);
        if(app->page == TvrPageNavigation) {
            app->nav_extras = !app->nav_extras;
            app->selected = 0U;
            tvr_build_visible(app);
            view_commit_model(app->remote_view, true);
        } else {
            tvr_switch(app, TvrViewManage);
        }
        return true;
    }
    if(app->page == TvrPageNavigation && !app->nav_extras && event->type == InputTypeShort) {
        TvrAction action = event->key == InputKeyUp    ? TvrActionUp :
                           event->key == InputKeyDown  ? TvrActionDown :
                           event->key == InputKeyLeft  ? TvrActionLeft :
                           event->key == InputKeyRight ? TvrActionRight :
                           event->key == InputKeyOk    ? TvrActionOk :
                                                         TvrActionOther;
        uint16_t index = tvr_find_action(app, action);
        if(index != UINT16_MAX) tvr_send_once(app, index);
        return index != UINT16_MAX;
    }
    uint16_t command = tvr_selected_command(app);
    if(event->key == InputKeyOk && event->type == InputTypePress && command != UINT16_MAX &&
       tvr_action_repeats(app->commands[command].action))
        return tvr_start_repeat(app, command);
    if(event->key == InputKeyOk && event->type == InputTypeRelease && app->transmitting) {
        tvr_stop_repeat(app);
        return true;
    }
    if(event->type != InputTypeShort) return false;
    if(event->key == InputKeyOk && command != UINT16_MAX) {
        if(!tvr_action_repeats(app->commands[command].action)) return tvr_send_once(app, command);
        return true;
    }
    if(!app->visible_count) return false;
    uint16_t old = app->selected;
    uint8_t columns = app->page == TvrPageNavigation ? 2U : 3U;
    if(event->key == InputKeyLeft && app->selected)
        app->selected--;
    else if(event->key == InputKeyRight && app->selected + 1U < app->visible_count)
        app->selected++;
    else if(event->key == InputKeyUp && app->selected >= columns)
        app->selected -= columns;
    else if(event->key == InputKeyDown && app->selected + columns < app->visible_count)
        app->selected += columns;
    if(old != app->selected) {
        view_commit_model(app->remote_view, true);
        return true;
    }
    return false;
}

static void tvr_open_remote(TvrApp* app, int8_t index) {
    if(!tvr_read_remote(app, index)) {
        tvr_show(
            app,
            "Remote rejected",
            "Could not read a valid Flipper IR signals file. The saved file may be missing, malformed, or on an unavailable SD card.",
            TvrViewMain);
        return;
    }
    app->page = TvrPageQuick;
    app->nav_extras = false;
    if(!tvr_page_has_commands(app, app->page)) tvr_change_page(app, 1);
    app->selected = 0U;
    tvr_build_visible(app);
    app->sent_visible = false;
    view_commit_model(app->remote_view, true);
    tvr_switch(app, TvrViewRemote);
}

static void tvr_command_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    if(index < app->command_count) {
        if(!tvr_send_once(app, (uint16_t)index))
            tvr_show(
                app,
                "Send failed",
                "The exact saved signal could not be loaded. Check the SD card and remote file.",
                TvrViewCommands);
    }
}

static void tvr_rebuild_commands(TvrApp* app) {
    submenu_reset(app->command_menu);
    submenu_set_header(app->command_menu, "All original commands");
    for(uint16_t i = 0; i < app->command_count; ++i)
        submenu_add_item(app->command_menu, app->commands[i].name, i, tvr_command_selected, app);
}

static void tvr_rebuild_manage(TvrApp* app);

static void tvr_rename_done(void* context) {
    TvrApp* app = context;
    tvr_sanitize_field(app->edit_name);
    if(app->active_tv >= 0 && app->edit_name[0]) {
        strlcpy(app->tvs[app->active_tv].friendly, app->edit_name, TVR_NAME_MAX);
        if(!tvr_save_config(app)) {
            tvr_show(
                app,
                "Rename not saved",
                "Could not commit settings to microSD. The in-memory name may be lost on exit.",
                TvrViewManage);
            return;
        }
    }
    tvr_rebuild_manage(app);
    tvr_switch(app, TvrViewManage);
}

static void tvr_remove_active(TvrApp* app) {
    int8_t removed = app->active_tv;
    if(removed < 0) return;
    for(uint8_t i = (uint8_t)removed; i + 1U < app->tv_count; ++i)
        app->tvs[i] = app->tvs[i + 1U];
    app->tv_count--;
    if(app->favorite == removed)
        app->favorite = -1;
    else if(app->favorite > removed)
        app->favorite--;
    app->active_tv = -1;
    tvr_save_config(app);
    tvr_rebuild_main(app);
    tvr_switch(app, TvrViewMain);
}

static void tvr_manage_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    if(app->active_tv < 0) return;
    if(index == ManageInfo) {
        char info[620];
        snprintf(
            info,
            sizeof(info),
            "Friendly: %s\nBrand/model: Not stored in .ir format\nSource: %s\nCommands: %u\nParsed: %u\nRAW: %u\n\nSignals are loaded unchanged from this file.",
            app->tvs[app->active_tv].friendly,
            app->tvs[app->active_tv].path,
            (unsigned)app->command_count,
            (unsigned)app->parsed_count,
            (unsigned)app->raw_count);
        tvr_show(app, "Remote information", info, TvrViewManage);
    } else if(index == ManageRename) {
        strlcpy(app->edit_name, app->tvs[app->active_tv].friendly, sizeof(app->edit_name));
        text_input_reset(app->input);
        text_input_set_header_text(app->input, "Friendly TV name");
        text_input_set_minimum_length(app->input, 1U);
        text_input_set_result_callback(
            app->input, tvr_rename_done, app, app->edit_name, sizeof(app->edit_name), true);
        tvr_switch(app, TvrViewInput);
    } else if(index == ManageFavorite) {
        app->favorite = app->active_tv;
        if(!tvr_save_config(app))
            tvr_show(
                app, "Favorite not saved", "Check the SD card and free space.", TvrViewManage);
        else {
            tvr_rebuild_manage(app);
            tvr_rebuild_main(app);
        }
    } else if(index == ManageCommands) {
        tvr_rebuild_commands(app);
        tvr_switch(app, TvrViewCommands);
    } else if(index == ManageRemove)
        tvr_remove_active(app);
}

static void tvr_rebuild_manage(TvrApp* app) {
    submenu_reset(app->manage_menu);
    submenu_set_header(
        app->manage_menu, app->active_tv >= 0 ? app->tvs[app->active_tv].friendly : "Remote");
    submenu_add_item(app->manage_menu, "Remote information", ManageInfo, tvr_manage_selected, app);
    submenu_add_item(
        app->manage_menu, "Rename friendly name", ManageRename, tvr_manage_selected, app);
    submenu_add_item(
        app->manage_menu,
        app->active_tv == app->favorite ? "Favorite (selected)" : "Set as favorite",
        ManageFavorite,
        tvr_manage_selected,
        app);
    submenu_add_item(app->manage_menu, "All commands", ManageCommands, tvr_manage_selected, app);
    submenu_add_item(app->manage_menu, "Remove from app", ManageRemove, tvr_manage_selected, app);
}

static void tvr_tv_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    if(index < app->tv_count) tvr_open_remote(app, (int8_t)index);
}

static void tvr_rebuild_tvs(TvrApp* app) {
    submenu_reset(app->tv_menu);
    submenu_set_header(app->tv_menu, "My TVs");
    for(uint8_t i = 0; i < app->tv_count; ++i)
        submenu_add_item(app->tv_menu, app->tvs[i].friendly, i, tvr_tv_selected, app);
}

static bool
    tvr_accept_remote(TvrApp* app, const char* path, const char* friendly, TvrViewId back) {
    if(app->tv_count >= TVR_MAX_TVS) {
        tvr_show(
            app,
            "TV limit reached",
            "Remove a saved television before adding another. The limit is 8.",
            back);
        return false;
    }
    strlcpy(app->tvs[app->tv_count].path, path, TVR_PATH_MAX);
    tvr_safe_label(friendly, app->tvs[app->tv_count].friendly, TVR_NAME_MAX);
    if(!app->tvs[app->tv_count].friendly[0])
        strlcpy(app->tvs[app->tv_count].friendly, "Television", TVR_NAME_MAX);
    uint8_t added = app->tv_count++;
    if(!tvr_read_remote(app, (int8_t)added)) {
        app->tv_count--;
        tvr_show(
            app, "Remote rejected", "This entry does not contain a valid Flipper IR remote.", back);
        return false;
    }
    if(app->favorite < 0) app->favorite = added;
    if(!tvr_save_config(app)) {
        app->tv_count--;
        if(app->favorite == (int8_t)added) app->favorite = -1;
        tvr_show(
            app,
            "Save failed",
            "The remote was valid but could not be saved. Check the SD card and free space.",
            back);
        return false;
    }
    tvr_rebuild_main(app);
    tvr_open_remote(app, (int8_t)added);
    return true;
}

static void tvr_add_remote(TvrApp* app, const char* start_path, const char* base_path) {
    if(app->tv_count >= TVR_MAX_TVS) {
        tvr_show(
            app,
            "TV limit reached",
            "Remove a saved television before adding another. The limit is 8.",
            TvrViewMain);
        return;
    }
    furi_string_set(app->browser_path, start_path);
    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, ".ir", NULL);
    options.hide_ext = false;
    options.skip_assets = false;
    options.base_path = base_path;
    if(!dialog_file_browser_show(app->dialogs, app->browser_path, app->browser_path, &options))
        return;
    FuriString* filename = furi_string_alloc_set(furi_string_get_cstr(app->browser_path));
    path_extract_filename(filename, filename, true);
    tvr_accept_remote(
        app, furi_string_get_cstr(app->browser_path), furi_string_get_cstr(filename), TvrViewMain);
    furi_string_free(filename);
}

static char tvr_db_group_for(const char* name) {
    char c = name && name[0] ? name[0] : '#';
    if(c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    return c >= 'A' && c <= 'Z' ? c : '#';
}

static bool tvr_db_extract(TvrApp* app, uint32_t model_index, char* path, size_t path_size) {
    File* source = storage_file_alloc(app->storage);
    File* output = storage_file_alloc(app->storage);
    if(!source || !output) {
        if(source) storage_file_free(source);
        if(output) storage_file_free(output);
        return false;
    }
    TvrDbModel model;
    bool ok = tvr_db_open(app, source) && tvr_db_read_model(app, source, model_index, &model);
    char temporary[TVR_PATH_MAX] = {0};
    if(ok) {
        storage_simply_mkdir(app->storage, APP_DATA_PATH(""));
        ok = storage_simply_mkdir(app->storage, APP_DATA_PATH("remotes"));
    }
    if(ok) {
        int n = snprintf(
            path, path_size, APP_DATA_PATH("remotes/db_%06lu.ir"), (unsigned long)model.id);
        int t = snprintf(
            temporary,
            sizeof(temporary),
            APP_DATA_PATH("remotes/db_%06lu.partial"),
            (unsigned long)model.id);
        ok = n > 0 && (size_t)n < path_size && t > 0 && (size_t)t < sizeof(temporary) &&
             storage_file_seek(source, model.payload_offset, true) &&
             storage_file_open(output, temporary, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    }
    uint8_t buffer[512];
    uint32_t remaining = ok ? model.payload_size : 0U;
    while(ok && remaining) {
        size_t amount = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        ok = tvr_read_exact(source, buffer, amount) &&
             storage_file_write(output, buffer, amount) == amount;
        remaining -= ok ? amount : 0U;
    }
    if(ok) ok = storage_file_sync(output);
    if(storage_file_is_open(output)) storage_file_close(output);
    if(storage_file_is_open(source)) storage_file_close(source);
    storage_file_free(output);
    storage_file_free(source);
    if(!ok) {
        if(temporary[0]) storage_common_remove(app->storage, temporary);
        return false;
    }
    storage_common_remove(app->storage, path);
    if(storage_common_rename(app->storage, temporary, path) != FSE_OK) {
        storage_common_remove(app->storage, temporary);
        return false;
    }
    return true;
}

static void tvr_model_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    File* file = storage_file_alloc(app->storage);
    TvrDbModel model;
    bool ok = file && tvr_db_open(app, file) && tvr_db_read_model(app, file, index, &model);
    if(file) {
        if(storage_file_is_open(file)) storage_file_close(file);
        storage_file_free(file);
    }
    char path[TVR_PATH_MAX];
    if(!ok || !tvr_db_extract(app, index, path, sizeof(path))) {
        tvr_show(
            app,
            "Code install failed",
            "The bundled TV database could not be read or the SD card could not save this remote.",
            TvrViewModels);
        return;
    }
    char friendly[TVR_NAME_MAX];
    snprintf(friendly, sizeof(friendly), "%.22s %.23s", app->db_brand.name, model.name);
    tvr_accept_remote(app, path, friendly, TvrViewModels);
}

static void tvr_brand_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    File* file = storage_file_alloc(app->storage);
    bool ok = file && tvr_db_open(app, file) &&
              tvr_db_read_brand(app, file, index, &app->db_brand);
    submenu_reset(app->model_menu);
    if(ok) {
        submenu_set_header(app->model_menu, app->db_brand.name);
        for(uint32_t i = 0U; i < app->db_brand.model_count; ++i) {
            uint32_t model_index = app->db_brand.first_model + i;
            TvrDbModel model;
            if(!tvr_db_read_model(app, file, model_index, &model)) {
                ok = false;
                break;
            }
            submenu_add_item(app->model_menu, model.name, model_index, tvr_model_selected, app);
        }
    }
    if(file) {
        if(storage_file_is_open(file)) storage_file_close(file);
        storage_file_free(file);
    }
    if(!ok) {
        tvr_show(app, "Database error", "The bundled model index is damaged.", TvrViewBrands);
        return;
    }
    tvr_switch(app, TvrViewModels);
}

static void tvr_group_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    app->db_group = (char)index;
    File* file = storage_file_alloc(app->storage);
    bool ok = file && tvr_db_open(app, file);
    submenu_reset(app->brand_menu);
    char header[16];
    snprintf(header, sizeof(header), "Brands: %c", app->db_group);
    submenu_set_header(app->brand_menu, header);
    for(uint32_t i = 0U; ok && i < app->db_header.brand_count; ++i) {
        TvrDbBrand brand;
        if(!tvr_db_read_brand(app, file, i, &brand)) {
            ok = false;
            break;
        }
        if(tvr_db_group_for(brand.name) == app->db_group)
            submenu_add_item(app->brand_menu, brand.name, i, tvr_brand_selected, app);
    }
    if(file) {
        if(storage_file_is_open(file)) storage_file_close(file);
        storage_file_free(file);
    }
    if(!ok) {
        tvr_show(app, "Database error", "The bundled brand index is damaged.", TvrViewGroups);
        return;
    }
    tvr_switch(app, TvrViewBrands);
}

static void tvr_start_setup(TvrApp* app) {
    if(app->tv_count >= TVR_MAX_TVS) {
        tvr_show(
            app,
            "TV limit reached",
            "Remove a saved television before adding another. The limit is 8.",
            TvrViewMain);
        return;
    }
    File* file = storage_file_alloc(app->storage);
    bool groups[27] = {false};
    bool ok = file && tvr_db_open(app, file);
    for(uint32_t i = 0U; ok && i < app->db_header.brand_count; ++i) {
        TvrDbBrand brand;
        ok = tvr_db_read_brand(app, file, i, &brand);
        if(ok) {
            char group = tvr_db_group_for(brand.name);
            groups[group == '#' ? 0U : (uint32_t)(group - 'A' + 1)] = true;
        }
    }
    if(file) {
        if(storage_file_is_open(file)) storage_file_close(file);
        storage_file_free(file);
    }
    if(!ok) {
        tvr_show(
            app,
            "Database unavailable",
            "The bundled TV-code database is missing or damaged. Reinstall the application.",
            TvrViewMain);
        return;
    }
    submenu_reset(app->group_menu);
    submenu_set_header(app->group_menu, "Choose brand letter");
    if(groups[0]) submenu_add_item(app->group_menu, "# / Other", '#', tvr_group_selected, app);
    char label[2] = {'A', '\0'};
    for(uint32_t i = 1U; i < COUNT_OF(groups); ++i) {
        if(groups[i]) {
            label[0] = (char)('A' + i - 1U);
            submenu_add_item(app->group_menu, label, label[0], tvr_group_selected, app);
        }
    }
    tvr_switch(app, TvrViewGroups);
}

static void tvr_main_selected(void* context, uint32_t index) {
    TvrApp* app = context;
    if(index == MainFavorite)
        tvr_open_remote(app, app->favorite);
    else if(index == MainTvs) {
        tvr_rebuild_tvs(app);
        tvr_switch(app, TvrViewTvs);
    } else if(index == MainFind)
        tvr_start_setup(app);
    else if(index == MainAdd)
        tvr_add_remote(app, "/ext/infrared", "/ext");
    else if(index == MainSettings)
        tvr_switch(app, TvrViewSettings);
    else
        tvr_show(
            app,
            "About TV Remote FZ",
            "Version " TVR_VERSION
            "\n\nIncludes 3,051 genuine TV remotes from the official Flipper Devices IRDB. Set Up TV lets you choose a brand and model without downloading codes.\n\nFirst launch takes about 2 minutes to extract the database. Later launches are fast.\n\nAliases affect layout only. Original signal bytes are never modified. TX means sent, not confirmed by the TV.\n\nHold Left/Right to change pages. Hold OK for remote tools.\n\nApp: GPL-3.0-or-later. IRDB data: MIT. No warranty.",
            TvrViewMain);
}

static void tvr_direct_changed(VariableItem* item) {
    TvrApp* app = variable_item_get_context(item);
    uint8_t value = variable_item_get_current_value_index(item);
    app->direct_launch = value;
    variable_item_set_current_value_text(item, value ? "On" : "Off");
    tvr_save_config(app);
}

static void tvr_haptic_changed(VariableItem* item) {
    TvrApp* app = variable_item_get_context(item);
    uint8_t value = variable_item_get_current_value_index(item);
    app->haptic = value;
    variable_item_set_current_value_text(item, value ? "On" : "Off");
    tvr_save_config(app);
}

static bool tvr_back(void* context) {
    TvrApp* app = context;
    tvr_stop_repeat(app);
    if(app->current == TvrViewMain)
        view_dispatcher_stop(app->dispatcher);
    else if(app->current == TvrViewGroups)
        tvr_switch(app, TvrViewMain);
    else if(app->current == TvrViewBrands)
        tvr_switch(app, TvrViewGroups);
    else if(app->current == TvrViewModels)
        tvr_switch(app, TvrViewBrands);
    else if(app->current == TvrViewRemote) {
        tvr_rebuild_main(app);
        tvr_switch(app, TvrViewMain);
    } else if(app->current == TvrViewManage)
        tvr_switch(app, TvrViewRemote);
    else if(app->current == TvrViewCommands)
        tvr_switch(app, TvrViewManage);
    else if(app->current == TvrViewText)
        tvr_switch(app, app->text_back);
    else
        tvr_switch(app, TvrViewMain);
    return true;
}

static void tvr_tick(void* context) {
    TvrApp* app = context;
    if(app->sent_visible &&
       furi_get_tick() - app->sent_tick > furi_kernel_get_tick_frequency() / 2U) {
        app->sent_visible = false;
        view_commit_model(app->remote_view, true);
    }
}

static TvrApp* tvr_alloc(void) {
    TvrApp* app = calloc(1U, sizeof(*app));
    if(!app) return NULL;
    app->active_tv = -1;
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dispatcher = view_dispatcher_alloc();
    app->main_menu = submenu_alloc();
    app->group_menu = submenu_alloc();
    app->brand_menu = submenu_alloc();
    app->model_menu = submenu_alloc();
    app->tv_menu = submenu_alloc();
    app->manage_menu = submenu_alloc();
    app->command_menu = submenu_alloc();
    app->settings = variable_item_list_alloc();
    app->widget = widget_alloc();
    app->input = text_input_alloc();
    app->remote_view = view_alloc();
    app->signal.message.protocol = InfraredProtocolUnknown;
    app->ir_worker = infrared_worker_alloc();
    app->browser_path = furi_string_alloc_set("/ext/infrared");
    app->text = furi_string_alloc();
    if(!app->gui || !app->storage || !app->dialogs || !app->notifications || !app->dispatcher ||
       !app->main_menu || !app->group_menu || !app->brand_menu || !app->model_menu ||
       !app->tv_menu || !app->manage_menu || !app->command_menu || !app->settings ||
       !app->widget || !app->input || !app->remote_view || !app->ir_worker || !app->browser_path ||
       !app->text)
        return app;
    tvr_load_config(app);
    tvr_rebuild_main(app);
    VariableItem* direct =
        variable_item_list_add(app->settings, "Favorite start", 2U, tvr_direct_changed, app);
    variable_item_set_current_value_index(direct, app->direct_launch);
    variable_item_set_current_value_text(direct, app->direct_launch ? "On" : "Off");
    VariableItem* haptic =
        variable_item_list_add(app->settings, "Vibrate", 2U, tvr_haptic_changed, app);
    variable_item_set_current_value_index(haptic, app->haptic);
    variable_item_set_current_value_text(haptic, app->haptic ? "On" : "Off");
    VariableItem* version = variable_item_list_add(app->settings, "Version", 1U, NULL, app);
    variable_item_set_current_value_text(version, TVR_VERSION);
    view_set_context(app->remote_view, app);
    view_set_draw_callback(app->remote_view, tvr_remote_draw);
    view_set_input_callback(app->remote_view, tvr_remote_input);
    view_allocate_model(app->remote_view, ViewModelTypeLocking, sizeof(TvrRemoteModel));
    TvrRemoteModel* model = view_get_model(app->remote_view);
    model->app = app;
    view_commit_model(app->remote_view, false);
    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, tvr_back);
    view_dispatcher_set_tick_event_callback(app->dispatcher, tvr_tick, 100U);
    view_dispatcher_add_view(app->dispatcher, TvrViewMain, submenu_get_view(app->main_menu));
    view_dispatcher_add_view(app->dispatcher, TvrViewGroups, submenu_get_view(app->group_menu));
    view_dispatcher_add_view(app->dispatcher, TvrViewBrands, submenu_get_view(app->brand_menu));
    view_dispatcher_add_view(app->dispatcher, TvrViewModels, submenu_get_view(app->model_menu));
    view_dispatcher_add_view(app->dispatcher, TvrViewTvs, submenu_get_view(app->tv_menu));
    view_dispatcher_add_view(app->dispatcher, TvrViewRemote, app->remote_view);
    view_dispatcher_add_view(app->dispatcher, TvrViewManage, submenu_get_view(app->manage_menu));
    view_dispatcher_add_view(
        app->dispatcher, TvrViewCommands, submenu_get_view(app->command_menu));
    view_dispatcher_add_view(
        app->dispatcher, TvrViewSettings, variable_item_list_get_view(app->settings));
    view_dispatcher_add_view(app->dispatcher, TvrViewText, widget_get_view(app->widget));
    view_dispatcher_add_view(app->dispatcher, TvrViewInput, text_input_get_view(app->input));
    app->views_added = true;
    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    return app;
}

static bool tvr_valid(TvrApp* app) {
    return app && app->gui && app->storage && app->dialogs && app->notifications &&
           app->dispatcher && app->main_menu && app->tv_menu && app->manage_menu &&
           app->group_menu && app->brand_menu && app->model_menu && app->command_menu &&
           app->settings && app->widget && app->input && app->remote_view && app->ir_worker &&
           app->browser_path && app->text;
}

static void tvr_free(TvrApp* app) {
    if(!app) return;
    tvr_stop_repeat(app);
    if(app->dispatcher && app->views_added)
        for(int i = TvrViewInput; i >= TvrViewMain; --i)
            view_dispatcher_remove_view(app->dispatcher, (uint32_t)i);
    if(app->ir_worker) infrared_worker_free(app->ir_worker);
    tvr_ir_clear(&app->signal);
    if(app->text) furi_string_free(app->text);
    if(app->browser_path) furi_string_free(app->browser_path);
    if(app->remote_view) view_free(app->remote_view);
    if(app->input) text_input_free(app->input);
    if(app->widget) widget_free(app->widget);
    if(app->settings) variable_item_list_free(app->settings);
    if(app->command_menu) submenu_free(app->command_menu);
    if(app->manage_menu) submenu_free(app->manage_menu);
    if(app->tv_menu) submenu_free(app->tv_menu);
    if(app->model_menu) submenu_free(app->model_menu);
    if(app->brand_menu) submenu_free(app->brand_menu);
    if(app->group_menu) submenu_free(app->group_menu);
    if(app->main_menu) submenu_free(app->main_menu);
    if(app->dispatcher) view_dispatcher_free(app->dispatcher);
    if(app->notifications) furi_record_close(RECORD_NOTIFICATION);
    if(app->dialogs) furi_record_close(RECORD_DIALOGS);
    if(app->storage) furi_record_close(RECORD_STORAGE);
    if(app->gui) furi_record_close(RECORD_GUI);
    free(app);
}

int32_t tv_remote_fz_app(void* context) {
    UNUSED(context);
    TvrApp* app = tvr_alloc();
    if(!tvr_valid(app)) {
        tvr_free(app);
        return -1;
    }
    if(app->direct_launch && app->favorite >= 0)
        tvr_open_remote(app, app->favorite);
    else
        tvr_switch(app, TvrViewMain);
    view_dispatcher_run(app->dispatcher);
    tvr_free(app);
    return 0;
}
