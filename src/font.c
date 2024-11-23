#include <stdio.h>
#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "editor.h"
#include "font.h"
#include "utils.h"
#include "shader.h"


int load_font(const char* file_path, struct font_t* font, const char* vert_shader, const char* frag_shader) {
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
        FT_Done_FreeType(ft);
        goto error;
    }

    // ok good, now start settings stuff up.


    FT_Set_Pixel_Sizes(face, 0, 32);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font->max_bitmap_w = 0;
    font->max_bitmap_h = 0;
    font->shader = 0;
    font->vbo = 0;
    font->vao = 0;
    font->shader_color_uniloc = -1;

    font->shader = create_shader_program(vert_shader, frag_shader);
    if(!font->shader) {
        goto error_and_done;
    }

    glGenVertexArrays(1, &font->vao);
    glBindVertexArray(font->vao);

    glGenBuffers(1, &font->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    const size_t stride_size = 4 * sizeof(float);

    // positions
    //
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, 0);
    glEnableVertexAttribArray(0);

    // texture coordinates
    //
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride_size, (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    font->shader_color_uniloc = 
        glGetUniformLocation(font->shader, "font_color");

    if(font->shader_color_uniloc < 0) {
        fprintf(stderr, "\033[33m'load_font'(warning): uniform location not found.\033[0m\n");
    }
    
    for(unsigned char c = 0x20; c < 0x7F; c++) {
        
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

        font->glyphs[c - 0x20] = (struct glyph_t) {
            .texture = tex,
            .width = bitmap_width,
            .height = bitmap_height,
            .bearing_x = face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top,
            .test = face->glyph->metrics.width
        };
    }

    font_set_color(font, 1.0, 1.0, 1.0);

error_and_done:

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    font_set_scale(font, 0.45);
    res = 1;

    font->ready = 1;
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

        glDeleteProgram(font->shader);
        glDeleteBuffers(1, &font->vbo);
        glDeleteVertexArrays(1, &font->vao);

        font->ready = 0;
        res = 1;
    
        printf(" unloaded font.\n");
    }

    return res;
}

void font_set_scale(struct font_t* font, float scale) {
    if(scale <= FONT_MIN_SCALE) {
        return;
    }
    if(scale >= FONT_MAX_SCALE) {
        return;
    }

    font->scale = scale;
    font->char_w = font->max_bitmap_w * scale;
    font->char_h = font->max_bitmap_h * scale;  
}

void font_set_color(struct font_t* font, float r, float g, float b) {
    glUseProgram(font->shader);
    glUniform3f(font->shader_color_uniloc, r, g, b);
}

void font_set_color_hex(struct font_t* font, unsigned int hex) {
    font_set_color(font, UNHEX(hex));
    font->color_hex = hex;
}


