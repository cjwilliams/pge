#include "pge_title.h"

// UI
static Window *s_window;
static TextLayer *s_title_layer, *s_light_layer;
static BitmapLayer *s_bg_layer;
static GBitmap *s_bg_bitmap;

static PGEClickHandler *s_click_handler;

// State
static int s_background_res_id;
static GColor s_title_color;
static char s_title_buffer[PGE_TITLE_LENGTH_MAX];
static bool s_light_on;

/*********************************** Clicks ***********************************/

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Toggle light
  s_light_on = !s_light_on;
  light_enable(s_light_on);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_click_handler(BUTTON_ID_SELECT);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_click_handler(BUTTON_ID_DOWN);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/********************************* Window *************************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Allocate background
  if(s_bg_bitmap) {
    gbitmap_destroy(s_bg_bitmap);
  }
  s_bg_bitmap = gbitmap_create_with_resource(s_background_res_id);

  // BG Layer
  s_bg_layer = bitmap_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bg_layer));

  // Title
  s_title_layer = text_layer_create(GRect(10, 40, window_bounds.size.w - 20, 60));
  text_layer_set_text_color(s_title_layer, s_title_color);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_title_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_title_layer, s_title_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  // Light TextLayer
  s_light_layer = text_layer_create(GRect(0, 20, window_bounds.size.w, 30));
  text_layer_set_text_color(s_light_layer, s_title_color);
  text_layer_set_background_color(s_light_layer, GColorClear);
  text_layer_set_text_alignment(s_light_layer, GTextAlignmentRight);
  text_layer_set_font(s_light_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_light_layer, "LIGHT >");
  layer_add_child(window_layer, text_layer_get_layer(s_light_layer));
}

static void window_unload(Window *window) {
  gbitmap_destroy(s_bg_bitmap);
  bitmap_layer_destroy(s_bg_layer);
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_light_layer);

  // Finally
  window_destroy(window);
  window = NULL;
}

/********************************* Public *************************************/

void pge_title_push(char *title, GColor title_color, int background_res_id, PGEClickHandler *click_handler) {
  // Store values
  s_background_res_id = background_res_id;
  s_title_color = title_color;
  snprintf(s_title_buffer, sizeof(s_title_buffer), "%s", title);

  s_click_handler = click_handler;

  // Create Window
  if(!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_fullscreen(s_window, true);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void pge_title_pop() {
  // Should self-destroy
  window_stack_pop(true);
}