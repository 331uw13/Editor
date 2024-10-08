#ifndef EDITOR_SHADER_H
#define EDITOR_SHADER_H



static const char VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 texture_coords;\n"
    "out vec2 tex_coords;\n"
    "\n"
    "void main() {\n"
    "    tex_coords = texture_coords;\n"
    "    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n\0",
};

static const char FRAGMENT_SHADER_SRC[] = {
    "#version 460 core\n"
    "out vec4 out_color;\n"
    "in vec2 tex_coords;"
    "\n"
    "uniform sampler2D tex;\n"
    "\n"
    "void main() {\n"
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, tex_coords).r);\n"
    "    out_color = vec4(1.0) * sampled;\n"
    "}\n\0",
};



unsigned int  create_shader          (const char* source, GLenum shader_type);
unsigned int  create_shader_program  (const char* vertex_src, const char* fragment_src);
void          delete_shader_program  (unsigned int shader);




#endif
