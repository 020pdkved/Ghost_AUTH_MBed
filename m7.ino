/*
 * Project: GIGA-NEXUS [COMPLETE SYSTEM]
 * Status: WEB UI (WORKING) + HMI UI (FIXED)
 * Core: M7
 */

#include <Arduino_H7_Video.h>
#include <Arduino_GigaDisplayTouch.h>
#include <lvgl.h>
#include <WiFi.h>
#include "portal_ui.h" // Your Working Matrix UI

// --- NETWORK CONFIG (DO NOT TOUCH - IT WORKS) ---
char ap_ssid[] = "VED_SECURE_AP";
char ap_pass[] = "12345678";

// FORCE STATIC IP (Vital for stability)
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);
Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch Touch;

// MEMORY FIX: Point buffer to SDRAM start (Standard)
// We will rely on careful memory management instead of offsets
uint16_t* lvgl_buf; 

// --- UI OBJECTS ---
lv_obj_t * ui_Screen;
lv_obj_t * kb;
lv_obj_t * ta_ssid;
lv_obj_t * ta_pass;
lv_obj_t * lbl_status;
lv_obj_t * lbl_ip;
bool is_touch_down = false; 

// --- DISPLAY DRIVER ---
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t screen_w = 800;
    
    // STANDARD GIGA FRAMEBUFFER ADDRESS
    uint16_t * fb = (uint16_t *) 0x60000000; 
    
    uint16_t * dest = fb + (area->y1 * screen_w) + area->x1;
    uint16_t * src = (uint16_t *) px_map;
    
    for (uint32_t y = 0; y < h; y++) {
        memcpy(dest, src, w * 2);
        dest += screen_w;
        src += w;
    }
    SCB_CleanDCache(); // Critical for H7
    lv_display_flush_ready(disp);
}

// --- TOUCH DRIVER ---
void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data) {
    GDTpoint_t points[1]; 
    if (Touch.getTouchPoints(points) > 0) {
        if (!is_touch_down) {
            data->state = LV_INDEV_STATE_PRESSED;
            data->point.x = points[0].x;
            data->point.y = points[0].y;
            is_touch_down = true;
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        is_touch_down = false;
    }
}

// --- UI EVENT HANDLERS ---
static void bg_cb(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && kb != NULL) 
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}
static void ta_cb(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_keyboard_set_textarea(kb, (lv_obj_t*)lv_event_get_target(e));
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(kb);
    }
}
static void btn_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        WiFi.begin(lv_textarea_get_text(ta_ssid), lv_textarea_get_text(ta_pass));
        lv_label_set_text(lbl_status, "STATUS: CONNECTING...");
    }
}

void build_ui() {
    ui_Screen = lv_screen_active();
    lv_obj_set_style_bg_color(ui_Screen, lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(ui_Screen, bg_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * t = lv_label_create(ui_Screen);
    lv_label_set_text(t, "VED_OS CONSOLE");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00ff00), 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 20);

    ta_ssid = lv_textarea_create(ui_Screen);
    lv_textarea_set_placeholder_text(ta_ssid, "SSID");
    lv_textarea_set_one_line(ta_ssid, true);
    lv_obj_set_width(ta_ssid, 350);
    lv_obj_align(ta_ssid, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_add_event_cb(ta_ssid, ta_cb, LV_EVENT_ALL, NULL);

    ta_pass = lv_textarea_create(ui_Screen);
    lv_textarea_set_placeholder_text(ta_pass, "PASSWORD");
    lv_textarea_set_password_mode(ta_pass, true);
    lv_textarea_set_one_line(ta_pass, true);
    lv_obj_set_width(ta_pass, 350);
    lv_obj_align(ta_pass, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_add_event_cb(ta_pass, ta_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * btn = lv_button_create(ui_Screen);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 190);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * l = lv_label_create(btn);
    lv_label_set_text(l, "CONNECT");
    lv_obj_center(l);

    lbl_status = lv_label_create(ui_Screen);
    lv_label_set_text(lbl_status, "STATUS: STANDBY");
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(0x888888), 0);
    lv_obj_align(lbl_status, LV_ALIGN_BOTTOM_LEFT, 20, -50);

    lbl_ip = lv_label_create(ui_Screen);
    lv_label_set_text(lbl_ip, "AP: 192.168.1.1");
    lv_obj_set_style_text_color(lbl_ip, lv_color_hex(0x00ff00), 0);
    lv_obj_align(lbl_ip, LV_ALIGN_BOTTOM_LEFT, 20, -20);

    kb = lv_keyboard_create(ui_Screen);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

void setup() {
    Serial.begin(115200);
    
    // 1. INIT DISPLAY (SCREEN FIX)
    Display.begin();
    Touch.begin();
    
    // 2. BUFFER ALLOCATION (SCREEN FIX)
    // We allocate in SDRAM but explicitly point to a safe region
    // 0x60000000 is FB. We use +1MB for LVGL buffer.
    lvgl_buf = (uint16_t*) (0x60100000); 

    // 3. INIT LVGL
    lv_init();
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_read);

    lv_display_t * disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, lvgl_buf, NULL, 800 * 40 * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 4. NETWORK STARTUP
    WiFi.disconnect();
    WiFi.end();
    delay(500);
    
    // FORCE STATIC IP
    WiFi.config(local_IP, gateway, subnet);
    WiFi.beginAP(ap_ssid, ap_pass);
    
    build_ui();
    server.begin();
}

void loop() {
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);

    WiFiClient client = server.accept();
    
    if (client) {
        String req = "";
        unsigned long timeout = millis();
        bool blankLine = true;
        
        while (client.connected() && millis() - timeout < 3000) {
            if (client.available()) {
                char c = client.read();
                req += c;
                if (c == '\n' && blankLine) {
                    
                    // Kill Favicon
                    if(req.indexOf("favicon") > 0) { client.stop(); return; }

                    // SERVE PAGE (CHUNKED 512)
                    int len = strlen_P(index_html);
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.print("Content-Length: "); client.println(len);
                    client.println("Connection: close");
                    client.println();
                    
                    const int CHUNK = 512;
                    char buf[CHUNK];
                    for(int i=0; i<len; i+=CHUNK) {
                        int r = min(CHUNK, len-i);
                        for(int j=0; j<r; j++) buf[j] = pgm_read_byte_near(index_html + i + j);
                        client.write((uint8_t*)buf, r);
                    }
                    client.flush();
                    break;
                }
                if (c == '\n') blankLine = true;
                else if (c != '\r') blankLine = false;
            }
        }
        delay(10);
        client.stop();
    }
}