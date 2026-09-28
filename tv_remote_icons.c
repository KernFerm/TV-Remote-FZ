/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "tv_remote_icons.h"

bool tvr_icon_available(TvrAction action) {
    return action == TvrActionPower || action == TvrActionMute || action == TvrActionInput ||
           action == TvrActionVolumeUp || action == TvrActionVolumeDown ||
           action == TvrActionChannelUp || action == TvrActionChannelDown ||
           action == TvrActionGuide || action == TvrActionInfo || action == TvrActionHome ||
           action == TvrActionMenu || action == TvrActionBack || action == TvrActionPlay ||
           action == TvrActionPause || action == TvrActionStop || action == TvrActionRewind ||
           action == TvrActionFastForward || action == TvrActionPrevious ||
           action == TvrActionNext || action == TvrActionRecord || action == TvrActionSettings;
}

static void tvr_icon_triangle(Canvas* canvas, int x, int y, CanvasDirection direction) {
    canvas_draw_triangle(canvas, x - 3, y - 3, 6, 6, direction);
}

void tvr_icon_draw(Canvas* canvas, TvrAction action, int x, int y) {
    if(action == TvrActionPower) {
        canvas_draw_circle(canvas, x, y + 1, 3);
        canvas_invert_color(canvas);
        canvas_draw_box(canvas, x - 1, y - 3, 3, 4);
        canvas_invert_color(canvas);
        canvas_draw_line(canvas, x, y - 4, x, y);
    } else if(action == TvrActionMute) {
        canvas_draw_box(canvas, x - 4, y - 2, 2, 5);
        canvas_draw_line(canvas, x - 2, y - 2, x + 1, y - 4);
        canvas_draw_line(canvas, x - 2, y + 2, x + 1, y + 4);
        canvas_draw_line(canvas, x + 3, y - 3, x + 6, y + 3);
        canvas_draw_line(canvas, x + 6, y - 3, x + 3, y + 3);
    } else if(action == TvrActionInput) {
        canvas_draw_frame(canvas, x - 5, y - 4, 8, 8);
        canvas_draw_line(canvas, x, y, x + 6, y);
        canvas_draw_line(canvas, x + 4, y - 2, x + 6, y);
        canvas_draw_line(canvas, x + 4, y + 2, x + 6, y);
    } else if(action == TvrActionVolumeUp || action == TvrActionVolumeDown) {
        canvas_draw_box(canvas, x - 5, y - 2, 2, 5);
        canvas_draw_line(canvas, x - 3, y - 2, x, y - 4);
        canvas_draw_line(canvas, x - 3, y + 2, x, y + 4);
        canvas_draw_line(canvas, x + 2, y, x + 6, y);
        if(action == TvrActionVolumeUp) canvas_draw_line(canvas, x + 4, y - 2, x + 4, y + 2);
    } else if(action == TvrActionChannelUp || action == TvrActionChannelDown) {
        canvas_draw_frame(canvas, x - 5, y - 4, 7, 8);
        canvas_draw_line(canvas, x + 3, y, x + 6, y);
        if(action == TvrActionChannelUp) canvas_draw_line(canvas, x + 4, y - 2, x + 4, y + 2);
    } else if(action == TvrActionHome) {
        canvas_draw_line(canvas, x - 5, y, x, y - 4);
        canvas_draw_line(canvas, x, y - 4, x + 5, y);
        canvas_draw_frame(canvas, x - 4, y, 9, 5);
    } else if(action == TvrActionMenu) {
        canvas_draw_line(canvas, x - 5, y - 3, x + 5, y - 3);
        canvas_draw_line(canvas, x - 5, y, x + 5, y);
        canvas_draw_line(canvas, x - 5, y + 3, x + 5, y + 3);
    } else if(action == TvrActionBack) {
        canvas_draw_line(canvas, x - 5, y, x + 5, y);
        canvas_draw_line(canvas, x - 5, y, x - 1, y - 4);
        canvas_draw_line(canvas, x - 5, y, x - 1, y + 4);
    } else if(action == TvrActionGuide) {
        canvas_draw_frame(canvas, x - 5, y - 4, 11, 9);
        canvas_draw_line(canvas, x, y - 4, x, y + 4);
        canvas_draw_line(canvas, x - 5, y, x + 5, y);
    } else if(action == TvrActionInfo) {
        canvas_draw_circle(canvas, x, y, 4);
        canvas_draw_dot(canvas, x, y - 2);
        canvas_draw_line(canvas, x, y, x, y + 2);
    } else if(action == TvrActionSettings) {
        canvas_draw_circle(canvas, x, y, 3);
        canvas_draw_dot(canvas, x, y);
        canvas_draw_line(canvas, x, y - 5, x, y - 3);
        canvas_draw_line(canvas, x, y + 3, x, y + 5);
        canvas_draw_line(canvas, x - 5, y, x - 3, y);
        canvas_draw_line(canvas, x + 3, y, x + 5, y);
    } else if(action == TvrActionPlay) {
        tvr_icon_triangle(canvas, x, y, CanvasDirectionLeftToRight);
    } else if(action == TvrActionPause) {
        canvas_draw_box(canvas, x - 4, y - 4, 3, 9);
        canvas_draw_box(canvas, x + 2, y - 4, 3, 9);
    } else if(action == TvrActionStop) {
        canvas_draw_box(canvas, x - 4, y - 4, 9, 9);
    } else if(action == TvrActionRecord) {
        canvas_draw_disc(canvas, x, y, 4);
    } else if(action == TvrActionRewind || action == TvrActionPrevious) {
        tvr_icon_triangle(canvas, x + 2, y, CanvasDirectionRightToLeft);
        if(action == TvrActionPrevious) canvas_draw_line(canvas, x - 5, y - 4, x - 5, y + 4);
    } else if(action == TvrActionFastForward || action == TvrActionNext) {
        tvr_icon_triangle(canvas, x - 2, y, CanvasDirectionLeftToRight);
        if(action == TvrActionNext) canvas_draw_line(canvas, x + 5, y - 4, x + 5, y + 4);
    }
}
