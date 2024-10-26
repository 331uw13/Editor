#include <GL/glew.h>
#include <stdio.h>

#include "editor.h"
#include "draw.h"
#include "utils.h"
#include <math.h>

static void to_grid_coords(struct editor_t* ed,
        float* x, float* y,
        float* w, float* h) 
{
    *x = (float)col_to_loc(ed, *x);
    *y = (float)row_to_loc(ed, *y);
    *w *= cellw(ed);
    *h *= cellh(ed);
}


void set_color(struct editor_t* ed, float r, float g, float b) {
    glUseProgram(ed->shader);
    glUniform3f(ed->shader_color_uniloc, r, g, b);
}

void set_color_hex(struct editor_t* ed, unsigned int hex) {
    set_color(ed, UNHEX(hex));
    ed->drw_color_hex = hex;
}

void draw_rect(struct editor_t* ed, 
        float x, float y,
        float w, float h,
        int need_mapping, int use_grid) {

    if((w * h) <= 0) {
        return;
    }

    if(use_grid) {
        to_grid_coords(ed, &x, &y, &w, &h);
    }

    if(need_mapping) {
        map_xywh(ed, &x, &y, &w, &h);
    }

    float vertices[] = {
        x,    y-h,
        x,    y,
        x+w,  y,

        x,    y-h,
        x+w,  y,
        x+w,  y-h
    };

    glBindVertexArray(ed->vao);
    glUseProgram(ed->shader);

    glBindBuffer(GL_ARRAY_BUFFER, ed->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void draw_framed_rect(struct editor_t* ed, 
        float x, float y, float w, float h,
        unsigned int frame_color, 
        float fthickness,
        int need_mapping, int use_grid)
{

    if((w * h) <= 0) {
        return;
    }


    if(use_grid) {
        to_grid_coords(ed, &x, &y, &w, &h);
        use_grid = 0;

    }

    if(fthickness > 0.9f) {
        x -= fthickness / 2;
        w += fthickness;

        y -= fthickness / 2;
        h += fthickness;
    }

    if(need_mapping) {
        map_xywh(ed, &x, &y, &w, &h);
        need_mapping = ALREADY_MAPPED;
    }

    draw_rect(ed, x, y, w, h, need_mapping, use_grid);
    glBindVertexArray(ed->vao);
    glUseProgram(ed->shader);
    glBindBuffer(GL_ARRAY_BUFFER, ed->vbo);


    unsigned int old_color = ed->drw_color_hex;

    // draw the frame.
    //
    glUniform3f(ed->shader_color_uniloc, UNHEX(frame_color));
    glLineWidth(fthickness);

    float f_vertices[] = {
        x,    y,
        x+w,  y,
        x+w,  y-h,
        x,    y-h,
        x,    y-h,
        x,    y
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f_vertices), f_vertices);
    glDrawArrays(GL_LINE_STRIP, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    glUniform3f(ed->shader_color_uniloc, UNHEX(old_color));
}


void draw_char(struct editor_t* ed, int x, int y, unsigned char c, int use_grid) {
    if(!char_ok(c)) {
        return;
    }
    struct glyph_t* g = &ed->font.glyphs[c - 0x20];

    if(!g) {
        return;
    }

    if(use_grid) {
        x = col_to_loc(ed, x);
        y = row_to_loc(ed, y);
    }

    y += ed->font.char_h;

    float xp = x + g->bearing_x * ed->font.scale;
    float yp = y + (g->height - g->bearing_y) * ed->font.scale;

    float w = (g->width * ed->font.scale);
    float h = (g->height * ed->font.scale);

    map_xywh(ed, &xp, &yp, &w, &h);


    float vertices[] = {
        
        xp,   yp+h,   0.0, 0.0,
        xp,   yp,     0.0, 1.0,
        xp+w, yp,     1.0, 1.0,

        xp,   yp+h,  0.0, 0.0,
        xp+w, yp,    1.0, 1.0,
        xp+w, yp+h,  1.0, 0.0

    };

    // TODO: optimize... yeah later

    glBindVertexArray(ed->font.vao);

    glUseProgram(ed->font.shader);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, g->texture);
    glBindBuffer(GL_ARRAY_BUFFER, ed->font.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void _draw_data_r(
        struct editor_t* ed,
        int x,
        int y,
        char* data,
        long int size,
        int use_grid,
        int use_wrapping,
        int max_x
        )
{
    if(!data) {
        return;
    }

    // search for null character?
    int seek_nc = 0;
    if((seek_nc = (size < 0) ? 1 : 0)) {
        size = STRING_MAX_SIZE;
    }

    const int xinc = use_grid ? 1 : ed->font.char_w;
    const int yinc = use_grid ? 1 : ed->font.char_h;
    const int xorigin = x;


    for(long int i = 0; i < size; i++) {
        int old_color = -1;
        char c = data[i];

        switch(c) {
            
            case 0:
                return;
            
            case 0x9:
                x += xinc * FONT_TAB_WIDTH;
                continue;

            case 0xA:
                continue;

            default:
                if(!char_ok(c)) {
                    //printf("unknown character: 0x%x/'%c'\n", c, c);
                    old_color = ed->font.color_hex;
                    c = '?';
                }
                break;
        }

        if(old_color > 0) {
            font_set_color_hex(&ed->font, 0xFF3020);
        }

        draw_char(ed, x, y, c, use_grid);
        x += xinc;

        if(use_wrapping && (x >= max_x)) {
            x = xorigin;
            y += yinc;
        }

        if(old_color > 0) {
            font_set_color_hex(&ed->font, old_color);
        }
    }
}

void draw_data(struct editor_t* ed, int x, int y, char* data, long int size, int use_grid) {
    _draw_data_r(ed, x, y, data, size, use_grid, 0, 0);
}

void draw_data_wrapped(struct editor_t* ed,
            int x, int y,
            char* data,
            long int size,
            int max_x,
            int use_grid) {
    _draw_data_r(ed, x, y, data, size, use_grid, 1, max_x);
}

#define FRAME_COLOR_A 0x00AAEE
#define FRAME_COLOR_B 0x003344
#define FRAME_COLOR_BG 0x101111
#define FILENAME_COLOR_A 0x70A0A0
#define FILENAME_COLOR_B 0x203030
#define BAR_COLOR_A 0x102020
#define BAR_COLOR_B 0x081515

#define CURSOR_COLOR_A 0x10EE10
#define CURSOR_COLOR_B 0x104410

#define DATA_COLOR 0xFFEEAA
#define LINENUM_COLOR_A 0x406040
#define LINENUM_COLOR_B 0x252525


static void draw_buffer_frame(struct editor_t* ed, struct buffer_t* buf, int is_current) {
    if(!buffer_ready(buf)) {
        return;
    }

    set_color_hex(ed, FRAME_COLOR_BG);

    draw_rect(ed, 
            buf->x,
            buf->y,
            buf->width,
            buf->height,
            MAP_XYWH, DRW_NO_GRID);

    // filename stuff
    //
    set_color_hex(ed, is_current ? BAR_COLOR_A : BAR_COLOR_B);
    draw_rect(ed,
            buf->x,
            buf->y + buf->height,
            buf->width,
            CELLH,
            MAP_XYWH, DRW_NO_GRID);

    font_set_color_hex(&ed->font, is_current ? FILENAME_COLOR_A : FILENAME_COLOR_B);
    draw_data(ed,
            buf->x + 10,
            buf->y + buf->height,
            buf->filename,
            buf->filename_size,
            DRW_NO_GRID);
}


static void draw_cursor(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    long int x = buf->cursor_x
            + buf->content_xoff;
    x *= cw;
    x += buf->x;

    long int y = buf->cursor_y + buf->y - buf->scroll;

    y *= ch;

    // cursor background
    //
    set_color_hex(ed, CURSOR_COLOR_B);
    draw_rect(ed, x, y, cw, ch, MAP_XYWH, DRW_NO_GRID);


    set_color_hex(ed, CURSOR_COLOR_A);
    
    // cursor line below
    //
    draw_rect(ed, 
            x,
            y + ch-1,
            cw,
            2,
            MAP_XYWH, DRW_NO_GRID);

    // cursor line right side
    //
    draw_rect(ed,
            x + cw - 1,
            y + ch/2 - 2,
            2,
            ch/2 + 2,
            MAP_XYWH, DRW_NO_GRID);
}

static void draw_buffer(struct editor_t* ed, struct buffer_t* buf, char* linenum_buf) {
    if(!buffer_ready(buf)) {
        return;
    }


    struct string_t* line = NULL;

    const int xdrw_off = buf->content_xoff;

    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i >= buf->max_row + buf->scroll) {
            break;
        }

        line = buf->lines[i];
        if(!string_ready(line)) {
            fprintf(stderr, "corrupted data?? buffer id:%i\n", buf->id);
            return;
        }

        const int cw = CELLW;
        const int ch = CELLH;

        int linenum_len = snprintf(linenum_buf, LINENUM_BUF_SIZE, "%li", i);

        int x = buf->x;
        int y = i - buf->scroll;
        int ln_x = x + cw * (xdrw_off - linenum_len - 2);

        y *= ch;
        y += buf->y;
        x += cw * xdrw_off;

        font_set_color_hex(&ed->font, DATA_COLOR);
        draw_data(ed, x, y, line->data, line->data_size, DRW_NO_GRID);


        // line numbers
        //
        font_set_color_hex(&ed->font,
                (i == buf->cursor_y) ? LINENUM_COLOR_A : LINENUM_COLOR_B);

        draw_data(ed, ln_x, y, linenum_buf, LINENUM_BUF_SIZE, DRW_NO_GRID);

    }
}

static void draw_selected(struct editor_t* ed, struct buffer_t* buf) {
    if((ed->mode != MODE_SELECT) || (buf->id != ed->current_buf_id)) {
        return;
    }

}

void draw_buffers(struct editor_t* ed) {
    
    char linenum_buf[LINENUM_BUF_SIZE];

    int num_bufs = iclamp(ed->num_active_buffers, 0, MAX_BUFFERS);
    struct buffer_t* buf = NULL;
    for(int i = 0; i < num_bufs; i++) {
        buf = &ed->buffers[i];
        if(!buffer_ready(buf)) {
            write_message(ed, ERROR_MSG, "Uninitialized buffer!\0");
            fprintf(stderr, "buffer %i, uninitialized.\n");
            continue;
        }

        draw_buffer_frame(ed, buf, i == ed->current_buf_id);
        draw_cursor(ed, buf);
        draw_buffer(ed, buf, linenum_buf);


    }

}


