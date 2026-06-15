#include <furi.h>
#include <gui/canvas.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAYAN_DECODER_MIN_VALUE 0
#define MAYAN_DECODER_MAX_VALUE 999
#define MAYAN_DECODER_QUEUE_SIZE 8

typedef struct {
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;
    int32_t value;
} MayanDecoderApp;

static void mayan_decoder_draw_shell(Canvas* canvas, int32_t center_x, int32_t top_y) {
    canvas_draw_circle(canvas, center_x, top_y + 4, 4);
    canvas_draw_line(canvas, center_x - 4, top_y + 4, center_x + 4, top_y + 4);
    canvas_draw_line(canvas, center_x - 2, top_y + 6, center_x + 2, top_y + 6);
    canvas_draw_dot(canvas, center_x - 2, top_y + 3);
    canvas_draw_dot(canvas, center_x, top_y + 2);
    canvas_draw_dot(canvas, center_x + 2, top_y + 3);
}

static void mayan_decoder_draw_digit(Canvas* canvas, int32_t digit, int32_t center_x, int32_t top_y) {
    furi_assert(digit >= 0);
    furi_assert(digit < 20);

    if(digit == 0) {
        mayan_decoder_draw_shell(canvas, center_x, top_y);
        return;
    }

    const int32_t bars = digit / 5;
    const int32_t dots = digit % 5;
    const int32_t bar_width = 24;
    const int32_t bar_height = 1;
    const int32_t bar_gap = 1;
    const int32_t bottom_y = top_y + 9;

    for(int32_t i = 0; i < bars; i++) {
        const int32_t y = bottom_y - ((bars - i) * (bar_height + bar_gap));
        canvas_draw_box(canvas, center_x - (bar_width / 2), y, bar_width, bar_height);
    }

    if(dots > 0) {
        int32_t dot_y = bottom_y - (bars * (bar_height + bar_gap)) - 2;
        if(bars == 0) {
            dot_y = top_y + 5;
        }

        const int32_t dot_spacing = 5;
        const int32_t start_x = center_x - ((dots - 1) * dot_spacing) / 2;
        for(int32_t i = 0; i < dots; i++) {
            canvas_draw_disc(canvas, start_x + (i * dot_spacing), dot_y, 1);
        }
    }
}

static uint8_t mayan_decoder_to_base20(int32_t value, int32_t* digits, size_t max_digits) {
    furi_assert(digits);
    furi_assert(max_digits > 0);

    if(value == 0) {
        digits[0] = 0;
        return 1;
    }

    uint8_t count = 0;
    while(value > 0 && count < max_digits) {
        digits[count++] = value % 20;
        value /= 20;
    }

    return count;
}

static void mayan_decoder_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    MayanDecoderApp* app = context;

    char value_text[24];
    snprintf(value_text, sizeof(value_text), "Decimal: %ld", (long)app->value);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignBottom, "Mayan Decoder");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 23, value_text);
    canvas_draw_str(canvas, 83, 23, "OK/B Exit");
    canvas_draw_line(canvas, 0, 27, 127, 27);

    int32_t digits[4] = {0};
    const uint8_t digit_count = mayan_decoder_to_base20(app->value, digits, COUNT_OF(digits));
    const int32_t tier_height = 9;
    const int32_t tier_gap = 2;
    const int32_t total_height = (digit_count * tier_height) + ((digit_count - 1) * tier_gap);
    int32_t y = 30 + ((32 - total_height) / 2);

    canvas_set_font(canvas, FontSecondary);
    for(int32_t i = digit_count - 1; i >= 0; i--) {
        mayan_decoder_draw_digit(canvas, digits[i], 64, y);

        if(i > 0) {
            canvas_draw_str_aligned(canvas, 19, y + 9, AlignCenter, AlignBottom, "x20");
            canvas_draw_line(canvas, 43, y + tier_height + 1, 85, y + tier_height + 1);
        }

        y += tier_height + tier_gap;
    }
}

static void mayan_decoder_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    MayanDecoderApp* app = context;
    furi_message_queue_put(app->input_queue, event, 0);
}

static void mayan_decoder_change_value(MayanDecoderApp* app, int32_t delta) {
    furi_assert(app);

    int32_t next_value = app->value + delta;
    if(next_value < MAYAN_DECODER_MIN_VALUE) {
        next_value = MAYAN_DECODER_MIN_VALUE;
    } else if(next_value > MAYAN_DECODER_MAX_VALUE) {
        next_value = MAYAN_DECODER_MAX_VALUE;
    }

    if(next_value != app->value) {
        app->value = next_value;
        view_port_update(app->view_port);
    }
}

int32_t mayan_decoder_app(void* p) {
    UNUSED(p);

    MayanDecoderApp* app = malloc(sizeof(MayanDecoderApp));
    furi_assert(app);
    app->input_queue = furi_message_queue_alloc(MAYAN_DECODER_QUEUE_SIZE, sizeof(InputEvent));
    app->view_port = view_port_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->value = 0;

    view_port_draw_callback_set(app->view_port, mayan_decoder_draw_callback, app);
    view_port_input_callback_set(app->view_port, mayan_decoder_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    view_port_update(app->view_port);

    bool running = true;
    InputEvent event;
    while(running) {
        const FuriStatus status = furi_message_queue_get(app->input_queue, &event, FuriWaitForever);
        if(status != FuriStatusOk) {
            continue;
        }

        if(event.type != InputTypeShort && event.type != InputTypeRepeat) {
            continue;
        }

        switch(event.key) {
        case InputKeyUp:
        case InputKeyRight:
            mayan_decoder_change_value(app, 1);
            break;
        case InputKeyDown:
        case InputKeyLeft:
            mayan_decoder_change_value(app, -1);
            break;
        case InputKeyOk:
        case InputKeyBack:
            running = false;
            break;
        default:
            break;
        }
    }

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
