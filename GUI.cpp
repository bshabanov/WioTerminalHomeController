#include <lvgl.h>
#include <TFT_eSPI.h>
#include "HomeAPI.h"
#define LVGL_TICK_PERIOD 16
#define IS_LANSCAPE false

struct Orientation {
  int type;
  int width;
  int height;
  int safeWidth;
  int safeHeight;
};

const Orientation portrait = {2, 240, 320, 220, 250};
const Orientation landscape = {3, 320, 240, 300, 170};

static TFT_eSPI tft = TFT_eSPI();
static HomeAPI api;
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
static lv_color_t buf2[LV_HOR_RES_MAX * 10];

class GUI {
  public:
    inline static Orientation orientation;
    inline static uint16_t tabIndex = 0;
    inline static lv_group_t * groupTab0;
    inline static lv_group_t * groupTab1;
    inline static lv_group_t * groupTab2;
    inline static lv_indev_t* input;

    /* Display flushing */
    static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
      uint16_t c;

      tft.startWrite(); /* Start new TFT transaction */
      tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
      for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
          c = color_p->full;
          tft.writeColor(c, 1);
          color_p++;
        }
      }
      tft.endWrite(); /* terminate TFT transaction */
      lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
    }

    static bool BTN_A(lv_indev_drv_t * indev, lv_indev_data_t * data) {
      return processBTN(WIO_KEY_A, data);
    }

    static bool BTN_B(lv_indev_drv_t * indev, lv_indev_data_t * data) {
      return processBTN(WIO_KEY_B, data);
    }

    static bool BTN_C(lv_indev_drv_t * indev, lv_indev_data_t * data) {
      return processBTN(WIO_KEY_C, data);
    }

    static bool processBTN(int pin, lv_indev_data_t * data) {
      bool btn_pr = digitalRead(pin) == LOW;
      if (btn_pr) {
        data->state = LV_INDEV_STATE_PR;
      } else {
        data->state = LV_INDEV_STATE_REL;
      }
      return false;
    }

    static bool keypad_navigation(lv_indev_drv_t * drv, lv_indev_data_t*data) {
      bool isPressed = true;
      uint16_t UP = tabIndex == 0 ? LV_KEY_UP : LV_KEY_PREV;
      uint16_t DOWN = tabIndex == 0 ? LV_KEY_DOWN : LV_KEY_NEXT;
      if (digitalRead(WIO_5S_UP) == LOW) {
        data->key = orientation.type == 2 ? LV_KEY_RIGHT : UP;
      } else if (digitalRead(WIO_5S_DOWN) == LOW) {
        data->key = orientation.type == 2 ? LV_KEY_LEFT : DOWN;
      } else if (digitalRead(WIO_5S_LEFT) == LOW) {
        data->key = orientation.type == 2 ? UP : LV_KEY_LEFT;
      } else if (digitalRead(WIO_5S_RIGHT) == LOW) {
        data->key = orientation.type == 2 ? DOWN : LV_KEY_RIGHT;
      } else if (digitalRead(WIO_5S_PRESS) == LOW) {
        data->key = LV_KEY_ENTER;
      } else {
        isPressed = false;
      }

      if (isPressed) {
        data->state = LV_INDEV_STATE_PR;
      } else {
        data->state = LV_INDEV_STATE_REL;
      }

      return false; /*No buffering now so no more data read*/
    }

    static void toggleKitchenLight(lv_obj_t * obj, lv_event_t event) {
      if (event == LV_EVENT_VALUE_CHANGED) {
        bool state = lv_switch_get_state(obj);
        api.lightKitchen(state);
      }
    }

    static void toggleLivingRoomLight(lv_obj_t * obj, lv_event_t event) {

    }

    static void handle_tv(lv_obj_t * obj, lv_event_t event) {
      if (event == LV_EVENT_VALUE_CHANGED) {
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);

        if (txt == "MUTE") {
          api.tvMute();
        } else if (txt == "V+") {
          api.tvVolUp();
        } else if (txt == "V-") {
          api.tvVolDown();
        } else if (txt == "POWER") {
          api.tvPower();
        } else if (txt == "SOURCE") {
          api.tvSource();
        }
      }
    }

    static void setup() {
      // TAB VIEW CONTROLLER
      pinMode(WIO_KEY_C, INPUT_PULLUP);
      pinMode(WIO_KEY_B, INPUT_PULLUP);
      pinMode(WIO_KEY_A, INPUT_PULLUP);

      pinMode(WIO_5S_UP, INPUT_PULLUP);
      pinMode(WIO_5S_DOWN, INPUT_PULLUP);
      pinMode(WIO_5S_LEFT, INPUT_PULLUP);
      pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
      pinMode(WIO_5S_PRESS, INPUT_PULLUP);

      pinMode(WIO_BUZZER, OUTPUT);

      orientation = IS_LANSCAPE ? landscape : portrait;

      tft.begin(); /* TFT init */
      tft.setRotation(orientation.type); /* Landscape orientation */

      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      tft.drawString("Initializing!", 10, 10);

      api.setup(&tft);

      lv_init();

      lv_disp_buf_init(&disp_buf, buf, buf2, LV_HOR_RES_MAX * 10);

      /*Initialize the display*/
      lv_disp_drv_t disp_drv;
      lv_disp_drv_init(&disp_drv);
      disp_drv.hor_res = orientation.width;
      disp_drv.ver_res = orientation.height;
      disp_drv.flush_cb = my_disp_flush;
      disp_drv.buffer = &disp_buf;
      lv_disp_drv_register(&disp_drv);

      int cellWidth = orientation.width / 3;
      int cellPadding = 5;

      /*Initialize the button*/
      lv_indev_drv_t indev_drv1;
      lv_indev_drv_init(&indev_drv1);
      indev_drv1.type = LV_INDEV_TYPE_BUTTON;
      indev_drv1.read_cb = BTN_C;
      lv_indev_t * my_indev1 = lv_indev_drv_register(&indev_drv1);
      static const lv_point_t points_array1[] =  { {cellPadding, cellPadding}, {cellWidth * 1 - cellPadding, cellPadding}};
      lv_indev_set_button_points(my_indev1, points_array1);

      lv_indev_drv_t indev_drv2;
      lv_indev_drv_init(&indev_drv2);
      indev_drv2.type = LV_INDEV_TYPE_BUTTON;
      indev_drv2.read_cb = BTN_B;
      lv_indev_t * my_indev2 = lv_indev_drv_register(&indev_drv2);
      static const lv_point_t points_array2[] =  { {cellWidth * 1 + cellPadding, cellPadding}, {cellWidth * 2 - cellPadding, cellPadding}};
      lv_indev_set_button_points(my_indev2, points_array2);

      lv_indev_drv_t indev_drv3;
      lv_indev_drv_init(&indev_drv3);
      indev_drv3.type = LV_INDEV_TYPE_BUTTON;
      indev_drv3.read_cb = BTN_A;
      lv_indev_t * my_indev3 = lv_indev_drv_register(&indev_drv3);
      static const lv_point_t points_array3[] =  { {cellWidth * 2 + cellPadding, cellPadding}, {cellWidth * 3 - cellPadding, cellPadding}};
      lv_indev_set_button_points(my_indev3, points_array3);

      lv_indev_drv_t indev_drv4;
      lv_indev_drv_init(&indev_drv4);
      indev_drv4.type = LV_INDEV_TYPE_KEYPAD;
      indev_drv4.read_cb = keypad_navigation;
      input = lv_indev_drv_register(&indev_drv4);

      lv_task_t * task = lv_task_create(tabView, 500, LV_TASK_PRIO_MID, NULL);
      lv_task_set_repeat_count(task, 1);
    }

    static void tick() {
      lv_tick_inc(LVGL_TICK_PERIOD);
      lv_task_handler();
    }

    // TV SCREEN
    static lv_group_t* tabTV(lv_obj_t *parent) {
      static const char * btnm_map[] = {"POWER", "\n", "MUTE", "\n", "SOURCE", "\n", "V-", "V+", ""};

      lv_obj_t * btnm1 = lv_btnmatrix_create(parent, NULL);
      lv_btnmatrix_set_map(btnm1, btnm_map);
      lv_obj_set_size(btnm1, orientation.safeWidth, orientation.safeHeight);
      lv_obj_align(btnm1, NULL, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_event_cb(btnm1, handle_tv);

      lv_group_t * g = lv_group_create();
      lv_group_add_obj(g, btnm1);
      return g;
    }

    // AC SCREEN
    static lv_group_t* tabAC(lv_obj_t *parent) {

      /* Create a label below the slider */
      static lv_obj_t * slider_label;
      slider_label = lv_label_create(parent, NULL);
      lv_label_set_text(slider_label, "20");
      lv_obj_set_auto_realign(slider_label, true);

      /* Create a slider in the center of the display */
      lv_obj_t * slider = lv_slider_create(parent, NULL);
      lv_obj_set_width(slider, LV_DPI * 2);
      lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);
      auto glambda = [](lv_obj_t * slider, lv_event_t event) {
        if (event == LV_EVENT_VALUE_CHANGED) {
          static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
          snprintf(buf, 4, "%u", lv_slider_get_value(slider));
          lv_label_set_text(slider_label, buf);
        }
      };
      lv_obj_set_event_cb(slider, glambda);
      lv_slider_set_range(slider, 20, 26);
      lv_obj_align(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

      static const char * btnm_map[] = {"ON HOT", "ON COOL", "\n", "POWER OFF", ""};

      lv_obj_t * btnm1 = lv_btnmatrix_create(parent, NULL);
      lv_btnmatrix_set_map(btnm1, btnm_map);
      lv_obj_set_size(btnm1, orientation.safeWidth, 100);
      lv_obj_align(btnm1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);

      lv_group_t * g = lv_group_create();
      lv_group_add_obj(g, slider);
      lv_group_add_obj(g, btnm1);
      return g;
    }

    // LIGHTS SCREEN
    static lv_group_t* tabLights(lv_obj_t *parent) {
      lv_obj_t * label1 = lv_label_create(parent, NULL);
      lv_label_set_text(label1, "Kitchen Light");
      lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 25, 20);

      lv_obj_t *sw1 = lv_switch_create(parent, NULL);
      lv_obj_align(sw1, NULL, LV_ALIGN_IN_TOP_LEFT, 25, 20);
      lv_obj_set_event_cb(sw1, toggleKitchenLight);

      lv_obj_t * label2 = lv_label_create(parent, NULL);
      lv_label_set_text(label2, "Living room");
      lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 25, 60);

      lv_obj_t *sw2 = lv_switch_create(parent, NULL);
      lv_obj_align(sw2, NULL, LV_ALIGN_IN_TOP_LEFT, 25, 60);
      lv_obj_set_event_cb(sw2, toggleLivingRoomLight);

      lv_group_t * g = lv_group_create();
      lv_group_add_obj(g, sw1);
      lv_group_add_obj(g, sw2);
      return g;
    }

    static void onTabChange(lv_obj_t * obj, lv_event_t event) {
      if (event == LV_EVENT_VALUE_CHANGED) {
        switch (lv_tabview_get_tab_act(obj)) {
          case 0: lv_indev_set_group(input, groupTab0); tabIndex = 0; break;
          case 1: lv_indev_set_group(input, groupTab1); tabIndex = 1; break;
          case 2: lv_indev_set_group(input, groupTab2); tabIndex = 2; break;
        }
      }
    }

    // MAIN INTERFACE
    static void tabView(lv_task_t * task) {
      /*Create a Tab view object*/
      lv_obj_t *tabview;
      tabview = lv_tabview_create(lv_scr_act(), NULL);

      /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
      lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "TV");
      lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "AC");
      lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "LIGHTS");

      lv_tabview_set_anim_time(tabview, 100);

      /*Add content to the tabs*/
      groupTab0 = tabTV(tab1);
      groupTab1 = tabAC(tab2);
      groupTab2 = tabLights(tab3);

      lv_obj_set_event_cb(tabview, onTabChange);
      lv_indev_set_group(input, groupTab0);
    }
};
