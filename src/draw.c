#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "editor.h"
#include "draw.h"
#include "utils.h"
#include "colors.h"

static void to_grid_coords(struct editor_t* ed,
        float* x, float* y,
        float* w, float* h) 
{
    *x = (float)col_to_loc(ed, *x);
    *y = (float)row_to_loc(ed, *y);
    *w *= CELLW;
    *h *= CELLH;
}



void set_color(struct editor_t* ed, unsigned int hex) {
    glUseProgram(ed->shader);
    glUniform3f(ed->shader_color_uniloc, UNHEX(hex));
    ed->drw_color_hex = hex;
}


void draw_rect(
        struct editor_t* ed,
        float x, float y,
        float w, float h, int usegrid, float adj_x, float adj_y) {

    if((w * h) <= 0) {
        return;
    }

    if((usegrid & DRW_ONGRID)) {
        to_grid_coords(ed, &x, &y, &w, &h);

        if((usegrid & DRW_ADJP)) {
            x -= EDITOR_X_PADDING;
            w += EDITOR_X_PADDING;
            y -= EDITOR_Y_PADDING;
            h += EDITOR_Y_PADDING;
        }

        x += adj_x;
        y += adj_y;
        w += abs(adj_x);
        h += abs(adj_y);
    }

    map_xywh(ed, &x, &y, &w, &h);

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


void draw_char(
        struct editor_t* ed,
        int x, int y, unsigned char c)
{
    

    if(!char_ok(c)) {
        return;
    }
    struct glyph_t* g = &ed->font.glyphs[c - 0x20];

    if(!g) {
        return;
    }

    x = col_to_loc(ed, x);
    y = row_to_loc(ed, y);

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
        int use_wrapping,
        int max_x,
        int use_syntax)
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

    const int xorigin = x;


    for(long int i = 0; i < size; i++) {
        int old_color = -1;
        char c = data[i];

        switch(c) {
            case 0:
                goto done;
            
            case 0x9:
                x += FONT_TAB_WIDTH;
                continue;

            case 0xA:
                if(use_wrapping) {
                    x = xorigin;
                    y++;
                    num_newlines++;
                }
                continue;

            default:
                if(!char_ok(c)) {
                    //printf("unknown character: 0x%x/'%c'\n", c, c);
                    old_color = ed->font.color_hex;
                    font_set_color_hex(&ed->font, 0xFF3020);
                    c = '?';
                }
                break;
        }

        draw_char(ed, x, y, c);

        x++;

        if(use_wrapping && (x >= (xorigin + max_x))) {
            x = xorigin;
            y++;
            num_newlines++;
        }

        if(old_color > 0) {
            font_set_color_hex(&ed->font, old_color);
        }
    }

done:
    return num_newlines;
}

void draw_data(struct editor_t* ed, 
        int x, int y, char* data, long int size) {
    _draw_data_r(ed, x, y, data, size, 0, 0, 0);
}

void draw_data_wrp(struct editor_t* ed,
        int x, int y,
        char* data, long int size,
        int max_col
        ) {
    _draw_data_r(ed, x, y, data, size, 1, max_col, 0);
}


static void draw_buffer_data(struct editor_t* ed, struct buffer_t* buf) {
 
    if(!buffer_ready(buf)) {
        return;
    }

    set_color(ed, ed->colors[BACKGROUND_COLOR]);
    draw_rect(ed, 0, 0,
            buf->max_col+1,
            buf->max_row+1, 
            DRW_ONGRID | DRW_ADJP, DRW_NOADJAFTER);


    struct string_t* line = NULL;
    const int xdrw_off = buf->content_xoff;
    char linenum_buf[LINENUM_BUF_SIZE];

    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i > (buf->max_row) + buf->scroll) {
            break;
        }


        line = buf->lines[i];
        if(!string_ready(line)) {
            fprintf(stderr, "corrupted data?? buffer id:%i\n", buf->id);
            return;
        }


        int linenum_len = snprintf(linenum_buf, LINENUM_BUF_SIZE, "%li", i);

        int y = i - buf->scroll;
        int ln_x = (xdrw_off - linenum_len - 2);

        // buffer data

        font_set_color_hex(&ed->font, ed->colors[FOREGROUND_COLOR]);
        draw_data(ed, xdrw_off, y, line->data, line->data_size);

        
        // buffer line numbers

        font_set_color_hex(&ed->font,
                (i == buf->cursor_y) 
                ? ed->colors[LINENUM_COLOR_A] : ed->colors[LINENUM_COLOR_B]);

        draw_data(ed, ln_x, y, linenum_buf, LINENUM_BUF_SIZE);
    }
}


