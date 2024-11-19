# 331uw13's Code Editor

> [!WARNING]
> The Editor is not finished. It needs alot more work to be done.

### Dependencies
* OpenGL, GLFW, GLEW, Freetype

#### Run the commands to test the application:
```bash
git clone https://github.com/331uw13/Editor.git
cd Editor
zig build && ./zig-out/bin/editor
```
> you can also use gccbuild.sh

![Editor](https://github.com/331uw13/Editor/blob/main/screenshots/Editor_2024-11-10_04-09.png?raw=true)

## Keybinds
| Key | Description |
| --- | --- |
| `CTRL + X` | Change mode (see mode options below)|
| `CTRL + D` | Goto end of the line |
| `CTRL + A` | Goto start of the line |
| `CTRL + S` | Goto middle of the line |
| `CTRL + W` | Write to file |
| `CTRL + P` | Command line mode |
| `CTRL + TAB` | Cycle through active buffers |
| `CTRL + LEFT/RIGHT/UP/DOWN` | Movement multiplied by 4 |
| `CTRL + SHIFT + UP` | Goto start of the buffer |
| `CTRL + SHIFT + DOWN` | Goto end of the buffer |
| `SHIFT + LEFT/RIGHT` | Move by 1 word |
| `SHIFT + UP/DOWN` | Move up/down but skip rows containing only whitespace |

## Mode options
| Key | Description |
| --- | --- |
| `c` | Insert mode |
| `s` | Select mode |
| `a` | Replace mode |

## Select mode Keybinds
| Key | Description |
| --- | --- |
| `d` | Delete selected region |

## Commands
`q.` quit without asking to save

`open <path>` read file to current buffer

`open-new` read file to new buffer

`write` save file

`write <new filename>` create a new file 

