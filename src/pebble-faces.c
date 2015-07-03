#include <pebble.h>
#include "netdownload.h"
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif
static Window *window;
static BitmapLayer *bitmap_layer;
static GBitmap *current_bmp;

static char *image = "http://pi.amcolash.com:5000/image.png";

static int downloaded;

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
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(current_bmp);
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
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use a long-lived buffer
  if (downloaded == 0) {
    printf("Tick\n");
    downloaded = 1;
    show_image();
  }
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