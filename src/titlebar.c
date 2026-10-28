/*
 * ============================================================================
 * fukiwm (吹雪ウィンドウマネージャー)
 * コアウィンドウマネージャー ＆ ネイティブタイトルバーエンジン
 * 
 * 設計・開発 (Author)  : phobos0_ (不慕子)
 * ライセンス (License) : GPLv3
 * 
 * macOSスタイルのタイトルバーとカスタムヒットボックスを統合した、
 * 完全に独立したbspwmの進化系。
 * ============================================================================
 */

#include <stdlib.h>
#include <xcb/xcb.h>
#include "bspwm.h"
#include "types.h"
#include "window.h"
#include "tree.h"
#include "events.h"
#include "pointer.h"
#include "titlebar.h"
#include "query.h"
#include "settings.h"

void titlebar_create(client_t *c) {
    if (c == NULL) return;

    c->titlebar = xcb_generate_id(dpy); 
    
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    uint32_t values[] = {titlebar_bg_color, 1, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_EXPOSURE};
    
    xcb_create_window(dpy, XCB_COPY_FROM_PARENT, c->titlebar, root,
                      0, 0, 10, titlebar_height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      XCB_COPY_FROM_PARENT, mask, values);
    xcb_map_window(dpy, c->titlebar);
}

void titlebar_draw(client_t *c) {

    if (c == NULL || c->titlebar == XCB_NONE) return;

    xcb_gcontext_t gc = xcb_generate_id(dpy);
    xcb_create_gc(dpy, gc, c->titlebar, 0, NULL);

    uint32_t colors[] = {shuuryou_btn, kakure_btn, saidaika_btn, houshiki_btn}; 
    int radius = 10;
    int y = (titlebar_height - radius) / 2;

    for (int i = 0; i < 4; i++) {
        int x = 10 + (i * (radius + 8));
        
        xcb_arc_t arc = {x, y, radius, radius, 0, 360 * 64}; 
        
        uint32_t val[] = {colors[i]};
        xcb_change_gc(dpy, gc, XCB_GC_FOREGROUND, val);
        xcb_poly_fill_arc(dpy, c->titlebar, gc, 1, &arc);
    }

    xcb_free_gc(dpy, gc);
    xcb_flush(dpy);
}

bool titlebar_handle_click(xcb_generic_event_t *evt) {

    xcb_button_press_event_t *e = (xcb_button_press_event_t *) evt;
    if (e->detail != XCB_BUTTON_INDEX_1) return false; 

    for (monitor_t *m = mon_head; m != NULL; m = m->next) {

        for (desktop_t *d = m->desk_head; d != NULL; d = d->next) {

            for (node_t *n = first_extrema(d->root); n != NULL; n = next_leaf(n, d->root)) {
                
                if (n->client != NULL && n->client->titlebar == e->event) {
                    int cx = e->event_x; 
                    
                    if (cx >= 8 && cx <= 22) {
                        close_node(n);
                        return true;
                    }

                    else if (cx >= 28 && cx <= 38) {
                        set_hidden(m, d, n, true); 
                        return true;
                    } 

                    else if (cx >= 46 && cx <= 56) {
                        client_state_t next = (n->client->state == STATE_FULLSCREEN) ? STATE_TILED : STATE_FULLSCREEN;
                        set_state(m, d, n, next); 
                        return true;
                    }

                    else if (cx >= 64 && cx <= 74) {
                        client_state_t next = (n->client->state == STATE_FLOATING) ? STATE_TILED : STATE_FLOATING;
                        set_state(m, d, n, next); 
                        arrange(m, d);
                        focus_node(m, d, n);
                        return true;
                    }

                        if (n->client->state != STATE_FLOATING) {
                        set_state(m, d, n, STATE_FLOATING);
                        arrange(m, d);
                    }

                    focus_node(m, d, n);
                    xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME); // X11 kilidini kır
                    grab_pointer(ACTION_MOVE); // bspwm motoruna devret
                    xcb_flush(dpy);
                    return true;           
                }
            }
        }
    }
    return false;
}

void titlebar_hook_move_resize(xcb_window_t win, int16_t *x, int16_t *y, uint16_t *w, uint16_t *h) {

    *y += titlebar_height;

    if (*h > titlebar_height) *h -= titlebar_height;

    coordinates_t loc;

    if (locate_window(win, &loc) && loc.node != NULL && loc.node->client != NULL) {

        client_t *c = loc.node->client;

        if (c->titlebar != XCB_NONE) {

            uint16_t tb_w = *w + (2 * c->border_width);
            uint32_t val[] = {*x, *y - titlebar_height, tb_w, titlebar_height};
            xcb_configure_window(dpy, c->titlebar, XCB_CONFIG_WINDOW_X_Y_WIDTH_HEIGHT, val);
            window_above(c->titlebar, win);
        }
    }
}

void titlebar_hook_resize(xcb_window_t win, uint16_t *w, uint16_t *h) {

    if (*h > titlebar_height) *h -= titlebar_height;

    coordinates_t loc;

    if (locate_window(win, &loc) && loc.node != NULL && loc.node->client != NULL) {

        client_t *c = loc.node->client;

        if (c->titlebar != XCB_NONE) {

            uint16_t tb_w = *w + (2 * c->border_width);
            uint32_t val[] = {tb_w, titlebar_height};
            xcb_configure_window(dpy, c->titlebar, XCB_CONFIG_WINDOW_WIDTH_HEIGHT, val);
        }
    }
}

void titlebar_hook_move(xcb_window_t win, int16_t *x, int16_t *y) {

    *y += titlebar_height;

    coordinates_t loc;

    if (locate_window(win, &loc) && loc.node != NULL && loc.node->client != NULL) {

        client_t *c = loc.node->client;

        if (c->titlebar != XCB_NONE) {

            uint32_t val[] = {*x, *y - titlebar_height};
            xcb_configure_window(dpy, c->titlebar, XCB_CONFIG_WINDOW_X_Y, val);
        }
    }
}

void titlebar_handle_expose(xcb_generic_event_t *evt) {

    xcb_expose_event_t *e = (xcb_expose_event_t *) evt;
    
    if (e->count != 0) return; 

    for (monitor_t *m = mon_head; m != NULL; m = m->next) {

        for (desktop_t *d = m->desk_head; d != NULL; d = d->next) {

            for (node_t *n = first_extrema(d->root); n != NULL; n = next_leaf(n, d->root)) {
                
                if (n->client != NULL && n->client->titlebar == e->window) {

                    titlebar_draw(n->client);
                    return;
                }
            }
        }
    }
}