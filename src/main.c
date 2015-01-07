#include <pebble.h>
#include <time.h>  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_LOCATION 2
#define HOUR_BELLS_FROM 3
#define HOUR_BELLS_TO 4
#define HALF_HOUR_BELLS 5
#define QUARTER_HOUR_BELLS 6
#define BLACK_ON_WHITE 8

// Configurable options
bool half_hour_bells = true;
bool quarter_hour_bells = false;
static int hour_bells_from = 8;
static int hour_bells_to = 22;
bool black_on_white = false;
// APP_LOG(APP_LOG_LEVEL_INFO, "Bells From: %d", (int)hour_bells_from);

// windows and layers  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_half_layer;
static TextLayer *s_quarter_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_date_layer;
static TextLayer *s_location_layer;

// background
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

// Calendar stuff
static const char wday_name[][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
static const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

// define presets for constants
const unsigned int VIBE_INTERVAL_IN_MINUTES = 30;
const unsigned int VIBE_SUBINTERVAL_IN_MINUTES = 15;

const VibePattern ONE_BELL = {
  .durations = (uint32_t []) {300,100},
    .num_segments = 2
};
const VibePattern TWO_BELLS = {
  .durations = (uint32_t []) {300,100,300,100},
    .num_segments = 4
};
const VibePattern THREE_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100},
    .num_segments = 6
};
const VibePattern FOUR_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100,300,100},
    .num_segments = 8
};
const VibePattern FIVE_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100,300,500,300,100},
    .num_segments = 10
};
const VibePattern SIX_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100,300,500,300,100,300,100},
    .num_segments = 12
};
const VibePattern SEVEN_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100,300,500,300,100,300,500,300,100},
    .num_segments = 14
};
const VibePattern EIGHT_BELLS = {
  .durations = (uint32_t []) {300,100,300,500,300,100,300,500,300,100,300,500,300,100,300,100},
    .num_segments = 16
};
const VibePattern QUARTER_HOUR_VIBE_PATTERN = {
  .durations = (uint32_t []) {200,100,200,100,200,100},
    .num_segments = 6
};

static void vibrate_bells() {
  // Get a tm structure
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  int hournow = tick_time->tm_hour;
  // Deal with Whole Hours
  if(tick_time->tm_min == 0) {
    if(hournow == 0 || hournow == 4 || hournow == 8 || hournow == 12 || hournow == 16 || hournow == 20) {
      vibes_enqueue_custom_pattern(EIGHT_BELLS);
    }
    if(hournow == 1 || hournow == 5 || hournow == 9 || hournow == 13 || hournow == 17 || hournow == 21) {
      vibes_enqueue_custom_pattern(TWO_BELLS);
    }
    if(hournow == 2 || hournow == 6 || hournow == 10 || hournow == 14 || hournow == 18 || hournow == 22) {
      vibes_enqueue_custom_pattern(FOUR_BELLS);
    }
    if(hournow == 4 || hournow == 7 || hournow == 11 || hournow == 15 || hournow == 19 || hournow == 23) {
      vibes_enqueue_custom_pattern(SIX_BELLS);
    }
    // now deal with half hours
  } else if((tick_time->tm_min % VIBE_INTERVAL_IN_MINUTES) == 0 && half_hour_bells) {
    if(hournow == 0 || hournow == 4 || hournow == 8 || hournow == 12 || hournow == 16 || hournow == 20) {
      vibes_enqueue_custom_pattern(ONE_BELL);
    }
    if(hournow == 1 || hournow == 5 || hournow == 9 || hournow == 13 || hournow == 17 || hournow == 21) {
      vibes_enqueue_custom_pattern(THREE_BELLS);
    }
    if(hournow == 2 || hournow == 6 || hournow == 10 || hournow == 14 || hournow == 18 || hournow == 22) {
      vibes_enqueue_custom_pattern(FIVE_BELLS);
    }
    if(hournow == 4 || hournow == 7 || hournow == 11 || hournow == 15 || hournow == 19 || hournow == 23) {
      vibes_enqueue_custom_pattern(SEVEN_BELLS);
    }
    // now deal with quarter hours
  } else if ((tick_time->tm_min % VIBE_SUBINTERVAL_IN_MINUTES) == 0 && quarter_hour_bells) {
    vibes_enqueue_custom_pattern(QUARTER_HOUR_VIBE_PATTERN);
  }
}

