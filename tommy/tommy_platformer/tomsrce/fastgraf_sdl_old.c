#include <SDL/SDL.h>
//#include "font.h"

int main_loop = 1;
SDL_Event event;
SDL_Surface *screen = NULL;
Uint8 *keystate;
unsigned char alpha_color = 0;
unsigned char transcolors = 0;
//unsigned char *frame_buffer;
int fb_width = 352;
int fb_height = 744;
int graphics_x = 0;
int graphics_y = 0;
int org_x = 0;
int org_y = 0;
unsigned char current_color = 0;
unsigned char palette[768];
SDL_Color FGPalette[256];
SDL_Rect screen_rect = {0, 0, 320, 200};
int clip_x1, clip_x2, clip_y1, clip_y2;
int fps = 1000 / 18.2; // DOS original clock

void setFGPalette(void)
{
   int i, j;
   for (i=0, j=0 ; i < 256 ; i++, j+=3)
   {
       FGPalette[i].r = palette[j];
       FGPalette[i].g = palette[j+1];
       FGPalette[i].b = palette[j+2];
   }
   SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, FGPalette, 0, 256);
}

void replace_palette_entries(int start, int count, char *values)
{
   int i, j;
   for (i=start, j=0 ; i < count ; i++, j+=3)
   {
       FGPalette[i].r = palette[j];
       FGPalette[i].g = palette[j+1];
       FGPalette[i].b = palette[j+2];
   }
   SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, FGPalette, 0, 256);
}

struct pcx_header
{
       char   header;
       char   version;
       char   encode;
       char   bitperpixel;
       short  x1,y1,x2,y2;
       short  hres,vres;
       char   palette[48];
       char   mode;
       char   nplanes;
       short  byteperline;
       short  palletteinfo;
       short  scanerhres,scanervres;
       char   temp[54];
} header;

void read_pcx( const char *name )
{
    FILE *fp;
    int i, x = 0, y = graphics_y, mx, my;
    unsigned char ch, data;

    if ( (fp = fopen( name, "rb" )) != NULL )
    {
        fread( &header, sizeof(header), 1, fp );

        fseek( fp, -768, SEEK_END );
        fread( palette, 768, 1, fp );
        //for ( i = 0; i < 768; i++ ) palette[i] >>= 2;
        //set_palette( palette );
        setFGPalette();
        fseek( fp, sizeof(header), SEEK_SET );
        mx = header.x2;
        if ( mx > fb_width-1 ) mx = fb_width;
        my = header.y2;
        if ( my > fb_height-1 ) my = fb_height;
        while(1)
        {
            ch = fgetc( fp );

            if ( (ch & 0xC0) == 0xC0 )
            {
                data = getc( fp );
                for ( i = 0; i < (ch & 0x3F); i++ )
                ((Uint8*)screen->pixels)[ (y * fb_width) + x++ ] = data;
            }
            else ((Uint8*)screen->pixels)[ (y * fb_width) + x++ ] = ch;

            if ( x > mx ) { x = 0; y += 1; }
            if ( y - graphics_y > my ) break;
        }
    }
    fclose( fp );SDL_Flip(screen);
}

void blit_raw(int x1, int x2, int y1, int y2, int x, int y)
{
    static int xr, yr;
    xr = x;
    yr = (y - (y2 - y1));
    Uint8  *dst_buffer = screen->pixels + yr * screen->pitch + xr;
    Uint8  *src_bitmap = screen->pixels + y1 * screen->pitch + x1;
    x2 = x2 - x1 + 1; y2 = y2 - y1 + 1;

    for(y = 0; y < y2; y++)
    {
        for(x = 0; x < x2; x++)
        {
            //if(src_bitmap[x] != alpha_color)
                dst_buffer[x] = src_bitmap[x];
        }
        dst_buffer += screen->w;
        src_bitmap += screen->w;
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, xr, yr, x2, y2);
}

#define not_transcolor(c) ((c + 1) ^ transcolors)

