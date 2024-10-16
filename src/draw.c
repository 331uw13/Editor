#include <GL/glew.h>
#include <stdio.h>

#include "editor.h"
#include "draw.h"
#include "utils.h"


static void to_grid_coords(struct editor_t* ed,
        float* x, float* y,
        float* w, float* h) 
{
    *x = (float)col_to_loc(ed, *x)-3.0;
    *y = (float)row_to_loc(ed, *y)-3.0;
    *w *= cellw(ed)+1.0;
    *h *= cellh(ed)+1.0;
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
        char c = data[i];

        switch(c) {
            case 0x9:
                x += xinc * FONT_TAB_WIDTH;
                continue;

            case 0:
                return;

            default:
                if(!char_ok(c)) {
                    continue;
                }
                break;
        }

        draw_char(ed, x, y, c, use_grid);
        x += xinc;

        if(use_wrapping && (x >= max_x)) {
            x = xorigin;
            y += yinc;
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

static void _draw_buffer_frame(struct editor_t* ed, struct buffer_t* buf) {
    
    const unsigned int 
        frame_color = 0x00AAEE;
    
    const unsigned int 
        frame_name_color = 0x00AAAA;


    int has_file = buf->file_opened && (buf->filename_size > 0);


    set_color_hex(ed, 0x101010);
    draw_framed_rect(ed, 
            buf->x, buf->y,
            buf->max_col, buf->max_row+1,
            frame_color, 2.0,
            MAP_XYWH, DRW_ONGRID);


    // filename stuff
    
    set_color_hex(ed, 0x181818);
    
    draw_rect(ed,
            buf->x, buf->y + buf->max_row,
            buf->max_col, 1,
            MAP_XYWH, DRW_ONGRID);

    if(has_file) {
        font_set_color_hex(&ed->font, frame_name_color);
        draw_data(ed, 
                buf->x,
                buf->y + buf->max_row,
                buf->filename,
                buf->filename_size,
                DRW_ONGRID);
    }

}



void draw_cursor(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    // find all tabs to cursor X position so it can rendered to correct place.
    const size_t num_tabs = string_num_chars(buf->current, 0, buf->cursor_x, '\t');
   
    const int cur_y = row_to_loc(ed, buf->cursor_y + buf->y - buf->scroll);
    const int cur_x = col_to_loc(ed, 
              buf->cursor_x 
            + buf->content_xoff
            + num_tabs * (FONT_TAB_WIDTH - 1)
            + buf->x);


    const unsigned int 
        cur_base_color = 0x104410;

    const unsigned int
        cur_color_2 = 0x10EE10;


    // cursor background.
    set_color_hex(ed, cur_base_color);
    draw_rect(ed, 
            cur_x,
            cur_y,
            ed->font.char_w,
            ed->font.char_h + 1,
            MAP_XYWH, DRW_NO_GRID);


    // cursor line below.
    set_color_hex(ed, cur_color_2);
    draw_rect(ed, 
            cur_x,
            cur_y + ed->font.char_h + 1,
            ed->font.char_w,
            2,
            MAP_XYWH, DRW_NO_GRID);

    // cursor line right side.
    draw_rect(ed,
            cur_x + ed->font.char_w - 1,
            cur_y + ed->font.char_h/2,
            2,
            ed->font.char_h/2 + 2,
            MAP_XYWH, DRW_NO_GRID);

}

void draw_buffer(struct editor_t* ed, struct buffer_t* buf, char* linenum_buf, int xdrw_off) {
    if(!buffer_ready(buf)) {
        return;
    }


    xdrw_off += buf->x;
    int ydrw_off = buf->y;
    
    struct string_t* line = NULL;


    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i >= (buf->max_row + buf->scroll)) {
            break;
        }

        line = buf->lines[i];
        if(!line) {
            continue;
        }

        const int y = i + ydrw_off - buf->scroll;
        const int line_num_len = snprintf(linenum_buf, LINENUM_BUF_SIZE, "%li", i);


        font_set_color_hex(&ed->font, 0xFFEEAA);
        draw_data(ed, xdrw_off, y, line->data, line->data_size, DRW_ONGRID);
       
        
        
        // line number stuff

        font_set_color_hex(
                &ed->font,
                (i == buf->cursor_y) ? 0x406040 : 0x252525);

        draw_data(ed, 
                xdrw_off - line_num_len - 2, y,
                linenum_buf, LINENUM_BUF_SIZE,
                DRW_ONGRID);

        
        font_set_color_hex(&ed->font, 0x504030);
        draw_char(ed, xdrw_off - 1, y, '|', 1);
    }
}


// 
// TODO: draw multple buffers.
//
void draw_buffers(struct editor_t* ed) {

    char linenum_buf[LINENUM_BUF_SIZE];
    struct buffer_t* buf = &ed->buffers[0];
   
    if(!buffer_ready(buf)) {
        write_message(ed, ERROR_MSG, "Uninitialized buffer!\0");
        return;
    }

    const int xdrw_off = buf->content_xoff;

    // need to draw buffer frames first 
    // or the cursor will be render under them.
    _draw_buffer_frame(ed, buf);
    
    draw_cursor(ed, buf);
    draw_buffer(ed, buf, linenum_buf, xdrw_off);
}




