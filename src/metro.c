#include <pebble.h>

static Window *window;

static Layer *deco_layer;

static GBitmap *arrow_image;
static BitmapLayer *image_layer;

static GFont *text_font;
static TextLayer *text_layer;

static GFont *time_font;
static TextLayer *time_layer;

#ifdef PBL_COLOR
    #define METRO_APP_TEXT_COLOR GColorWhite
    #define METRO_APP_BACK_COLOR GColorBlack
#else
    #define METRO_APP_TEXT_COLOR GColorBlack
    #define METRO_APP_BACK_COLOR GColorWhite
#endif


static inline struct tm *get_time() {
    time_t t = time(NULL);
    return localtime(&t);
}

static void deco_layer_onupdate(Layer *this_layer, GContext *ctx) {
    GColor color = METRO_APP_BACK_COLOR;

#ifdef PBL_COLOR
    // Determine the color of deco layer based on time of day
    struct tm *tick_time = get_time();
    if (tick_time->tm_hour >= 22 && tick_time->tm_hour < 2)
        color = GColorPurpureus;    // 0x7b5aa3 -> 0xaa55aa, Airport Express
    else if (tick_time->tm_hour < 6)
        color = GColorRed;          // 0xe3002d -> 0xff0000, Line 2
    else if (tick_time->tm_hour < 10)
        color = GColorRajah;        // 0xc48d33 -> 0xffaa55, Line 1
    else if (tick_time->tm_hour < 14)
        color = GColorChromeYellow; // 0xf8b51c -> 0xffaa00, Line 4
    else if (tick_time->tm_hour < 18)
        color = GColorJaegerGreen;  // 0x01865b -> 0x00aa55, Line 3
    else
        color = GColorCobaltBlue;   // 0x0070bc -> 0x0055aa, Line 5
#endif
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, layer_get_bounds(this_layer), 0, GCornerNone);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    // GRect bounds = layer_get_bounds(window_layer);

    deco_layer = layer_create(GRect(0, 0, 144, 8));
    layer_set_update_proc(deco_layer, deco_layer_onupdate);

    arrow_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARROW_24);
    image_layer = bitmap_layer_create(GRect(12, 20, 24, 24));
    bitmap_layer_set_bitmap(image_layer, arrow_image);

    text_layer = text_layer_create(GRect(16, 113, 128, 24));
    text_layer_set_background_color(text_layer, METRO_APP_BACK_COLOR);
    text_layer_set_text_color(text_layer, METRO_APP_TEXT_COLOR);

    text_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_LIGHT_18));
    text_layer_set_font(text_layer, text_font);

    time_layer = text_layer_create(GRect(16, 58, 128, 48));
    text_layer_set_background_color(time_layer, METRO_APP_BACK_COLOR);
    text_layer_set_text_color(time_layer, METRO_APP_TEXT_COLOR);

    time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_REGULAR_48));
    text_layer_set_font(time_layer, time_font);

    layer_add_child(window_layer, deco_layer);
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
    layer_add_child(window_layer, text_layer_get_layer(time_layer));
}

static void window_unload(Window *window) {
    fonts_unload_custom_font(time_font);
    text_layer_destroy(time_layer);
    fonts_unload_custom_font(text_font);
    text_layer_destroy(text_layer);
    gbitmap_destroy(arrow_image);
    bitmap_layer_destroy(image_layer);
}

static void update_time(struct tm *tick_time) {
    static char time_buffer[8];
    static char text_buffer[32];

    int hours = tick_time->tm_hour;
    if (!clock_is_24h_style() && hours > 12)
        hours -= 12;

    snprintf(time_buffer, sizeof(time_buffer), "%d:%02d", hours, tick_time->tm_min);
    text_layer_set_text(time_layer, time_buffer);

    static char* months[12] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    snprintf(text_buffer, sizeof(text_buffer), "%s %d", months[tick_time->tm_mon], tick_time->tm_mday);
    text_layer_set_text(text_layer, text_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
}

static void init(void) {
    window = window_create();
    window_set_background_color(window, METRO_APP_BACK_COLOR);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);

    // Register with system services
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // Get current time beforehand
    update_time(get_time());
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
