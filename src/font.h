#ifndef FONT_H
#define FONT_H

#define FONT_NUM_CHARS 127
#define FONT_TAB_WIDTH 4

struct glyph_t {
    unsigned int texture; // texture id opengl can use
    unsigned int advance; // horizontal distance to next glyph
    int width;
    int height;

    int bearing_x; // offset from baseline to top of the glyph.
    int bearing_y; // 

    int test;
};

struct font_t {
    struct glyph_t glyphs[FONT_NUM_CHARS];
    
    float scale;

    int max_bitmap_w;
    int max_bitmap_h; 

    // character width and height (in scale)
    int char_w; 
    int char_h;
   
    int ready; // NOTE: this should be set to 0 before loading the font.
};


// ------
//
// TODO: we dont need the non characters
// 
// ---------


struct editor_t;

int     load_font_from_file(const char* file_path, struct font_t* font);
int     unload_font(struct font_t* font);
void    font_set_scale(struct font_t* font, float scale);

#define DRW_NO_GRID 0
#define DRW_ONGRID  1
void    draw_char(struct editor_t* ed, int x, int y, unsigned char c, int use_grid);
void    draw_data(struct editor_t* ed, int x, int y, char* data, size_t size, int use_grid);


#endif
