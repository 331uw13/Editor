#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <GLFW/glfw3.h>


void key_input_handler        (GLFWwindow* win, int key, int scancode, int action, int mods);
void char_input_handler       (GLFWwindow* win, unsigned int codepoint);
void scroll_input_handler     (GLFWwindow* win, double xoff, double yoff);
//void mouse_bttn_input_handler (GLFWwindow* win, int button, int action, int mods);


#endif
