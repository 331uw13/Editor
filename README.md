# 331uw13's Code Editor

> [!WARNING]
> The Editor is not finished. It needs alot more work to be done.

### Dependencies
* OpenGL, GLFW, GLEW, Freetype

#### Run the commands to test the application:
```bash
git clone https://github.com/331uw13/Editor.git
cd Editor
./gccbuild.sh r
```

> [!CAUTION]
> Current status: not functioning.

![Editor](https://github.com/331uw13/Editor/blob/main/screenshots/2024-12-09_15-10-Editor.png?raw=true)

## Keybinds
| Key | Description |
| --- | --- |
| `CTRL + X` | Change mode (see mode options below)|
| `CTRL + D` | Goto end of the line |
| `CTRL + A` | Goto start of the line |
| `CTRL + S` | Goto middle of the line |
| `CTRL + W` | Write to file |
| `CTRL + V` | Paste clipboard to cursor position |
| `CTRL + P` | Command line mode |
| `CTRL + E` | Remove current line |
| `CTRL + TAB` | Cycle through tabs |
| `CTRL + LEFT/RIGHT/UP/DOWN` | Movement multiplied by 4 |
| `CTRL + SHIFT + UP` | Goto start of the buffer |
| `CTRL + SHIFT + DOWN` | Goto end of the buffer |
| `SHIFT + LEFT/RIGHT` | Move by 1 word |
| `SHIFT + UP/DOWN` | Move up/down but skip rows containing only whitespace |
| `ALT + TAB` | Toggle tabs |
| `ALT + LEFT/RIGHT` | Move through tabs |

## Mode options
| Key | Description |
| --- | --- |
| `c` | Insert mode |
| `s` | Select mode |
| `d` | B Select mode (just different type select mode, doesnt select columns only rows) |
| `a` | Replace mode |

## Select mode Keybinds
| Key | Description |
| --- | --- |
| `q` | Delete selected region |
| `c` | Copy selected region |
| `d` | Copy and then delete selected region |

## B - Select mode Keybinds
| Key | Description |
| --- | --- |
| `a` | Move selected rows left by font tab width |
| `d` | Move selected rows right by font tab width |



## Commands
`q.` quit without asking to save

`open <path>` read file to current buffer

`open-new` read file to new buffer

`write` save file

`write <new filename>` create a new file 

