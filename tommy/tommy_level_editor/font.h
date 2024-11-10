
#define CHAR_COLUMNS 16
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define CHAR_OFFSET 0

void load_font(void);
void free_font(void);
void color_font(Uint32 color);
void print_char(SDL_Surface *surface, int x, int y, unsigned char c);
void print_string(SDL_Surface *surface, int x, int y, char *s);
void print_char_colored(SDL_Surface *surface, int x, int y, unsigned char c, Uint8 color);
void print_string_colored(SDL_Surface *surface, int x, int y, char *s, Uint8 color);
void fg_sdl_print(char *s, int n);
void fg_sdl_print_scaled_X2(char *s, int n);

