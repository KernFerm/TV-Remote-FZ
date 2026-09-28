/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include "tv_remote_core.h"
#include <gui/canvas.h>

bool tvr_icon_available(TvrAction action);
void tvr_icon_draw(Canvas* canvas, TvrAction action, int x, int y);
