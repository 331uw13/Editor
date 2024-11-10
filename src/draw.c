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
    *w *= CELLW;
    *h *= CELLH;
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

static size_t _draw_data_r(
        struct editor_t* ed,
        int x,
        int y,
        char* data,
        long int size,
        int use_grid,
        int use_wrapping,
        int max_x)
{
    size_t num_newlines = 0;
    if(!data) {
        goto done;
    }

    // search for null character?
    int seek_nc = 0;
    if((seek_nc = (size < 0) ? 1 : 0)) {
        size = STRING_MAX_SIZE;
    }

    const int xinc = use_grid ? 1 : CELLW;
    const int yinc = use_grid ? 1 : CELLH;
    const int xorigin = x;

    for(long int i = 0; i < size; i++) {
        int old_color = -1;
        char c = data[i];

        switch(c) {
            
            case 0:
                goto done;
            
            case 0x9:
                x += xinc * FONT_TAB_WIDTH;
                continue;

            case 0xA:
                if(use_wrapping) {
                    x = xorigin;
                    y += yinc;
                    num_newlines++;
                }
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
            num_newlines++;
        }

        if(old_color > 0) {
            font_set_color_hex(&ed->font, old_color);
        }
    }

done:
    return num_newlines;
}

void draw_data(struct editor_t* ed, int x, int y, char* data, long int size, int use_grid) {
    _draw_data_r(ed, x, y, data, size, use_grid, 0, 0);
}

size_t draw_data_w(struct editor_t* ed,
            int x, int y,
            char* data,
            long int size,
            int max_x,
            int use_grid) {
    return _draw_data_r(ed, x, y, data, size, use_grid, 1, max_x);
}


// _A brighter color / when it is active
// _B dimmer color
//
#define FRAME_COLOR_A    0x00AAEE
#define FRAME_COLOR_B    0x003344
#define FRAME_COLOR_BG   0x101111
#define FILENAME_COLOR_A 0x90A0E0
#define FILENAME_COLOR_B 0x203030
#define BAR_COLOR_A      0x041624
#define BAR_COLOR_B      0x03101a
#define CURSOR_COLOR_A   0x10EE10
#define CURSOR_COLOR_B   0x104410
#define DATA_COLOR       0xFFEEAA
#define LINENUM_COLOR_A  0x406040
#define LINENUM_COLOR_B  0x252525
#define READONLY_COLOR   0xA34514
#define BUFFER_MODEI_COLOR 0x278a8f
#define SELECT_COLOR       0x661d43 
#define SELECT_CURSOR_COLOR 0xA33B72
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
            buf->x + CELLW*3 + 15,
            buf->y + buf->height,
            buf->file.name,
            buf->file.name_size,
            DRW_NO_GRID);

    // mode stuff.
    // 
    font_set_color_hex(&ed->font, BUFFER_MODE_INDICCOLORS[buf->mode]);
    draw_data(ed,
            buf->x + 10,
            buf->y + buf->height,
            buf->mode_indicstr, BUFFER_MODE_INDICSIZE,
            DRW_NO_GRID);


    if(buf->file.readonly) {
        font_set_color_hex(&ed->font, READONLY_COLOR);
        draw_data(ed,
                buf->x + CELLW * (buf->file.name_size + 1),
                buf->y + buf->height,
                "[read only]\0", -1,
                DRW_NO_GRID);
    }
}


static void draw_buffer(
        struct editor_t* ed,
        struct buffer_t* buf,
        char* linenum_buf)
{
    if(!buffer_ready(buf)) {
        return;
    }


    struct string_t* line = NULL;
    const int xdrw_off = buf->content_xoff;


    const int cw = CELLW;
    const int ch = CELLH;


    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i >= buf->max_row + buf->scroll + 1) {
            break;
        }

        line = buf->lines[i];
        if(!string_ready(line)) {
            fprintf(stderr, "corrupted data?? buffer id:%i\n", buf->id);
            return;
        }


        int linenum_len = snprintf(linenum_buf, LINENUM_BUF_SIZE, "%li", i);

        // uhhh...
        int x = buf->x;
        int y = i - buf->scroll;
        int ln_x = x + cw * (xdrw_off - linenum_len - 2);
        int max_x = x + buf->width - cw;

        y *= ch;
        y += buf->y;
        x += cw * xdrw_off;

        font_set_color_hex(&ed->font, DATA_COLOR);
        draw_data(ed,
                x, y,
                line->data,
                line->data_size,
                DRW_NO_GRID);

        // line numbers
        //
        font_set_color_hex(&ed->font,
                (i == buf->cursor_y) ? LINENUM_COLOR_A : LINENUM_COLOR_B);

        draw_data(ed, ln_x, y, linenum_buf, LINENUM_BUF_SIZE, DRW_NO_GRID);

    }

}

