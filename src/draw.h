#ifndef EDITOR_DRAW_H
#define EDITOR_DRAW_H


static const char DRW_VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "out vec3 color;\n"
    "uniform vec3 v_color;\n"
    "\n"
    "void main() {\n"
    "    color = v_color;\n"
    "    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n\0",
};

static const char DRW_FRAGMENT_SHADER_SRC[] = {
    "#version 460 core\n"
    "out vec4 out_color;\n"
    "in vec3 color;\n"
    "\n"
    "void main() {\n"
    "    out_color = vec4(color, 1.0);\n"
    "}\n\0",
};

struct editor_t;
struct buffer_t;


// options for 'usegrid'
#define DRW_NO_GRID (1<<0)
#define DRW_ONGRID  (1<<1)
// only for 'draw_rect':
#define DRW_ADJP (1<<2) // adjustment for xy padding (see editor.h)
#define DRW_NOADJAFTER 0.0, 0.0 // no xy adjustment 
                                // after converting to grid coordinates

void draw_everything(struct editor_t* ed);


void set_color(struct editor_t* ed, unsigned int hex);

void draw_rect(
        struct editor_t* ed,
        float x, float y,
        float w, float h, 
        int usegrid, 

        // for adjustment after calculating xywh to grid coordinates.
        // DRW_NOADJAFTER should be used for more readability.
        float adj_x, float adj_y
        ); 

void draw_char(
        struct editor_t* ed,
        int x, int y, unsigned char c);

void draw_data(struct editor_t* ed,
        int x, int y,
        char* data, long int size
        );

// draw data line wrapped.
void draw_data_wrp(struct editor_t* ed,
        int x, int y,
        char* data, long int size,
        int max_col
        );




#endif
