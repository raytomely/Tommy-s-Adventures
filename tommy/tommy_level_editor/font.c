#include <SDL/SDL.h>
#include <SDL_image.h>
#include "font.h"

extern SDL_Surface *screen;
extern int graphics_x, graphics_y, org_x, org_y;
int read_keyboard(void);
void sleep(void);

SDL_Surface *font = NULL;
Uint8 bg_color = 128+4;
Uint8 fg_color = 0;
Uint8 font_alpha_color = 0;

void convert_surface(SDL_Surface **surface)
{
    SDL_Surface *temp_surf = &(**surface);
    *surface = SDL_DisplayFormat (temp_surf);
    SDL_FreeSurface(temp_surf);
}

Uint8 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    return *p;
}

void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x >= surface->w || y >= surface->h || x < 0 || y < 0)
        return;
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *p = pixel;
}

void blit_char(SDL_Rect *src, int x, int y)
{
    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8  *src_bitmap = font->pixels + src->y * font->pitch + src->x;

    for(y = 0; y < src->h; y++)
    {
        for(x = 0; x < src->w; x++)
        {
            if(src_bitmap[x] != font_alpha_color)
                dst_buffer[x] = fg_color;
            //else
                //dst_buffer[x] = bg_color;
        }
        dst_buffer += screen->w;
        src_bitmap += font->w;;
    }
}

void blit_char_scaled_X2(SDL_Rect *src, int x, int y)
{
    int x2;
    Uint8  *dst_buffer = screen->pixels + (y<<1) * screen->pitch + (x<<1);
    Uint8  *src_bitmap = font->pixels + src->y * font->pitch + src->x;

    for(y = 0; y < src->h; y++)
    {
        for(x = 0, x2= 0; x < src->w; x++, x2 += 2)
        {
            if(src_bitmap[x] != font_alpha_color)
            {
                dst_buffer[x2] = fg_color; dst_buffer[x2+1] = fg_color;
                (dst_buffer + screen->w)[x2] = fg_color; (dst_buffer + screen->w)[x2+1] = fg_color;
            }
        }
        dst_buffer += screen->w << 1;
        src_bitmap += font->w;;
    }
}

void blit_char_colored(SDL_Rect *src, int x, int y, Uint8 color)
{
    Uint8  *dst_buffer = screen->pixels + y * screen->pitch + x;
    Uint8  *src_bitmap = font->pixels + src->y * font->pitch + src->x;

    for(y = 0; y < src->h; y++)
    {
        for(x = 0; x < src->w; x++)
        {
            if(src_bitmap[x] != font_alpha_color)
                dst_buffer[x] = color;
            else
                dst_buffer[x] = src_bitmap[x];
        }
        dst_buffer += screen->w;
        src_bitmap += font->w;;
    }
}

void load_font(void)
{
    if(font != NULL)
        SDL_FreeSurface(font);
    font = SDL_LoadBMP("LN_EGA8x8.bmp");
    convert_surface(&font);
    SDL_SetColorKey(font, SDL_SRCCOLORKEY, getpixel(font, 0, 0));
    font_alpha_color = font->format->colorkey;
}

void free_font(void)
{
    SDL_FreeSurface(font);
}

void color_font(Uint32 color)
{
    Uint32 background_color = getpixel(font, 0, 0);
    int x, y;
    for(y = 0; y < font->h; y++)
    {
        for(x = 0; x < font->w; x++)
        {
            if(getpixel(font, x, y) != background_color)
            {
                setPixel(font, x, y, color);
            }
        }
    }
}

void print_char(SDL_Surface *surface, int x, int y, unsigned char c)
{
    //SDL_Rect image_pos = {x, y};
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};

    image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
    image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
    blit_char_scaled_X2(&image_size, x, y);
    //SDL_BlitSurface(font, &image_size, surface, &image_pos);
    //blit_colored(font, &image_size, surface, &image_pos, 0XFFFFFF);
    //blit_rotated(font, &image_size, surface, &image_pos, 30);
}

void print_string(SDL_Surface *surface, int x, int y, char *s)
{
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    for (i = 0; i < strlen(s); i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        blit_char(&image_size, x, y);
        x += CHAR_WIDTH;
    }
}

void print_char_colored(SDL_Surface *surface, int x, int y, unsigned char c, Uint8 color)
{
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};

    image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
    image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
    blit_char_colored(&image_size, x, y, color);
}

void print_string_colored(SDL_Surface *surface, int x, int y, char *s, Uint8 color)
{
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    for (i = 0; i < strlen(s); i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        image_size.w = CHAR_WIDTH;
        image_size.h = CHAR_HEIGHT;
        blit_char_colored(&image_size, x, y, color);
        x += CHAR_WIDTH;
    }
}

void fg_sdl_print(char *s, int n)
{
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    int len = strlen(s), x = graphics_x - org_x, y = graphics_y - CHAR_HEIGHT - org_y;
    //SDL_Rect rect = {graphics_x - 20, graphics_y - 15, (len*CHAR_WIDTH)+33, 21};
    //SDL_FillRect(screen, &rect, bg_color);
    for (i = 0; i < len && i < n; i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        blit_char(&image_size, x, y);
        //blit_char_scaled_X2(&image_size, x, y);
        x += CHAR_WIDTH;
    }
    //SDL_Flip(screen);
    //SDL_UpdateRect(screen, graphics_x - 20, graphics_y - 15, (len*CHAR_WIDTH)+33, 21);
    SDL_UpdateRect(screen, graphics_x, graphics_y-CHAR_HEIGHT, len*CHAR_WIDTH, CHAR_HEIGHT);
    //SDL_UpdateRect(screen, (graphics_x - org_x)<<1, (graphics_y-CHAR_HEIGHT-org_y)<<1, (len*CHAR_WIDTH)<<1, CHAR_HEIGHT<<1);
}

void fg_sdl_print_scaled_X2(char *s, int n)
{
    static SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    int len = strlen(s), x = graphics_x - org_x, y = graphics_y - CHAR_HEIGHT - org_y;
    //SDL_Rect rect = {graphics_x - 20, graphics_y - 15, (len*CHAR_WIDTH)+33, 21};
    //SDL_FillRect(screen, &rect, bg_color);
    for (i = 0; i < len && i < n; i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        //blit_char(&image_size, x, y);
        blit_char_scaled_X2(&image_size, x, y);
        x += CHAR_WIDTH;
    }
    //SDL_Flip(screen);
    //SDL_UpdateRect(screen, graphics_x - 20, graphics_y - 15, (len*CHAR_WIDTH)+33, 21);
    //SDL_UpdateRect(screen, graphics_x, graphics_y-CHAR_HEIGHT, len*CHAR_WIDTH, CHAR_HEIGHT);
    SDL_UpdateRect(screen, (graphics_x - org_x)<<1, (graphics_y-CHAR_HEIGHT-org_y)<<1, (len*CHAR_WIDTH)<<1, CHAR_HEIGHT<<1);
}

