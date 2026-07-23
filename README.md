# RID

A minimal text editor written from scratch in C, designed to provide an essential editing experience with built-in syntax highlighting for the C language.

<p align="center">
  <img src="assets/rid-dragon.png" alt="rid dragon" width="320">
</p>

## Features

* **Syntax Highlighting:** Full support for standard C11 keywords and categories.
* **Auto-completion:** Automatically inserts closing pairs for parentheses `()`, brackets `[]`, and braces `{}`.
* **Cursor Management:** Smooth navigation using terminal arrow keys.
* **Optional Word Wrapping:** Ability to disable automatic line wrapping via a command-line flag.
* **UTF-8 Support:** Internal handling of multibyte characters and Unicode codepoints.
* **Status & Header Bars:** Visual interface elements showing the editor name, version, and file information.

## Installation

The project includes a `Makefile` to streamline the build process. Ensure you have `gcc` installed, then run:

```bash
make

```

This command compiles the source files with safety and standard flags: `-Wall -Wextra -pedantic -std=c99`.

## Usage

To launch the editor with a specific file:

```bash
./rid <filename>

```

### Command Line Options

* `--nowr`: Disables the word wrapping feature.

### Keybindings

| Key | Action |
| --- | --- |
| **CTRL + S** | Save the current file and exit the editor |
| **CTRL + Q** | Quit the editor without saving changes |
| **BACKSPACE** | Delete the character before the cursor |
| **TAB** | Insert a tab character |
| **Arrow Keys** | Move the cursor through the text |

---

## Project Status

This software is currently a Work in Progress. Core functionalities are implemented, but the project is not yet considered feature-complete.

---
