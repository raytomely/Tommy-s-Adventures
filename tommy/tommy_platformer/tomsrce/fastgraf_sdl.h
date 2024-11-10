#ifndef FASTGRAF_SDL_H
#define FASTGRAF_SDL_H

#include <SDL/SDL.h>
#include <time.h>

#define KB_ALT      SDLK_LALT
#define KB_CTRL     SDLK_LCTRL
#define KB_ESC      SDLK_ESCAPE
#define KB_SPACE    SDLK_SPACE
#define KB_UP       SDLK_UP
#define KB_LEFT     SDLK_LEFT
#define KB_RIGHT    SDLK_RIGHT
#define KB_DOWN     SDLK_DOWN
#define KB_F1       SDLK_F1
#define KB_F2       SDLK_F2
#define KB_W        SDLK_w
#define KB_D        SDLK_d

extern int graphics_x, graphics_y, org_x, org_y, fb_width;
extern int clip_x1, clip_x2, clip_y1, clip_y2;
extern unsigned char current_color;
extern unsigned char transcolors;
extern SDL_Surface *screen;
extern SDL_Rect clip_rect;
extern SDL_Event event;
extern Uint8 *keystate;

void read_pcx( const char *name );
void read_pcx_to_screen( const char *name );
void blit_raw(int x1, int x2, int y1, int y2, int x, int y);
void blit_raw_alpha(int x1, int x2, int y1, int y2, int x, int y);
void blit_scaled_X2_all(void);
void blit_raw_all(void);
void display_run_image(char *map_array, int runs, int width);
void draw_bitmap(char *map_array, int width, int height);
void draw_bitmap_scaled_X2(char *map_array, int width, int height);
void draw_bitmap_raw(char *map_array, int width, int height);
void draw_bitmap_raw_flipped(char *map_array, int width, int height);
void getimage(char *map_array, int width, int height);
void replace_palette_entries(int start, int count, char *values);
int read_keyboard(void);
int get_input(void);
void read_mouse(short *x, short *y, short *button);
void FG_SDL_init(void);
void FG_SDL_quit(void);
void processInput(void);
void sleep(void);
void init_clock(int speed);
void main_event_loop(void);
void print_level(void);
void draw_box( int x1, int y1, int x2, int y2, char col );
void draw_boxx( int x1, int y1, int x2, int y2, char col );
void draw_rect(int x1, int x2, int y1, int y2);
void draw_rect2(int x1, int x2, int y1, int y2);

#define clear_screen()  SDL_FillRect(screen, &screen_rect, 32)
#define show_screen()  SDL_Flip(screen)
#define scroll_box()  draw_box(org_x, org_y, org_x + 320, org_y + 200, 0); show_screen()
#define update_keystate()  SDL_PumpEvents(); keystate = SDL_GetKeyState(NULL); sleep()
#define draw_clip_rect()  draw_box( clip_x1, clip_y1, clip_x2, clip_y2, 30 ); show_screen()

#define fg_intkey(k,a)  *k = *a = read_keyboard(); sleep();
#define fg_transfer(x1,x2,y1,y2,x,y,s,d)  blit_raw(x1, x2, y1, y2, x, y)
#define fg_showpcx(fn,f)  if(f) read_pcx(fn); else read_pcx_to_screen(fn)
#define fg_move(x,y)  graphics_x = x; graphics_y = y
#define fg_mousepos(x,y,b)  read_mouse(x,y,b)
#define fg_setcolor(c)  current_color = c
#define fg_waitfor(t) SDL_Delay(t*60)
#define fg_boxx(x1,x2,y1,y2)  draw_boxx(x1, y1, x2, y2, current_color)
#define fg_mousevis(v) if(v) SDL_ShowCursor(SDL_ENABLE); else SDL_ShowCursor(SDL_DISABLE)
#define fg_pan(xp,yp)  org_x = xp; org_y = yp; blit_scaled_X2_all()//draw_box(org_x, org_y, org_x+320, org_y+200, 0)
#define fg_resize(w,h)  FG_SDL_init()
#define fg_reset()  FG_SDL_quit()
#define fg_print(fmt,n)  fg_sdl_print(fmt, n);
#define fg_getkey(k,a)  while(!(*k = *a = get_input())) sleep()
#define fg_rect(x1,x2,y1,y2)  draw_rect2(x1,x2,y1,y2)
#define fg_mouseini()  1

#define fg_setmode(m)
#define fg_mousefin()
#define fg_mouselim(x1,x2,y1,y2)
#define fg_mousemov(x,y)

#define fg_tcxfer(x1,x2,y1,y2,x,y,s,d)  blit_raw_alpha(x1, x2, y1, y2, x, y)
#define fg_display(m,r,w)  display_run_image(m, r, w)
#define fg_tcmask(m)  transcolors = (m) & 0x0F
#define fg_kbinit(a)  while(SDL_PollEvent(&event) == 1) sleep(); SDL_Delay(60)
#define fg_kbtest(k)  keystate[k]
#define fg_drwimage(m,w,h)  draw_bitmap_raw(m, w, h)
#define fg_getimage(m,w,h)  getimage(m, w, h)
#define fg_setclip(x1,x2,y1,y2)  clip_x1 = x1; clip_x2 = x2; clip_y1 = y1; clip_y2 = y2;
#define fg_flpimage(m,w,h)  draw_bitmap_raw_flipped(m, w, h)
#define fg_clpimage(m,w,h)  draw_bitmap_raw(m, w, h)
#define fg_setdacs(s,c,v)  replace_palette_entries(s, c, v)
#define fg_getclock()  time(0)
#define fg_testmode(a,b)  1
#define fg_waitkey()  while(!get_input()) sleep(); SDL_FillRect(screen, NULL, 0)
#define fg_drawmap(m,w,h)  draw_bitmap_scaled_X2(m, w, h)

#define _dos_setvect(a,b)
#define _dos_getvect(a)  NULL

#define far
#define near
#define interrupt

#undef main
#define int short

#endif // FASTGRAF_SDL_H
