#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_ICON 2
#define KEY_DAY_ONE_TEMPERATURE 3
#define KEY_DAY_ONE_ICON 4
#define KEY_DAY_TWO_TEMPERATURE 5
#define KEY_DAY_TWO_ICON 6
#define KEY_DAY_ONE_DAY 7
#define KEY_DAY_TWO_DAY 8

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static GFont *s_time_font;
static GFont *s_date_font;
static GFont *s_summary_font;
static GBitmap *s_weather_bitmap;
static BitmapLayer *s_weather_bitmap_layer;
static TextLayer *s_weather_text_layer;
static TextLayer *s_weather_summary_layer;
static Layer *s_weather_day_summary_layer;
static GBitmap *s_day_summary_first;
static BitmapLayer *s_day_summary_first_layer;
static GBitmap *s_day_summary_second;
static BitmapLayer *s_day_summary_second_layer;
static TextLayer *s_day_summary_text_first_layer;
static TextLayer *s_day_summary_text_second_layer;
static TextLayer *s_day_summary_first_day_layer;
static TextLayer *s_day_summary_second_day_layer;

// https://api.forecast.io/forecast/acb79d16706f871691877ca0e5a9f346/40.68724460903619,-111.84785331609831

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "09:06";
  static char dateBuffer[] = "Mon 30";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%l:%M", tick_time);
  }
  
  // Write current Day Date
  strftime(dateBuffer, sizeof("Mon 30"), "%a %d", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, dateBuffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 15 minutes
  if(tick_time->tm_min % 15 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  bool summary = false;
  bool day_summary = true;
  
  if (axis == ACCEL_AXIS_Y) {
    if (layer_get_hidden(s_weather_day_summary_layer) == true) { 
      summary = true;
      day_summary = false;
    }
    layer_set_hidden(text_layer_get_layer(s_weather_summary_layer), summary);
    layer_set_hidden(s_weather_day_summary_layer, day_summary);  
  }
}

static void main_window_load(Window *window) {
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_REGULAR_34));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_REGULAR_12));
  s_summary_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_REGULAR_14));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 122, 95, 36));
  text_layer_set_background_color(s_time_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_time_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_time_layer, GColorWhite);
  #endif
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(105, 130, 30, 36));
  text_layer_set_background_color(s_date_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_date_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_date_layer, GColorWhite);
  #endif
  text_layer_set_overflow_mode(s_date_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Create Icon
  s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_DAY);
  s_weather_bitmap_layer = bitmap_layer_create(GRect(10, 5, 48, 48));
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
  #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_weather_bitmap_layer, GCompOpSet);
  #endif
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_bitmap_layer));
  
  // Create Weather Temperature Layer
  s_weather_text_layer = text_layer_create(GRect(56, 14, 83, 36));
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_weather_text_layer, GColorPictonBlue);
  #else
    text_layer_set_text_color(s_weather_text_layer, GColorWhite);
  #endif
  text_layer_set_font(s_weather_text_layer, s_time_font);
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_text_layer));
  
  // Create Weather Summary Layer
  s_weather_summary_layer = text_layer_create(GRect(5, 52, 134, 74));
  text_layer_set_background_color(s_weather_summary_layer, GColorClear);
  text_layer_set_text_color(s_weather_summary_layer, GColorWhite);
  text_layer_set_overflow_mode(s_weather_summary_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_weather_summary_layer, s_summary_font);
  text_layer_set_text_alignment(s_weather_summary_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_summary_layer));
  
  // Create Weather Day Summary Layer (default hidden)
  s_weather_day_summary_layer = layer_create(GRect(0, 52, 144, 74));
  layer_set_hidden(s_weather_day_summary_layer, true);
  layer_add_child(window_get_root_layer(window), s_weather_day_summary_layer);
  
  // Create Day Layers
  // First
  s_day_summary_first_day_layer = text_layer_create(GRect(20, 0, 36, 18));
  text_layer_set_background_color(s_day_summary_first_day_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_day_summary_first_day_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_day_summary_first_day_layer, GColorWhite);
  #endif
  text_layer_set_font(s_day_summary_first_day_layer, s_summary_font);
  text_layer_set_text_alignment(s_day_summary_first_day_layer, GTextAlignmentCenter);
  layer_add_child(s_weather_day_summary_layer, text_layer_get_layer(s_day_summary_first_day_layer));

  // Second
  s_day_summary_second_day_layer = text_layer_create(GRect(88, 0, 36, 18));
  text_layer_set_background_color(s_day_summary_second_day_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_day_summary_second_day_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_day_summary_second_day_layer, GColorWhite);
  #endif
  text_layer_set_font(s_day_summary_second_day_layer, s_summary_font);
  text_layer_set_text_alignment(s_day_summary_second_day_layer, GTextAlignmentCenter);
  layer_add_child(s_weather_day_summary_layer, text_layer_get_layer(s_day_summary_second_day_layer));
  
  // Create Summary Icons For 2 days
  // First
  s_day_summary_first = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_DAY);
  s_day_summary_first_layer = bitmap_layer_create(GRect(20, 20, 36, 36));
  bitmap_layer_set_bitmap(s_day_summary_first_layer, s_day_summary_first);
  #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_day_summary_first_layer, GCompOpSet);
  #endif
  layer_add_child(s_weather_day_summary_layer, bitmap_layer_get_layer(s_day_summary_first_layer));
  
  // Second
  s_day_summary_second = gbitmap_create_with_resource(RESOURCE_ID_CLEAR_DAY);
  s_day_summary_second_layer = bitmap_layer_create(GRect(88, 20, 36, 36));
  bitmap_layer_set_bitmap(s_day_summary_second_layer, s_day_summary_second);
  #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_day_summary_second_layer, GCompOpSet);
  #endif
  layer_add_child(s_weather_day_summary_layer, bitmap_layer_get_layer(s_day_summary_second_layer));
  
  // Create Summary Temperature layers
  // First
  s_day_summary_text_first_layer = text_layer_create(GRect(20, 56, 36, 18));
  text_layer_set_background_color(s_day_summary_text_first_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_day_summary_text_first_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_day_summary_text_first_layer, GColorWhite);
  #endif
  text_layer_set_font(s_day_summary_text_first_layer, s_summary_font);
  text_layer_set_text_alignment(s_day_summary_text_first_layer, GTextAlignmentCenter);
  layer_add_child(s_weather_day_summary_layer, text_layer_get_layer(s_day_summary_text_first_layer));
  
  // Second
  s_day_summary_text_second_layer = text_layer_create(GRect(88, 56, 36, 18));
  text_layer_set_background_color(s_day_summary_text_second_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_day_summary_text_second_layer, GColorOrange);
  #else
    text_layer_set_text_color(s_day_summary_text_second_layer, GColorWhite);
  #endif
  text_layer_set_font(s_day_summary_text_second_layer, s_summary_font);
  text_layer_set_text_alignment(s_day_summary_text_second_layer, GTextAlignmentCenter);
  layer_add_child(s_weather_day_summary_layer, text_layer_get_layer(s_day_summary_text_second_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_summary_font);
  // Destroy Weather Icon
  gbitmap_destroy(s_weather_bitmap);
  // Destroy Weather Icon Layer
  bitmap_layer_destroy(s_weather_bitmap_layer);
  // Destroy Weather Text Layer
  text_layer_destroy(s_weather_text_layer);
  // Destroy Weather Summary Layer
  text_layer_destroy(s_weather_summary_layer);
  // Destroy summary days layers
  text_layer_destroy(s_day_summary_first_day_layer);
  text_layer_destroy(s_day_summary_second_day_layer);
  // Destroy summary Icons
  gbitmap_destroy(s_day_summary_first);
  bitmap_layer_destroy(s_day_summary_first_layer);
  gbitmap_destroy(s_day_summary_second);
  bitmap_layer_destroy(s_day_summary_second_layer);
  // Destroy Temperature Summary Layers
  text_layer_destroy(s_day_summary_text_first_layer);
  text_layer_destroy(s_day_summary_text_second_layer);
  // Destroy Weather Day Summary Layer
  layer_destroy(s_weather_day_summary_layer);
}