void blit_raw_alpha(int x1, int x2, int y1, int y2, int x, int y)
{
    static int xr, yr;
    xr = x;
    yr = (y - (y2 - y1));
    Uint8  *dst_buffer = screen->pixels + yr * screen->pitch + xr;
    Uint8  *src_bitmap = screen->pixels + y1 * screen->pitch + x1;
    x2 = x2 - x1 + 1; y2 = y2 - y1 + 1;

    for(y = 0; y < y2; y++)
    {
        for(x = 0; x < x2; x++)
        {
            if(not_transcolor(src_bitmap[x]))
                dst_buffer[x] = src_bitmap[x];
        }
        dst_buffer += screen->w;
        src_bitmap += screen->w;
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, xr, yr, x2, y2);
}

//extern short nrows, ncols;
//extern unsigned char level_map[240][200];

/*void print_level(void)
{
    int x,y;
    for(y = 0; y < nrows; y++)
        for(x = 0; x < ncols; x++)
            printf("x=%d y=%d v=%d \n",x,y,level_map[x][y]);
}*/

#define SWAP(a,b)  {a^=b;b^=a;a^=b;}

void line_X( int xl, int xh, int y, char col )
{
    //int temp;

    if ( xl > xh ) SWAP( xl, xh );

    memset( (Uint8*)screen->pixels+xl+fb_width*y, col, (xh-xl)+1 );
}

void line_Y( int x, int yl, int yh, char col )
{
    int i; //,temp;

    if ( yl > yh ) SWAP( yl, yh );

    for( i = 0; i < (yh - yl + 1); i++ )
    {
        *((Uint8*)screen->pixels + x + (i + yl) * fb_width ) = col;
    }
}

void draw_box( int x1, int y1, int x2, int y2, char col )
{
    if ( x1 > x2 ) SWAP( x1, x2 );
    if ( y1 > y2 ) SWAP( y1, y2 );

    line_X( x1, x2, y1, col );
    line_X( x1, x2, y2, col );

    line_Y( x1, y1+1, y2-1, col );
    line_Y( x2, y1+1, y2-1, col );
}

void line_Xx( int xl, int xh, int y, char col )
{
    int i; //,temp;

    if ( xl > xh ) SWAP( xl, xh );

    for( i = 0; i < (xh - xl + 1); i++ )
    {
        *((Uint8*)screen->pixels + (xl + i) + y * fb_width ) ^= col;
    }
}

void line_Yx( int x, int yl, int yh, char col )
{
    int i; //,temp;

    if ( yl > yh ) SWAP( yl, yh );

    for( i = 0; i < (yh - yl + 1); i++ )
    {
        *((Uint8*)screen->pixels + x + (i + yl) * fb_width ) ^= col;
    }
}

void draw_boxx( int x1, int y1, int x2, int y2, char col )
{
    if ( x1 > x2 ) SWAP( x1, x2 );
    if ( y1 > y2 ) SWAP( y1, y2 );

    line_Xx( x1, x2, y1, col );
    line_Xx( x1, x2, y2, col );

    line_Yx( x1, y1+1, y2-1, col );
    line_Yx( x2, y1+1, y2-1, col );
    SDL_UpdateRect(screen, x1, y1, x2-x1+1, y2-y1+1);
}

void draw_rect(int x1, int x2, int y1, int y2)
{
    SDL_Rect rect = {x1, y1, x2-x1+1, y2-y1+1};
    SDL_FillRect(screen, &rect, current_color);
    SDL_UpdateRect(screen, x1, y1, x2-x1+1, y2-y1+1);
}

void display_run_image(char *map_array, int runs, int width)
{
    int x, y, i, j;
    x = graphics_x;
    y = graphics_y;

    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8 color, run;
    x = 0; y = 0; runs <<= 1;

    for(i = 0; i < runs; i += 2)
    {
        color = map_array[i];
        run = map_array[i+1];
        for(j = 0; j < run; j++)
        {
            dst_buffer[x] = color;
            x++;
            if(x >= width)
            {
                dst_buffer -= screen->w;
                x = 0; y++;
            }
        }
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, graphics_x, graphics_y - y, width, y+1);
}

