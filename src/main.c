#include <pebble.h>

Window *window;
TextLayer *hhmm_layer;
BitmapLayer *background_layer, *sec1_layer, *sec2_layer;

static GBitmap *background, *seconds;
static GFont digitM;

char hhmmBuffer[] = "00:00";

//-----------------------------------------------------------------------------------------------------------------------
void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	int secs = tick_time->tm_sec;
	GRect sub_rect;
	sub_rect = GRect(47*(secs%10), 0, 47*(secs%10)+47, 53);
	bitmap_layer_set_bitmap(sec2_layer, gbitmap_create_as_sub_bitmap(seconds, sub_rect));
	if (secs%10 == 0 || units_changed == MINUTE_UNIT)
	{
		sub_rect = GRect(47*(secs/10), 0, 47*(secs/10)+47, 53);
		bitmap_layer_set_bitmap(sec1_layer, gbitmap_create_as_sub_bitmap(seconds, sub_rect));
	}

	if(secs == 0 || units_changed == MINUTE_UNIT)
	{
		if(clock_is_24h_style())
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%H.%M", tick_time);
		else
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%I.%M", tick_time);
		
		//strcpy(hhmmBuffer, "23.57");
		//strftime(ssBuffer, sizeof(ssBuffer), "%S.%S", tick_time);
		text_layer_set_text(hhmm_layer, hhmmBuffer);
	}
}
//-----------------------------------------------------------------------------------------------------------------------
static void update_configuration(void)
{
	//Get a time structure so that it doesn't start blank
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
}
//-----------------------------------------------------------------------------------------------------------------------
void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	//Init Background
	background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	background_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_background_color(background_layer, GColorClear);
	bitmap_layer_set_bitmap(background_layer, background);
	layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
	
	//SECOND layer
	seconds = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECONDS);
	sec1_layer = bitmap_layer_create(GRect(23, 35, 47, 53));
	layer_add_child(window_layer, bitmap_layer_get_layer(sec1_layer));
	sec2_layer = bitmap_layer_create(GRect(75, 35, 47, 53));
	layer_add_child(window_layer, bitmap_layer_get_layer(sec2_layer));
     
	digitM = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ST_37));
  
	//HOUR+MINUTE layer
	hhmm_layer = text_layer_create(GRect(0, 100, 144, 40));
	text_layer_set_background_color(hhmm_layer, GColorClear);
	text_layer_set_text_color(hhmm_layer, GColorWhite);
	text_layer_set_text_alignment(hhmm_layer, GTextAlignmentCenter);
	text_layer_set_font(hhmm_layer, digitM);
	layer_add_child(window_layer, text_layer_get_layer(hhmm_layer));
        
	//Update Configuration
	update_configuration();
}
//-----------------------------------------------------------------------------------------------------------------------
void window_unload(Window *window)
{
	//Destroy text layers
	text_layer_destroy(hhmm_layer);
	
	//Unload Fonts
	fonts_unload_custom_font(digitM);
	
	//Destroy BitmapLayers
	bitmap_layer_destroy(sec2_layer);
	bitmap_layer_destroy(sec1_layer);
	bitmap_layer_destroy(background_layer);

	//Destroy GBitmaps
	gbitmap_destroy(background);
	gbitmap_destroy(seconds);
}
//-----------------------------------------------------------------------------------------------------------------------
void handle_init(void) 
{
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
    window_stack_push(window, true);
	
	//Subscribe services
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler)tick_handler);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
}
//-----------------------------------------------------------------------------------------------------------------------
void handle_deinit(void) 
{
	tick_timer_service_unsubscribe();
	window_destroy(window);
}
//-----------------------------------------------------------------------------------------------------------------------
int main(void) 
{
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
//----------------------------------