static int get_resource_id_for_forecast(int icon_int) {
  int resource_id = 0;
  switch(icon_int) {
    case 0:
      resource_id = RESOURCE_ID_CLEAR_DAY;
      break;
    case 1:
      resource_id = RESOURCE_ID_CLEAR_NIGHT;
      break;
    case 2:
      resource_id = RESOURCE_ID_RAIN;
      break;
    case 3:
      resource_id = RESOURCE_ID_SNOW;
      break;
    case 4:
      resource_id = RESOURCE_ID_SLEET;
      break;
    case 5:
      resource_id = RESOURCE_ID_WIND;
      break;
    case 6:
      resource_id = RESOURCE_ID_FOG;
      break;
    case 7:
      resource_id = RESOURCE_ID_CLOUDY;
      break;
    case 8:
      resource_id = RESOURCE_ID_PARTLY_CLOUDY_DAY;
      break;
    case 9:
      resource_id = RESOURCE_ID_PARTLY_CLOUDY_NIGHT;
      break;
    case 10:
      resource_id = RESOURCE_ID_HAIL;
      break;
    case 11:
      resource_id = RESOURCE_ID_THUNDERSTORM;
      break;
    case 12:
      resource_id = RESOURCE_ID_CLEAR_DAY_36;
      break;
    case 13:
      resource_id = RESOURCE_ID_CLEAR_NIGHT_36;
      break;
    case 14:
      resource_id = RESOURCE_ID_RAIN_36;
      break;
    case 15:
      resource_id = RESOURCE_ID_SNOW_36;
      break;
    case 16:
      resource_id = RESOURCE_ID_SLEET_36;
      break;
    case 17:
      resource_id = RESOURCE_ID_WIND_36;
      break;
    case 18:
      resource_id = RESOURCE_ID_FOG_36;
      break;
    case 19:
      resource_id = RESOURCE_ID_CLOUDY_36;
      break;
    case 20:
      resource_id = RESOURCE_ID_PARTLY_CLOUDY_DAY_36;
      break;
    case 21:
      resource_id = RESOURCE_ID_PARTLY_CLOUDY_NIGHT_36;
      break;
    case 22:
      resource_id = RESOURCE_ID_HAIL_36;
      break;
    case 24:
      resource_id = RESOURCE_ID_THUNDERSTORM_36;
      break;
    default:
      resource_id = RESOURCE_ID_CLEAR_DAY;
      break;
  }
  return resource_id;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temperature_buffer[8];
  static char conditions_buffer[50];
  static int icon_int;
  static char day_one_temperature_buffer[8];
  static char day_two_temperature_buffer[8];
  static int day_one_icon;
  static int day_two_icon;
  static char day_one_buffer[5];
  static char day_two_buffer[5];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d˚", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_ICON:
      icon_int = (int)t->value->int32;
      break;
    case KEY_DAY_ONE_TEMPERATURE:
      snprintf(day_one_temperature_buffer, sizeof(day_one_temperature_buffer), "%d˚", (int)t->value->int32);
      break;
    case KEY_DAY_ONE_ICON:
      day_one_icon = (int)t->value->int32;
      break;
    case KEY_DAY_TWO_TEMPERATURE:
      snprintf(day_two_temperature_buffer, sizeof(day_two_temperature_buffer), "%d˚", (int)t->value->int32);
      break;
    case KEY_DAY_TWO_ICON:
      day_two_icon = (int)t->value->int32;
      break;
    case KEY_DAY_ONE_DAY:
      snprintf(day_one_buffer, sizeof(day_one_buffer), "%s", t->value->cstring);
      break;
    case KEY_DAY_TWO_DAY:
      snprintf(day_two_buffer, sizeof(day_two_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Key %d IS ICON!", icon_int);
  // Let's try destroying our bitmaps first
  gbitmap_destroy(s_weather_bitmap);
  gbitmap_destroy(s_day_summary_first);
  gbitmap_destroy(s_day_summary_second);
  
  // Set our Weather and Summary
  text_layer_set_text(s_weather_text_layer, temperature_buffer);
  text_layer_set_text(s_weather_summary_layer, conditions_buffer);
  // Set the main weather icon
  s_weather_bitmap = gbitmap_create_with_resource(get_resource_id_for_forecast(icon_int));
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
  // Set Day summary Days
  text_layer_set_text(s_day_summary_first_day_layer, day_one_buffer);
  text_layer_set_text(s_day_summary_second_day_layer, day_two_buffer);
  // Set Day Summary Icons
  s_day_summary_first = gbitmap_create_with_resource(get_resource_id_for_forecast(day_one_icon));
  s_day_summary_second = gbitmap_create_with_resource(get_resource_id_for_forecast(day_two_icon));
  bitmap_layer_set_bitmap(s_day_summary_first_layer, s_day_summary_first);
  bitmap_layer_set_bitmap(s_day_summary_second_layer, s_day_summary_second);
  // Set Day Summary Weather
  text_layer_set_text(s_day_summary_text_first_layer, day_one_temperature_buffer);
  text_layer_set_text(s_day_summary_text_second_layer, day_two_temperature_buffer);
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
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Register tap handler to show Day summary
  accel_tap_service_subscribe(tap_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Make sure the time is displayed from the start
  update_time();

}

static void deinit() {
  // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}