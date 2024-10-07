#ifndef EDITOR_SHADER_H
#define EDITOR_SHADER_H



static const char VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"
    "layout (location = 0) in vec3 pos;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
    "}\n\0",
};

static const char FRAGMENT_SHADER_SRC[] = {
    "#version 460 core\n"
    "out vec4 out_color;\n"
    "\n"
    "void main() {\n"
    "    out_color = vec4(1.0, 0.6, 0.2, 1.0);\n"
    "}\n\0",
};



unsigned int  create_shader          (const char* source, GLenum shader_type);
unsigned int  create_shader_program  (const char* vertex_src, const char* fragment_src);
void          delete_shader_program  (unsigned int shader);




#endif