static void draw_cursor(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    long int x = buf->cursor_x + buf->content_xoff;
    long int y = buf->cursor_y - buf->scroll;

    set_color(ed, buf->cursor_color);
    draw_rect(ed, x, y, 1, 1, 
            DRW_ONGRID,
            -1.0, -1.32 /* adjustment */
            );


    if(buf->cursor_x < buf->current->data_size) {
        font_set_color_hex(&ed->font, buf->cursor_charcolor);
        draw_char(ed, x, y, 
                string_get_char(buf->current, buf->cursor_x));
    } 


}

static void draw_buffer_nameinfo(struct editor_t* ed, struct buffer_t* buf) {

    // bottom bar
    set_color(ed, ed->colors[BAR_COLOR]);
    draw_rect(ed, 
            0, 
            buf->max_row,
            buf->max_col+1,
            1,
            DRW_ONGRID | DRW_ADJP, DRW_NOADJAFTER);
   
    // filename
    font_set_color_hex(&ed->font, ed->colors[FILENAME_COLOR]);
    draw_data(ed,
            4,
            buf->max_row,
            buf->file.name,
            buf->file.name_size);


    // mode
    font_set_color_hex(&ed->font, BUFFER_MODE_INDICCOLORS[buf->mode]);
    draw_data(ed,
            0,
            buf->max_row,
            buf->mode_indicstr, BUFFER_MODE_INDICSIZE);


    if(buf->file.readonly) {
        font_set_color_hex(&ed->font, ed->colors[READONLY_COLOR]);
        draw_data(ed,
                buf->file.name_size + 5,
                buf->max_row,
                "(Read Only)\0", -1);
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

    set_color(ed, ed->colors[SELECT_REGION_COLOR]);


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

    if(y >= (buf->y + buf->height - CELLH)) {
        goto done;
    }

    width = liclamp(width, 1, line->data_size);

    draw_rect(ed,
            x, y,
            width, 1,
            DRW_ONGRID, DRW_NOADJAFTER);

    // draw text on top of just drawn rectangle.
    // the reason why not just change the order, 
    // user can change the text color that is selected
    if(line->data_size > 0) {
        font_set_color_hex(&ed->font, ed->colors[SELECT_CURSORCHAR_COLOR]);
        size_t offs = 0;
        if((flag & PROCSELECTED_BEGIN)) {
            offs = buf->select.x0;
        }
        draw_data(ed, x, y, line->data + offs, width);
    }

    res = 1;
done:
    return res;
}


static int draw_blockslct_callback(
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
    
    long int y = buf->select.y0;
    long int x = buf->content_xoff;
    x += buf->select.x0;

    set_color(ed, 0x22FF22);

    draw_rect(ed, 
            x, y,
            1, 1,
            DRW_ONGRID, DRW_NOADJAFTER);


    res = 1;
done:
    return res;
}



#define TABNAME_MAX_CHARS 16

static void draw_tabs(struct editor_t* ed) {

    if(!ed->tabs_visible) {
        return;
    }

    set_color(ed, 0x24211d);

    int tabname_x = 2;
    int tabname_y = ed->max_row-2;
    draw_rect(ed,
            0, tabname_y,
            ed->max_column+1, 1,
            DRW_ONGRID | DRW_ADJP, DRW_NOADJAFTER);


    for(size_t i = 0; i < ed->num_buffers; i++) {
        struct buffer_t* buf = &ed->buffers[i];

        if(buf->file.name_size > 0) {

            int currentbuf = (i == ed->current_bufid);
 
            if(currentbuf) {
                font_set_color_hex(&ed->font, 0x68d44e);
                draw_char(ed, tabname_x, tabname_y, '*');
                tabname_x += 1;
            }

            int toolong_name = (buf->file.name_size > TABNAME_MAX_CHARS);

            size_t tabname_size = toolong_name ? TABNAME_MAX_CHARS : buf->file.name_size;
            size_t offset = toolong_name ? buf->file.name_size-TABNAME_MAX_CHARS : 0;


            font_set_color_hex(&ed->font, currentbuf ? 0x649459 : 0x4f7047);
            draw_data(ed, tabname_x, tabname_y,
                    buf->file.name + offset, 
                    tabname_size);
            
            tabname_x += tabname_size +2;

        }
        else {
            // TODO: empty name.
        }
    }
}


static void draw_commandline(struct editor_t* ed) {
    
    if(ed->mode != MODE_CMDL) {
        return;
    }

    int y = ed->max_row - (ed->tabs_visible ? 3 : 2);
    int x = 0;

    set_color(ed, ed->colors[CMDLBG_COLOR]);
    draw_rect(ed, x, y, ed->max_column+1, 1, DRW_ONGRID | DRW_ADJP, DRW_NOADJAFTER);

    font_set_color_hex(&ed->font, ed->colors[CMDLFG_COLOR]);
    draw_char(ed, x, y, '>');


    draw_data(ed, x+2, y, ed->cmdstr->data, ed->cmdstr->data_size);

    set_color(ed, ed->colors[CMDLCURSOR_COLOR]);
    draw_rect(ed, x+2 + ed->cmd_cursor, y, 1, 1, DRW_ONGRID, 0.0, -1.0);

    font_set_color_hex(&ed->font, ed->colors[CMDLCURSORCHAR_COLOR]);
    draw_char(ed, x+2 + ed->cmd_cursor, y, string_get_char(ed->cmdstr, ed->cmd_cursor));

}

 

void draw_everything(struct editor_t* ed) {

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    
    draw_buffer_data(ed, buf);
    draw_buffer_nameinfo(ed, buf);
    draw_cursor(ed, buf);

    if(buf->mode == BUFMODE_SELECT) {
        buffer_proc_selected_reg(buf, ed, draw_selected_callback);
    }
 
    draw_tabs(ed);
    draw_commandline(ed);
    draw_info_buffer(ed);
    draw_error_buffer(ed);
}




/*
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
*/



/*
static size_t _draw_data_r(
        struct editor_t* ed,
        int x,
        int y,
        char* data,
        long int size,
        int use_grid,
        int use_wrapping,
        int max_x,
        int use_syntax)
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
                    font_set_color_hex(&ed->font, 0xFF3020);
                    c = '?';
                }
                break;
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
    _draw_data_r(ed, x, y, data, size, use_grid, 0, 0, 0);
}

void draw_data_syntax(struct editor_t* ed, int x, int y, char* data, long int size, int use_grid) {
    _draw_data_r(ed, x, y, data, size, use_grid, 0, 0, 1);
}

size_t draw_data_w(struct editor_t* ed,
            int x, int y,
            char* data,
            long int size,
            int max_x,
            int use_grid) {
    return _draw_data_r(ed, x, y, data, size, use_grid, 1, max_x, 0);
}
*/


    /*
static void draw_buffer_frame(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    set_color_hex(ed, ed->colors[BACKGROUND_COLOR]);

    draw_rect(ed, 
            buf->x,
            buf->y,
            buf->width,
            buf->height,
            MAP_XYWH, DRW_NO_GRID);



    // filename stuff
    //
    set_color_hex(ed, ed->colors[BAR_COLOR_A]);
    draw_rect(ed,
            buf->x,
            buf->y + buf->height - CELLH,
            buf->width,
            CELLH,
            MAP_XYWH, DRW_NO_GRID);
   
    font_set_color_hex(&ed->font, ed->colors[FILENAME_COLOR_A]);
    draw_data(ed,
            buf->x + CELLW*3 + 15,
            buf->y + buf->height - CELLH,
            buf->file.name,
            buf->file.name_size,
            DRW_NO_GRID);

    // mode stuff.
    // 
    font_set_color_hex(&ed->font, BUFFER_MODE_INDICCOLORS[buf->mode]);
    draw_data(ed,
            buf->x + 10,
            buf->y + buf->height - CELLH,
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
    */

    /*

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
        if(i > buf->max_row + buf->scroll) {
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


        y *= ch;
        y += buf->y;
        x += cw * xdrw_off;


        font_set_color_hex(&ed->font, ed->colors[FOREGROUND_COLOR]);
        draw_data_syntax(ed,
                x, y,
                line->data,
                line->data_size,
                DRW_NO_GRID);

        // line numbers
        //
        font_set_color_hex(&ed->font,
                (i == buf->cursor_y) 
                ? ed->colors[LINENUM_COLOR_A] : ed->colors[LINENUM_COLOR_B]);

        draw_data(ed, ln_x, y, linenum_buf, LINENUM_BUF_SIZE, DRW_NO_GRID);

    }

}
    */

    /*
static void draw_cursor(struct editor_t* ed, struct buffer_t* buf) {
    if(!buffer_ready(buf)) {
        return;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    long int y = buf->cursor_y + buf->y - buf->scroll;
    long int x = buf->cursor_x + buf->content_xoff;

    x *= cw;
    y *= ch;
    
    x += buf->x;
    y += buf->y;

    // cursor background
    //
    set_color_hex(ed, buf->cursor_color[1]);
    draw_rect(ed, x, y, cw, ch, MAP_XYWH, DRW_NO_GRID);

    set_color_hex(ed, buf->cursor_color[0]);
   

    switch(buf->mode) {

        case BUFMODE_INSERT:
        case BUFMODE_REPLACE:
        case BUFMODE_NONE:
        case BUFMODE_B_SELECT:
            {
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
            break;


        case BUFMODE_SELECT:
            {
                draw_rect(ed,
                    x-1,
                    y,
                    2,
                    ch,
                    MAP_XYWH, DRW_NO_GRID);

            }
            break;
    }

    if(buf->cursor_x < buf->current->data_size) {
        font_set_color_hex(&ed->font, buf->cursor_charcolor);
        draw_char(ed, x, y, 
                buf->current->data[buf->cursor_x], DRW_NO_GRID);
    }
}
    */


    /*
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

    res = 1;
    

    const int cw = CELLW;
    const int ch = CELLH;

    set_color_hex(ed, ed->colors[SELECT_REGION_COLOR]);


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

    if(y >= (buf->y + buf->height - ch)) {
        goto done;
    }

    width = liclamp(width, 1, line->data_size);

    draw_rect(ed,
            x, y,
            width*cw, ch,
            MAP_XYWH, DRW_NO_GRID);



done:
    return res;
}
    */

    /*
static int draw_B_selected_callback(
        struct buffer_t* buf,
        struct string_t* line,
        size_t line_y,
        int flag,
        void* userptr
        )
{
    int res = 1;

    struct editor_t* ed = (struct editor_t*) userptr;
    if(!ed) {
        res = 0;
        goto error;
    }

    const int cw = CELLW;
    const int ch = CELLH;


    long int y = line_y - buf->scroll;
    long int x = buf->content_xoff - 1;
    
    x *= cw;
    y *= ch;
    x += buf->x;
    y += buf->y;

    // left side line
    set_color_hex(ed, ed->colors[B_SELECT_REGION_COLOR_A]);
    draw_rect(ed,
            x, y,
            cw/2, ch,
            MAP_XYWH, DRW_NO_GRID);


    // color affected text background
    
    set_color_hex(ed, ed->colors[B_SELECT_REGION_COLOR_B]);
    draw_rect(ed,
            x+cw, y,
            (line->data_size > 0 ? line->data_size : 1) * cw,
            ch,
            MAP_XYWH, DRW_NO_GRID
            );

error:
    return res;
}
    */


/*
#define TABNAME_MAX_CHARS 16

static void draw_tabs(struct editor_t* ed) {

    if(!ed->show_tabs) {
        return;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    set_color_hex(ed, 0x24211d);

    int tabname_x = cw*2;
    int tabname_y = ed->window_height - ch*2;
    draw_rect(ed,
            0, tabname_y,
            ed->window_width, ch,
            MAP_XYWH, DRW_NO_GRID);



    for(size_t i = 0; i < ed->num_buffers; i++) {
        struct buffer_t* buf = &ed->buffers[i];

        if(buf->file.name_size > 0) {

            int currentbuf = (i == ed->current_bufid);
 
            if(currentbuf) {
                font_set_color_hex(&ed->font, 0x68d44e);
                draw_char(ed, tabname_x, tabname_y, '*', DRW_NO_GRID);
                tabname_x += cw;
            }

            int toolong_name = (buf->file.name_size > TABNAME_MAX_CHARS);

            size_t tabname_size = toolong_name ? TABNAME_MAX_CHARS : buf->file.name_size;
            size_t offset = toolong_name ? buf->file.name_size-TABNAME_MAX_CHARS : 0;



            font_set_color_hex(&ed->font, currentbuf ? 0x649459 : 0x4f7047);
            draw_data(ed, tabname_x, tabname_y,
                    
                    buf->file.name + offset, 
                    tabname_size,
                    
                    DRW_NO_GRID);
            
            tabname_x += tabname_size * cw + cw*2;

        }
        else {
            
        }
    }
}
    */


    /*
void draw_buffers(struct editor_t* ed) {
    
    char linenum_buf[LINENUM_BUF_SIZE];

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    if(!buffer_ready(buf)) {
        printf("^ '%s'\n", __func__);
        write_message(ed, ERROR_MSG, "Uninitialized buffer(%i).\0", buf->id);
        return;
    }

  
    draw_buffer_frame(ed, buf);
    
    if(buf->mode == BUFMODE_SELECT) {
        buffer_proc_selected_reg(buf, ed, draw_selected_callback);
    }
    else if(buf->mode == BUFMODE_B_SELECT) {
        buffer_proc_selected_reg(buf, ed, draw_B_selected_callback);
    }
    
    draw_tabs(ed);
    
    draw_buffer(ed, buf, linenum_buf);
    draw_cursor(ed, buf);

}

    */
    /*
void draw_everything(struct editor_t* ed) {

    draw_buffers(ed);

    if(ed->mode == MODE_COMMAND_LINE) {
        set_color_hex(ed, 0x231100);
        draw_rect(ed, 
                5, 3,
                ed->window_width-10, ed->font.char_h+8,
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


}

    */


