#include <pebble.h>
#include "netdownload.h"
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif
static Window *window;
static BitmapLayer *bitmap_layer;
static GBitmap *current_bmp;

static TextLayer *time_layer;
static TextLayer *time_shadow0;
static TextLayer *time_shadow1;
static TextLayer *time_shadow2;
static TextLayer *time_shadow3;

static TextLayer *date_layer;
static TextLayer *date_shadow0;
static TextLayer *date_shadow1;
static TextLayer *date_shadow2;
static TextLayer *date_shadow3;

static char buffer_time[] = "00:00";
static char buffer_date[] = "00/00";

static char *image = "http://pi.amcolash.com:5000/image.png";

struct tm *last_check;

void show_image() {
  // show that we are loading by showing no image
  bitmap_layer_set_bitmap(bitmap_layer, NULL);

  // Unload the current image if we had one and save a pointer to this one
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
    current_bmp = NULL;
  }

  netdownload_request(image);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  GRect bounds = layer_get_bounds(window_layer);

  bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  current_bmp = NULL;
  
  int time_x = 0;
  int time_y = 90;
  // Set up main time layer
  time_layer = text_layer_create(GRect(time_x, time_y, 144, 50));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  int time_thickness = 3;
  // Set up shadow layers for time
  // Left
  time_shadow0 = text_layer_create(GRect(time_x - time_thickness, time_y, 144, 50));
  text_layer_set_background_color(time_shadow0, GColorClear);
  text_layer_set_font(time_shadow0, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_shadow0, GTextAlignmentCenter);
  // Right
  time_shadow1 = text_layer_create(GRect(time_x + time_thickness, time_y, 144, 50));
  text_layer_set_background_color(time_shadow1, GColorClear);
  text_layer_set_font(time_shadow1, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_shadow1, GTextAlignmentCenter);
  // Up
  time_shadow2 = text_layer_create(GRect(time_x, time_y - time_thickness, 144, 50));
  text_layer_set_background_color(time_shadow2, GColorClear);
  text_layer_set_font(time_shadow2, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_shadow2, GTextAlignmentCenter);
  // Down
  time_shadow3 = text_layer_create(GRect(time_x, time_y + time_thickness, 144, 50));
  text_layer_set_background_color(time_shadow3, GColorClear);
  text_layer_set_font(time_shadow3, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_shadow3, GTextAlignmentCenter);
  
  // Add time shadows to window
  layer_add_child(window_layer, text_layer_get_layer(time_shadow0));
  layer_add_child(window_layer, text_layer_get_layer(time_shadow1));
  layer_add_child(window_layer, text_layer_get_layer(time_shadow2));
  layer_add_child(window_layer, text_layer_get_layer(time_shadow3));
  // Time after shadows
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  int date_x = 0;
  int date_y = 136;
  // Set up main date layer
  date_layer = text_layer_create(GRect(date_x, date_y, 144, 40));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  
  int date_thickness = 2;
  // Set up shadow layers for date
  // Left
  date_shadow0 = text_layer_create(GRect(date_x - date_thickness, date_y, 144, 40));
  text_layer_set_background_color(date_shadow0, GColorClear);
  text_layer_set_font(date_shadow0, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_shadow0, GTextAlignmentCenter);
  // Right
  date_shadow1 = text_layer_create(GRect(date_x + date_thickness, date_y, 144, 40));
  text_layer_set_background_color(date_shadow1, GColorClear);
  text_layer_set_font(date_shadow1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_shadow1, GTextAlignmentCenter);
  // Up
  date_shadow2 = text_layer_create(GRect(date_x, date_y - date_thickness, 144, 40));
  text_layer_set_background_color(date_shadow2, GColorClear);
  text_layer_set_font(date_shadow2, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_shadow2, GTextAlignmentCenter);
  // Down
  date_shadow3 = text_layer_create(GRect(date_x, date_y + date_thickness, 144, 40));
  text_layer_set_background_color(date_shadow3, GColorClear);
  text_layer_set_font(date_shadow3, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_shadow3, GTextAlignmentCenter);
  
  // Add date shadows to window
  layer_add_child(window_layer, text_layer_get_layer(date_shadow0));
  layer_add_child(window_layer, text_layer_get_layer(date_shadow1));
  layer_add_child(window_layer, text_layer_get_layer(date_shadow2));
  layer_add_child(window_layer, text_layer_get_layer(date_shadow3));
  // Date text after shadows
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

