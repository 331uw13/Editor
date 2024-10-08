#include <stdio.h>
#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "editor.h"
#include "font.h"
#include "utils.h"


int load_font_from_file(const char* file_path, struct font_t* font) {
    int res = 0;

    if(font->ready) {
        fprintf(stderr, "font is already loaded. ?\n");
        goto error;
    }

    FT_Library ft;
    
    if(FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Failed to initialize freetype library.\n");
        goto error;
    }


    FT_Face face;
    
    if(FT_New_Face(ft, file_path, 0, &face)) {
        fprintf(stderr, "Failed to load font from '%s'.\n", file_path);
        goto error;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font->max_bitmap_w = 0;
    font->max_bitmap_h = 0;

    for(unsigned char c = 0x20; c < FONT_NUM_CHARS; c++) {
        
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "warning: FT_Load_Char failed '%c'\n font_path: '%s'\n", 
                    c, file_path);
            continue;
        }

        int bitmap_width = face->glyph->bitmap.width;
        int bitmap_height = face->glyph->bitmap.rows;

        if(bitmap_width > font->max_bitmap_w) {
            font->max_bitmap_w = bitmap_width;
        }
        if(bitmap_height > font->max_bitmap_h) {
            font->max_bitmap_h = bitmap_height;
        }

        unsigned int tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D,
                0, GL_RED,
                bitmap_width,
                bitmap_height,
                0, GL_RED, GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        font->glyphs[c] = (struct glyph_t) {
            .texture = tex,
            .advance = face->glyph->advance.x,
            .width = bitmap_width,
            .height = bitmap_height,
            .bearing_x = face->glyph->bitmap_left,//face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top,
            .test = face->glyph->metrics.width
        };

        /*
        if(c >= 0x20) {
            printf(" '%c' | '%i'\n", c, tex);
        }
        */
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    font_set_scale(font, 0.5);
    res = 1;

error:
    return res;
}

int unload_font(struct font_t* font) {
    int res = 0;

    if(font) {
        for(int i = 0; i < FONT_NUM_CHARS; i++) {
            if(font->glyphs[i].texture > 0) {
                glDeleteTextures(1, &font->glyphs[i].texture);
                font->glyphs[i].texture = 0;
            }
            
        }

        res = 1;
    }

    printf(" unloaded font.\n");

    return res;
}

void font_set_scale(struct font_t* font, float scale) {
    if(scale <= 0.0) {
        return;
    }

    font->scale = scale;
    font->char_w = font->max_bitmap_w * scale;
    font->char_h = font->max_bitmap_h * scale;
    
}


void draw_char(struct editor_t* ed, int x, int y, unsigned char c, int use_grid) {
    if((c < 0x20) || (c > 0x7F)) {
        return;
    }

    struct glyph_t* g = &ed->font.glyphs[c];

    float xp = x + g->bearing_x * ed->font.scale;
    float yp = y + (g->height - g->bearing_y) * ed->font.scale;

    float w = (g->width * ed->font.scale);
    float h = (g->height * ed->font.scale) * 2;

    map_xywh(ed, &xp, &yp, &w, &h);


    float vertices[] = {
        
        xp, yp+h,   0.0, 0.0,
        xp, yp,     0.0, 1.0,
        xp+w, yp,   1.0, 1.0,

        xp,  yp+h,  0.0, 0.0,
        xp+w, yp,   1.0, 1.0,
        xp+w, yp+h,  1.0, 0.0

    };

    // TODO: optimize.

    glUseProgram(ed->shader);
    glActiveTexture(GL_TEXTURE0);


    glBindTexture(GL_TEXTURE_2D, g->texture);
    glBindBuffer(GL_ARRAY_BUFFER, ed->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_data(struct editor_t* ed, int x, int y, char* data, size_t size, int use_grid) {

    if(!data || size == 0) {
        return;
    }

    const int x_inc = use_grid ? 1 : ed->font.char_w;

    for(size_t i = 0; i < size; i++) {
        char c = data[i];

        switch(c) {
            case 0x9:
                x += x_inc * FONT_TAB_WIDTH;
                break;

            case 0:
                return;
        }

        draw_char(ed, x, y, c, use_grid);

        x += x_inc;
    }
}










