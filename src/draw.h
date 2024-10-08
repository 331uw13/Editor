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


void set_color(struct editor_t* ed, float r, float g, float b); // rgb 0.0 - 1.0
void set_color_hex(struct editor_t* ed, unsigned int hex);

void draw_rect(struct editor_t* ed, float x, float y, float w, float h);



#endif