static void update_time(struct tm *tick_time) {
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer_time, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    int hour = tick_time->tm_hour;
    strftime(buffer_time, sizeof("00:00"), "%l:%M", tick_time);
    // Trim space character
    if ((hour > 0 && hour < 9) || (hour > 11 && hour < 22)) {
      int i;
      for (i = 0; i < 4; i++) {
        buffer_time[i] = buffer_time[i+1];
      }
      buffer_time[4] = '\0';
    }
  }
  
  int month = tick_time->tm_mon;
  int date = tick_time->tm_mday;
  int offset;
  strftime(buffer_date, sizeof("00/00"), "%m/%d", tick_time);
  // Trim 0 in month
  if (month < 10) {
    offset = 1;
    int i;
    for (i = 0; i < 4; i++) {
      buffer_date[i] = buffer_date[i+1];
    }
    buffer_date[4] = '\0';
  }
  // Trim 0 in date
  if (date < 10) {
    int i;
    for (i = 1+offset; i < 4+offset; i++) {
      buffer_date[i] = buffer_date[i+1];
    }
    buffer_date[4+offset] = '\0';
  }

  // Display this time on the TextLayers
  text_layer_set_text(time_layer, buffer_time);
  text_layer_set_text(time_shadow0, buffer_time);
  text_layer_set_text(time_shadow1, buffer_time);
  text_layer_set_text(time_shadow2, buffer_time);
  text_layer_set_text(time_shadow3, buffer_time);
  
  // Display the date on the TextLayers
  text_layer_set_text(date_layer, buffer_date);
  text_layer_set_text(date_shadow0, buffer_date);
  text_layer_set_text(date_shadow1, buffer_date);
  text_layer_set_text(date_shadow2, buffer_date);
  text_layer_set_text(date_shadow3, buffer_date);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
//   // Use a long-lived buffer
//   if (!current_bmp && lastCheck == 0) {
//     printf("showimage\n");
//     show_image();
//     lastCheck = 15;
//   } else if (!current_bmp) {
//     printf("tick\n");
//     lastCheck--;
//   }
  
  if (!current_bmp || (last_check->tm_mday != tick_time->tm_mday && tick_time->tm_min > 1)) {
    printf("showimage\n");
    show_image();
    time_t temp = time(NULL);
    last_check = localtime(&temp);
  }
  
  update_time(tick_time);
}

void download_complete_handler(NetDownload *download) {
  printf("Loaded image with %lu bytes", download->length);
  printf("Heap free is %u bytes", heap_bytes_free());

  #ifdef PBL_PLATFORM_APLITE
  GBitmap *bmp = gbitmap_create_with_png_data(download->data, download->length);
  #else
    GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
  #endif
  bitmap_layer_set_bitmap(bitmap_layer, bmp);

  // Save pointer to currently shown bitmap (to free it)
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
  }
  current_bmp = bmp;

  // Free the memory now
  #ifdef PBL_PLATFORM_APLITE
  // gbitmap_create_with_png_data will free download->data
  #else
    free(download->data);
  #endif
  // We null it out now to avoid a double free
  download->data = NULL;
  netdownload_destroy(download);
  
  // Change tick handler to every minute now to save some battery
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(current_bmp);
}

static void init(void) {
  // Need to initialize this first to make sure it is there when
  // the window_load function is called by window_stack_push.
  netdownload_initialize(download_complete_handler);

  window = window_create();
#ifdef PBL_SDK_2
  window_set_fullscreen(window, true);
#endif
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  netdownload_deinitialize(); // call this to avoid 20B memory leak
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}