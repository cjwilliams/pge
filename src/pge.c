#include "pge.h"

// Internal prototypes
static void delta_handler(void *context);
static void canvas_update_proc(Layer *layer, GContext *ctx);
static void stop_rendering(PGE *this);
static void start_rendering(PGE *this);
static void destroy(PGE *this);
static void click_config_provider(void *context);

PGE* pge_begin(Window *parent, PGELogicHandler *logic_handler, PGERenderHandler *render_handler, PGEClickHandler *click_handler) {
  PGE *this = malloc(sizeof(PGE));

  // Allocate
  this->parent = parent;
  this->canvas = layer_create_with_data(GRect(0, 0, 144, 168), sizeof(uint8_t));

  // Set up
  this->is_running = true; // Required for first draw
  layer_set_update_proc(this->canvas, canvas_update_proc);
  layer_add_child(window_get_root_layer(this->parent), this->canvas);
  this->logic_handler = logic_handler;
  this->render_handler = render_handler;

  if(click_handler != NULL) {
    this->click_handler = click_handler;
    window_set_click_config_provider_with_context(this->parent, click_config_provider, this);
  }

  // Init button states (may be redundant)
  for(int i = 0; i < 3; i++) {
    this->button_states[i] = false;
  }

  // Hack to get ref of this into LayerUpdateProc
  void *ptr = layer_get_data(this->canvas);
  *((PGE*)ptr) = *(this);

  // Go!
  start_rendering(this);
  return this;
}

void pge_finish(PGE *this) {
  // Stop the game
  stop_rendering(this);
}

bool pge_get_button_state(PGE *this, ButtonId button) {
  switch(button) {
    case BUTTON_ID_UP:
      return this->button_states[0];
    case BUTTON_ID_SELECT:
      return this->button_states[1];
    case BUTTON_ID_DOWN:
      return this->button_states[2];
    default: 
      return false;
  }
}

/**************************** Internal Functions ******************************/

static void delta_handler(void *context) {
  PGE *this = (PGE*)context;

  if(this->logic_handler != NULL && this->render_handler != NULL) {
    // Do this frame
    layer_mark_dirty(this->canvas);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Loop or Render handler not set!");
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Woo hack!
  PGE *this = (PGE*)layer_get_data(layer);

  // Render and logic
  if(this->logic_handler != NULL && this->render_handler != NULL) {
    this->render_handler(ctx);
    this->logic_handler();

    // Next frame, only if stop has not been registered
    if(this->is_running == true) {
      this->render_timer = app_timer_register(PGE_RENDER_DELTA, delta_handler, this);
    } else {
      destroy(this);
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Loop or Render handler not set!");
  }
}

static void stop_rendering(PGE *this) {
  if(this->render_timer != NULL) {
    // Cancel any Timer
    app_timer_cancel(this->render_timer);
    this->render_timer = NULL;
  }

  // Will stop at the end of the next frame update_proc
  this->is_running = false;
}

static void start_rendering(PGE *this) {
  // Stop any current Timer
  stop_rendering(this);

  this->is_running = true;

  // Register new Timer to begin frame rendering loop
  this->render_timer = app_timer_register(PGE_RENDER_DELTA, delta_handler, this);
}

static void destroy(PGE *this) {
  // Destroy canvas
  layer_destroy(this->canvas);

  free(this);
}

static void up_pressed_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[0] = true;
}

static void up_released_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[0] = false;
}

static void select_pressed_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[1] = true;
}

static void select_released_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[1] = false;
}

static void down_pressed_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[2] = true;
}

static void down_released_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->button_states[2] = false;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->click_handler(BUTTON_ID_SELECT);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->click_handler(BUTTON_ID_UP);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  PGE *this = (PGE*)context;
  this->click_handler(BUTTON_ID_DOWN);
}

static void click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_UP, up_pressed_click_handler, up_released_click_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_pressed_click_handler, select_released_click_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, down_pressed_click_handler, down_released_click_handler, NULL);

  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
