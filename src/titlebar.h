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

#ifndef titlebar_h
#define titlebar_h

#include <xcb/xcb.h>
#include <stdbool.h>
#include "types.h"

#define titlebar_height 24
#define titlebar_bg_color 0x262626
#define shuuryou_btn 0xFF605C
#define kakure_btn 0xFFBD44
#define saidaika_btn 0x00CA4E
#define houshiki_btn 0x007AFF 

void titlebar_create(client_t *c);
void titlebar_draw(client_t *c);
bool titlebar_handle_click(xcb_generic_event_t *evt);
void titlebar_hook_move_resize(xcb_window_t win, int16_t *x, int16_t *y, uint16_t *w, uint16_t *h);
void titlebar_hook_resize(xcb_window_t win, uint16_t *w, uint16_t *h);
void titlebar_hook_move(xcb_window_t win, int16_t *x, int16_t *y);
void titlebar_handle_expose(xcb_generic_event_t *evt);

#endif