void draw_bitmap(char *map_array, int width, int height)
{
    int x, y, i, j;
    x = graphics_x;
    y = graphics_y;

    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    int length = width * height;
    char byte;
    x = 0; y = 0;

    for(i = 0; i < length; i ++)
    {
        byte = map_array[i];
        if(i >= width)
        {
            dst_buffer -= screen->w;
            x = 0; y++;
        }
        for(j = 7; j > -1; j--)
        {
            if((byte & (1 << j)) != 0)
                dst_buffer[x] = current_color;
            x++;
        }
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, graphics_x, graphics_y - height, width << 3, height+1);
}


void draw_bitmap_raw(char *map_array, int width, int height)
{
    int x, y;
    int src_x = 0, src_y = 0, src_pitch = width;
    x = graphics_x;
    y = graphics_y;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((x >= clip_x2) || ((y - height) >= clip_y2) ||
       ((x + width) <= clip_x1) || (y  <= clip_y1))
            return;

    // clip rectangles
    // upper left hand corner first
    if (x  < clip_x1)
    {
        src_x = clip_x1 - x;
    }

    if (y - height < clip_y1)
    {
        height = y - clip_y1;
    }

    // now lower left hand corner
    if (x + width > clip_x2)
    {
        width = clip_x2 - x;
    }

    if (y > clip_y2)
    {
        src_y = y - clip_y2;
        y -= src_y;
    }

    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8  *src_bitmap = (Uint8*)map_array + src_y * src_pitch;

    for(y = src_y; y < height; y++)
    {
        for(x = src_x; x < width; x++)
        {
            if(src_bitmap[x] != alpha_color)
                dst_buffer[x] = src_bitmap[x];
        }
        dst_buffer -= screen->w;
        src_bitmap += src_pitch;
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, graphics_x, graphics_y - height, width, height+1);
}

void draw_bitmap_raw_flipped(char *map_array, int width, int height)
{
    int x, y, x2;
    int src_x = 0, src_y = 0, src_pitch = width;
    x = graphics_x;
    y = graphics_y;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((x >= clip_x2) || ((y - height) >= clip_y2) ||
       ((x + width) <= clip_x1) || (y  <= clip_y1))
            return;

    // clip rectangles
    // upper left hand corner first
    if (x < clip_x1)
    {
        src_x = clip_x1 - x;
    }

    if (y - height < clip_y1)
    {
        height = y - clip_y1;
    }

    // now lower left hand corner
    if (x + width > clip_x2)
    {
        width = clip_x2 - x;
    }

    if (y > clip_y2)
    {
        src_y = y - clip_y2;
        y -= src_y;
    }

    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8  *src_bitmap = (Uint8*)map_array + src_y * src_pitch;// + (src_pitch - width);

    for(y = src_y; y < height; y++)
    {
        for(x = src_x, x2 = src_pitch-src_x-1; x < width; x++, x2--)
        {
            if(src_bitmap[x2] != alpha_color)
                dst_buffer[x] = src_bitmap[x2];
        }
        dst_buffer -= screen->w;
        src_bitmap += src_pitch;
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, graphics_x, graphics_y - height, width, height+1);
}

void getimage(char *map_array, int width, int height)
{
    int x, y;
    x = graphics_x;
    y = graphics_y;

    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8  *src_bitmap = (Uint8*)map_array;

    x = 0; y = 0;

    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            //if(src_bitmap[x] != alpha_color)
                src_bitmap[x] = dst_buffer[x];
        }
        dst_buffer -= screen->w;
        src_bitmap += width;
    }
}

#define KB_ALT      56             /* low-level keyboard scan codes */
#define KB_CTRL     29
#define KB_ESC       1
#define KB_SPACE    57
#define KB_UP       72
#define KB_LEFT     75
#define KB_RIGHT    77
#define KB_DOWN     80
#define KB_F1       59
#define KB_F2       60
#define KB_W        17
#define KB_D        32

int fg_keys[SDLK_LAST];

