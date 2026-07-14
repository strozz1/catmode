# Cat Mode 🐈

Cat Mode is a lightweight X11 overlay for Linux that protects your keyboard and mouse from curious cats (or accidental input).

<p align="center">
  <img src="assets/screenshot.png" alt="Cat Mode screenshot" width="900">
</p>

When enabled, it displays a fullscreen translucent overlay with a friendly cat, blocks user input, and can be dismissed using a predefined key combination.


## Dependencies
- XCB
- XCB RandR
- Cairo
- Pango / Pangocairo
## Build

```bash
make
```
## Install

```bash
sudo make install
```
## Run

```bash
catmode
```
## Purpose
My cat always manages to mess up my PC whenever I step away. I'm also too lazy
to lock my computer and type my password every time I get up, so I built a lil
tool to just block keyboard and mouse input. And to unblock I just press Cntrl-F. :)

## Advisory
When I started this project, I knew nothing about X11 or low-level Linux graphics programming. 
I used Claude as a learning companion, asking lots of questions and having it explain
each concept as I went. It helped me understand how X11, XCB, Cairo, and the rest of the stack work.

And it's a messy code
## License
MIT