static void draw_cursor(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    long int y = buf->cursor_y + buf->y - buf->scroll;
    long int x = buf->cursor_x + buf->content_xoff;

    /* ----- TODO
    if(buf->current->num_wraps > 0) {
       
        // figure out how many wraps there are
        // before index 'x'.
        // so the cursor draw y can be offset correctly.
        const int m = buf->max_col - buf->content_xoff;
        long int k = round(x / m);
        x = x % m;

        y += k;
    }
    */

    x *= cw;
    y *= ch;
    
    x += buf->x;
    y += buf->y;


    if(buf->mode != BUFMODE_SELECT) {
        
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
    else {
        
        set_color_hex(ed, SELECT_CURSOR_COLOR);


        draw_rect(ed, x, y, cw, ch, MAP_XYWH, DRW_NO_GRID);


    }
}


static int draw_selected_callback(
        struct buffer_t* buf,
        struct string_t* line,
        size_t line_y,
        int flag,
        void* userptr
        )
{
    int res = 0;
    struct editor_t* ed = (struct editor_t*) userptr;
    if(!ed) {
        goto done;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    set_color_hex(ed, SELECT_COLOR);


    long int y = line_y - buf->scroll;
    long int x = buf->content_xoff;
    long int width = line->data_size;


    if((flag & PROCSELECTED_BEGIN)) {
        x += buf->select.x0;
        width -= buf->select.x0;
    }

    if((flag & PROCSELECTED_END)) {
        width = buf->content_xoff + buf->select.x1 - x;
    }


    x *= cw;
    y *= ch;
    x += buf->x;
    y += buf->y;

    width = liclamp(width, 1, line->data_size);

    draw_rect(ed,
            x, y,
            width*cw, ch,
            MAP_XYWH, DRW_NO_GRID);


    res = 1;

done:
    return res;
}

void draw_buffers(struct editor_t* ed) {
    
    char linenum_buf[LINENUM_BUF_SIZE];

    int num_bufs = iclamp(ed->num_active_buffers, 0, MAX_BUFFERS);
    struct buffer_t* buf = NULL;
    for(int i = 0; i < num_bufs; i++) {
        buf = &ed->buffers[i];
        if(!buffer_ready(buf)) {
            write_message(ed, ERROR_MSG, "Uninitialized buffer(%i).\0", i);
            continue;
        }

        draw_buffer_frame(ed, buf, i == ed->current_buf_id);
        
        if(buf->mode == BUFMODE_SELECT) {
            buffer_proc_selected_reg(buf, ed, draw_selected_callback);
        }
        
        draw_cursor(ed, buf);
        
        draw_buffer(ed, buf, linenum_buf);

    }

}

void draw_everything(struct editor_t* ed) {

        draw_buffers(ed);

        if(ed->mode == MODE_COMMAND_LINE) {
            set_color_hex(ed, 0x231100);
            draw_framed_rect(ed, 
                    5, 3,
                    ed->window_width-10, ed->font.char_h+8,
                    0x554510,
                    1.0,
                    MAP_XYWH, DRW_NO_GRID);
            
            font_set_color_hex(&ed->font, 0x773310);
            draw_char(ed, 10, 6, '>', DRW_NO_GRID);


            // cmd_cursor
            //
            set_color_hex(ed, 0x225522);
            draw_rect(ed, 
                    10 + (ed->cmd_cursor+1) * ed->font.char_w,
                    6,
                    ed->font.char_w,
                    ed->font.char_h,
                    MAP_XYWH, DRW_NO_GRID);

            font_set_color_hex(&ed->font, 0xAA6633);
            draw_data(ed, ed->font.char_w+10, 6, 
                    ed->cmd_str->data, ed->cmd_str->data_size, DRW_NO_GRID);
        }


        draw_error_buffer(ed);
        draw_info_buffer(ed);
}



