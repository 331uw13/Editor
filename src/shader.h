#ifndef EDITOR_SHADER_H
#define EDITOR_SHADER_H


unsigned int  create_shader          (const char* source, GLenum shader_type);
unsigned int  create_shader_program  (const char* vertex_src, const char* fragment_src);
void          delete_shader_program  (unsigned int shader);




#endif
