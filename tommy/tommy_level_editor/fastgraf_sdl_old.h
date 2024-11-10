#ifndef FASTGRAF_SDL_H
#define FASTGRAF_SDL_H

#include <SDL/SDL.h>

extern int graphics_x, graphics_y, org_x, org_y, fb_width;
extern unsigned char current_color;
extern SDL_Surface *screen;
extern SDL_Rect screen_rect;

void read_pcx( const char *name );
void blit_raw(int x1, int x2, int y1, int y2, int x, int y);
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

#define clear_screen()  SDL_FillRect(screen, &screen_rect, 32)
#define show_screen()  SDL_Flip(screen)
#define scroll_box()  draw_box(org_x, org_y, org_x + 320, org_y + 200, 0); show_screen()

#define fg_intkey(k,a)  *k = *a = read_keyboard(); sleep();
#define fg_transfer(x1,x2,y1,y2,x,y,s,d)  blit_raw(x1, x2, y1, y2, x, y)
#define fg_showpcx(fn,f)  read_pcx(fn)
#define fg_move(x,y)  graphics_x = x; graphics_y = y
#define fg_mousepos(x,y,b)  read_mouse(x,y,b)
#define fg_setcolor(c)  current_color = c
#define fg_waitfor(t) SDL_Delay(t)
#define fg_boxx(x1,x2,y1,y2)  draw_boxx(x1, y1, x2, y2, current_color)
#define fg_mousevis(v) if(v) SDL_ShowCursor(SDL_ENABLE); else SDL_ShowCursor(SDL_DISABLE)
#define fg_pan(xp,yp)  org_x = xp; org_y = yp; //draw_box(org_x, org_y, org_x+320, org_y+200, 0)
#define fg_resize(w,h)  FG_SDL_init()
#define fg_reset()  FG_SDL_quit()
#define fg_print(fmt,n)  fg_sdl_print(fmt, n);
#define fg_getkey(k,a)  while(!(*k = *a = get_input())) sleep()
#define fg_rect(x1,x2,y1,y2)  draw_rect(x1,x2,y1,y2)
#define fg_mouseini()  1

#define fg_setmode(m)
#define fg_mousefin()
#define fg_mouselim(x1,x2,y1,y2)
#define fg_mousemov(x,y)

#define far

#undef main
#define int short

#endif // FASTGRAF_SDL_H
