#include <SDL/SDL.h>
#include "keys.h"
#include "font.h"

int main_loop = 1;
SDL_Event event;
SDL_Surface *screen = NULL;
unsigned char alpha_color = 0;
//unsigned char *frame_buffer;
int fb_width = 352;
int fb_height = 727;
int graphics_x = 0;
int graphics_y = 0;
int org_x = 0;
int org_y = 0;
unsigned char current_color = 0;
unsigned char palette[768];
SDL_Color FGPalette[256];
SDL_Rect screen_rect = {0, 0, 320, 200};
int fps = 16;

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
        src_bitmap += screen->w;;
    }
    //SDL_FillRect(screen, NULL, 255);
    //SDL_Flip(screen);
    SDL_UpdateRect(screen, xr, yr, x2, y2);
}

extern short nrows, ncols;
extern unsigned char level_map[240][200];

void print_level(void)
{
    int x,y;
    for(y = 0; y < nrows; y++)
        for(x = 0; x < ncols; x++)
            printf("x=%d y=%d v=%d \n",x,y,level_map[x][y]);
}

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

#define ESC         27
#define SPACE       32
#define UP_ARROW    72
#define LEFT_ARROW  75
#define RIGHT_ARROW 77
#define DOWN_ARROW  80
#define INSERT      82
#define DELETE      83

int fg_keys[SDLK_LAST];

void intit_fg_keys(void)
{
    memset(fg_keys, 0, sizeof(fg_keys));

    fg_keys[SDLK_ESCAPE] = ESC;
    fg_keys[SDLK_SPACE] = SPACE;
    fg_keys[SDLK_UP] = UP_ARROW;
    fg_keys[SDLK_LEFT] = LEFT_ARROW;
    fg_keys[SDLK_RIGHT] = RIGHT_ARROW;
    fg_keys[SDLK_DOWN] = DOWN_ARROW;
    fg_keys[SDLK_INSERT] = INSERT;
    fg_keys[SDLK_DELETE] = DELETE;
}

int read_keyboard(void)
{
    if(SDL_PollEvent(&event) == 1)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                return ESC;
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

void FG_SDL_init(void)
{
   SDL_Init(SDL_INIT_VIDEO);
   SDL_WM_SetCaption("Tommy Level Editor",NULL);
   screen = SDL_SetVideoMode(352, 727, 8, SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF);
   //frame_buffer = malloc(352*727);
   //setSFRPalette();
   intit_fg_keys();
   SDL_EnableKeyRepeat(180, 100);
   load_font();
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
   free_font();
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
        SDL_Delay(fps - (actual_time - old_time));
        old_time = SDL_GetTicks();
    }
    else
    {
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





