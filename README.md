# 331uw13's Code Editor

> [!WARNING]
> The Editor is not finished. It needs alot more work to be done.


#### Run the commands to test the application:
```bash
git clone https://github.com/331uw13/Editor.git
cd Editor
zig build && ./zig-out/bin/editor
```

![Editor](https://github.com/331uw13/Editor/blob/main/screenshots/Editor_2024-10-26_18-49.png?raw=true)

## Keybinds
| Key | Description |
| --- | --- |
| `CTRL + D` | Goto end of the line |
| `CTRL + A` | Goto start of the line |
| `CTRL + S` | Goto middle of the line |
| `CTRL + W` | Write to file |
| `CTRL + P` | Command line mode |
| `CTRL + TAB` | Cycle through active buffers |
| `CTRL + LEFT/RIGHT/UP/DOWN` | Movement multiplied by 4 |
| `ALT + UP` | Goto start of the buffer |
| `ALT + DOWN` | Goto end of the buffer |
| `SHIFT + LEFT/RIGHT` | Move by 1 word |
| `SHIFT + UP/DOWN` | Move up/down but skip rows containing only whitespace |

## Commands
`q.` quit without asking to save

`open <path>` read file to current buffer
