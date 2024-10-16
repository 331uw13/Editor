#ifndef FONT_H
#define FONT_H


#define FONT_NUM_CHARS 95

#define FONT_MIN_SCALE 0.15f
#define FONT_MAX_SCALE 1.5f
#define FONT_TAB_WIDTH 4




static const char FONT_VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 texture_coords;\n"
    "uniform vec3 font_color;\n"
    "out vec2 tex_coords;\n"
    "out vec3 color;\n"
    "\n"
    "void main() {\n"
    "    color = font_color;\n"
    "    tex_coords = texture_coords;\n"
    "    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n\0",
};

static const char FONT_FRAGMENT_SHADER_SRC[] = {
    "#version 460 core\n"
    "out vec4 out_color;\n"
    "in vec2 tex_coords;\n"
    "in vec3 color;\n"
    "\n"
    "uniform sampler2D tex;\n"
    "\n"
    "void main() {\n"
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, tex_coords).r);\n"
    "    out_color = vec4(color, 1.0) * sampled;\n"
    "}\n\0",
};



struct glyph_t {
    unsigned int texture; // texture id opengl can use
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
    
    unsigned int shader;
    unsigned int vbo;
    unsigned int vao;
    int shader_color_uniloc; // uniform location for 'font_color'
};


struct editor_t;

int load_font(
        const char* file_path,
        struct font_t* font,
        const char* vert_shader,
        const char* frag_shader
        );
int unload_font(struct font_t* font);

void    font_set_scale(struct font_t* font, float scale);
void    font_set_color(struct font_t* font, float r, float g, float b); // rgb 0.0 - 1.0.
void    font_set_color_hex(struct font_t* font, unsigned int hex);


#endif
