#include <GL/glew.h>

#include "editor.h"
#include "draw.h"
#include "utils.h"


void set_color(struct editor_t* ed, float r, float g, float b) {
    glUseProgram(ed->shader);
    glUniform3f(ed->shader_color_uniloc, r, g, b);
}

void set_color_hex(struct editor_t* ed, unsigned int hex) {
    set_color(ed, UNHEX(hex));
}

void draw_rect(struct editor_t* ed, float x, float y, float w, float h) {
    map_xywh(ed, &x, &y, &w, &h);

    float vertices[] = {
        x,  y+h,
        x,  y,
        x+w,  y,

        x,  y+h,
        x+w,  y,
        x+w,  y+h
    };

    glBindVertexArray(ed->vao);
    glUseProgram(ed->shader);

    glBindBuffer(GL_ARRAY_BUFFER, ed->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