void intit_fg_keys(void)
{
    memset(fg_keys, 0, sizeof(fg_keys));

    fg_keys[SDLK_LALT] = KB_ALT;
    fg_keys[SDLK_LCTRL] = KB_CTRL;
    fg_keys[SDLK_ESCAPE] = KB_ESC;
    fg_keys[SDLK_SPACE] = KB_SPACE;
    fg_keys[SDLK_UP] = KB_UP;
    fg_keys[SDLK_LEFT] = KB_LEFT;
    fg_keys[SDLK_RIGHT] = KB_RIGHT;
    fg_keys[SDLK_DOWN] = KB_DOWN;
    fg_keys[SDLK_F1] = KB_F1;
    fg_keys[SDLK_F2] = KB_F2;
    fg_keys[SDLK_w] = KB_W;
    fg_keys[SDLK_d] = KB_D;
}

int read_keyboard(void)
{
    if(SDL_PollEvent(&event) == 1)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                return KB_ESC;
                break;
            case SDL_KEYDOWN:
                return fg_keys[event.key.keysym.sym];
            case SDL_KEYUP:
                return fg_keys[event.key.keysym.sym]+128;
        }
    }
    return 0;
}

int get_input(void)
{
    if(SDL_PollEvent(&event) == 1)
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
                return event.key.keysym.sym;
        }
    }
    return 0;
}

void read_mouse(short *x, short *y, short *button)
{
    static int temp_x, temp_y;
    *button = SDL_GetMouseState(&temp_x, &temp_y);
    *x = temp_x; *y = temp_y;
    if(*button & SDL_BUTTON(1)) *button = 1;
    else if(*button & SDL_BUTTON(3)) *button = 2;
    //if(*y > org_y+224) *y = org_y+224;
    if(*y > org_y+200) *y = org_y+200;
    else if(*y < org_y) *y = org_y;
    if(*x > org_x+305) *x = org_x+305;
    else if(*x < org_x) *x = org_x;

}

extern unsigned long game_time;
SDL_TimerID timer; /* Variable to store the timer's number */

/* Xallback Function(will be called every 7 ms) */
Uint32 increment_game_timer(Uint32 interval, void *param)
{
    game_time++;
    return interval;
}

void FG_SDL_init(void)
{
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
   SDL_WM_SetCaption("Tommy",NULL);
   screen = SDL_SetVideoMode(352, 744, 8, SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF);
   //frame_buffer = malloc(352*727);
   //setSFRPalette();
   intit_fg_keys();
   //SDL_EnableKeyRepeat(180, 100);
   freopen("stdout_sdl.txt", "w", stdout); // redirects stdout
   freopen("stderr_sdl.txt", "w", stderr); // redirects stderr
   freopen("CON", "r", stdin);
   timer = SDL_AddTimer(1000/(18.2*8), increment_game_timer, NULL); /* start timer */
   //load_font();
   //Mix_Init(0);
   //Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);
}

void FG_SDL_quit(void)
{
   /*Mix_HaltChannel(-1);
   Mix_HaltMusic();
   Mix_FreeMusic(game_music);
   Mix_CloseAudio();
   Mix_Quit();*/
   //free(frame_buffer);
   //free_font();
   SDL_RemoveTimer(timer); /* stop timer */
   SDL_Quit();
   printf("Bye! from Tommy SDL.\n");
   printf("Thank to Raytomely for this port.\n");
}
void processInput(void)
{
    if(SDL_PollEvent(&event) == 1)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                main_loop = 0;
                break;
            /*case SDL_KEYDOWN:
                 switch(event.key.keysym.sym)
                {
                    case SDLK_1:
                        printf("1\n");
                        break;
                }*/
        }
    }
}

void sleep(void)
{
    static int old_time = 0,  actual_time = 0;
    actual_time = SDL_GetTicks();
    if (actual_time - old_time < fps) // if less than fps ms has passed
    {
        //game_time += (fps - (actual_time - old_time)) / 7;
        SDL_Delay(fps - (actual_time - old_time));
        old_time = SDL_GetTicks();
    }
    else
    {
        //game_time += (actual_time - old_time) / 7;
        old_time = actual_time;
    }
}

void init_clock(int speed)
{
    if(speed > 0)
        fps = 16 + (speed * 10);
}

void main_event_loop(void)
{
    main_loop = 1;
    while(main_loop)
    {
        processInput();
        sleep();
    }
}

void set_rate(int rate){};