static void update_time() {
  // Get a tm structure
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  // Create a buffer
  static char buffer[] = "00:00";
  //Write current hours and minutes
  if(clock_is_24h_style() == true) {
    //use 24hr format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // User 12hr format
    strftime(buffer, sizeof("00:00"), "I:%M", tick_time);
  }
  // Are we within the From and To hours?
  if (tick_time->tm_hour >= hour_bells_from && tick_time->tm_hour < hour_bells_to) {
    vibrate_bells();
  }
  // Display this time
  text_layer_set_text(s_time_layer, buffer);
  // Setup the date
  static char date_buffer[18];
  snprintf(date_buffer, sizeof(date_buffer), "%s %d %s %d", wday_name[tick_time->tm_wday], tick_time->tm_mday, mon_name[tick_time->tm_mon], 1900 + tick_time->tm_year);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 15 minutes
  if(tick_time->tm_min % 15 == 0){
    // Begin dictionary
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating weather at %d:%d", tick_time->tm_hour, tick_time->tm_min );
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
    // Send the message!
    app_message_outbox_send();
  }
  static char s_battery_buffer[16];
  BatteryChargeState charge_state = battery_state_service_peek();
  if (charge_state.is_charging) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Bat: chrg");
  } else {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Bat: %d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void main_window_load(Window *window) {
  //Check for saved options
  bool black_on_white = persist_read_bool(BLACK_ON_WHITE);
  bool half_hour_bells = persist_read_bool(HALF_HOUR_BELLS);
  bool quarter_hour_bells = persist_read_bool(QUARTER_HOUR_BELLS);
  hour_bells_from = persist_read_int(HOUR_BELLS_FROM);
  hour_bells_to = persist_read_int(HOUR_BELLS_TO);
  // Create GBitmap, then set to created BitmapLayer
  if(black_on_white) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHIPS_BELLS_BG);
  } else {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHIPS_BELLS_BGB);
  }
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  // Create Display TextLayers
  s_time_layer = text_layer_create(GRect(0,12,144,50));
  s_weather_layer = text_layer_create(GRect(0,60,144,30));
  s_half_layer = text_layer_create(GRect(0,145,71,25));
  s_quarter_layer = text_layer_create(GRect(72,145,72,25));
  s_battery_layer = text_layer_create(GRect(72,0,72,18));
  s_location_layer = text_layer_create(GRect(0,87,144,23));
  s_date_layer = text_layer_create(GRect(0,112,144,30));
  // Set text layer backgrounds
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_background_color(s_half_layer, GColorClear);
  text_layer_set_background_color(s_quarter_layer, GColorClear);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_background_color(s_date_layer, GColorClear);
  // Set text foreground color
  if(black_on_white) {
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_weather_layer, GColorBlack);
    text_layer_set_text_color(s_half_layer, GColorBlack);
    text_layer_set_text_color(s_quarter_layer, GColorBlack);
    text_layer_set_text_color(s_battery_layer, GColorBlack);
    text_layer_set_text_color(s_location_layer, GColorBlack);
    text_layer_set_text_color(s_date_layer, GColorBlack);
  } else {
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_color(s_half_layer, GColorWhite);
    text_layer_set_text_color(s_quarter_layer, GColorWhite);
    text_layer_set_text_color(s_battery_layer, GColorWhite);
    text_layer_set_text_color(s_location_layer, GColorWhite);
    text_layer_set_text_color(s_date_layer, GColorWhite);
  }
  // Set text alignments
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_half_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_quarter_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // Set text layer fonts
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_font(s_half_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_font(s_quarter_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  // text_layer_set_text(s_time_layer,"12:34");
  // Add all texts as child layers
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  text_layer_set_text(s_weather_layer, "Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  static char half_buffer[10];
  static char qtr_buffer[10];
  APP_LOG(APP_LOG_LEVEL_INFO, "Bells From: %d", (int)hour_bells_from);
  APP_LOG(APP_LOG_LEVEL_INFO, "Bells To: %d", (int)hour_bells_to);
  if(half_hour_bells) {
    snprintf(half_buffer, sizeof(half_buffer), "H:On %d", (int)hour_bells_from);
  } else {
    snprintf(half_buffer, sizeof(half_buffer), "H:Off %d", (int)hour_bells_from);
  }
  if(quarter_hour_bells) {
    snprintf(qtr_buffer, sizeof(qtr_buffer), "Q:On %d", (int)hour_bells_to);
  } else {
    snprintf(qtr_buffer, sizeof(qtr_buffer), "Q:Off %d", (int)hour_bells_to);
  }
  text_layer_set_text(s_half_layer, half_buffer);
  text_layer_set_text(s_quarter_layer, qtr_buffer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_half_layer));  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_quarter_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_location_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_half_layer);
  text_layer_destroy(s_quarter_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_location_layer);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char location_buffer[16];
  static char weather_layer_buffer[32]; 
  // Read first item
  Tuple *t = dict_read_first(iterator);
  // For all items
  while(t != NULL) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Value: %s", t->value->cstring);
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_LOCATION:
      snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
      break;
    case BLACK_ON_WHITE:
      if(strcmp(t->value->cstring, "on") == 0) {
        //Set and save as inverted
        black_on_white = true;
        persist_write_bool(BLACK_ON_WHITE, true);
      } else {
        //Set and save as not inverted
        black_on_white = false;
        persist_write_bool(BLACK_ON_WHITE, false);
      }
      break;
    case HALF_HOUR_BELLS:
      if (strcmp(t->value->cstring, "on") == 0) {
        half_hour_bells = true;
        persist_write_bool(HALF_HOUR_BELLS, true);
      } else {
        half_hour_bells = false;
        persist_write_bool(HALF_HOUR_BELLS, false);
      }
      break;
    case QUARTER_HOUR_BELLS:
      if (strcmp(t->value->cstring, "on") == 0) {
        quarter_hour_bells = true;
        persist_write_bool(QUARTER_HOUR_BELLS, true);
      } else {
        quarter_hour_bells = false;
        persist_write_bool(QUARTER_HOUR_BELLS, false);
      }
      break;
    case HOUR_BELLS_FROM:
      APP_LOG(APP_LOG_LEVEL_INFO, "Bells From: %i", (int)t->value->int8);
      persist_write_int(HOUR_BELLS_FROM, (int)t->value->int8);
      break;
    case HOUR_BELLS_TO:
      persist_write_int(HOUR_BELLS_TO, (int)t->value->int8);
      break;  
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  }
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
  text_layer_set_text(s_location_layer, location_buffer);
  APP_LOG(APP_LOG_LEVEL_INFO, "Weather: %s,%s %s", temperature_buffer, conditions_buffer, location_buffer );
} 

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Create main Window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the window on the watch
  window_stack_push(s_main_window, true);
  // Make sure time is displayed from the start
  update_time();
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}