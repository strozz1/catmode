CC = gcc

CFLAGS = -Wall -Wextra -std=c11 \
	$(shell pkg-config --cflags xcb xcb-randr cairo cairo-xcb pangocairo)

LDFLAGS = $(shell pkg-config --libs xcb xcb-randr cairo cairo-xcb pangocairo) -lm

SRC = main.c overlay.c cairo.c animation.c
OBJ = $(SRC:.c=.o)

TARGET = catmode

PREFIX ?= /usr/local

BINDIR      = $(PREFIX)/bin
DATADIR     = $(PREFIX)/share/catmode
APPDIR      = $(PREFIX)/share/applications
ICONDIR     = $(PREFIX)/share/icons/hicolor/256x256/apps

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

oldinstall: $(TARGET)
		mv $(TARGET) /usr/local/bin/catmode
		mkdir /usr/local/share/catmode  -p
		cp cat.png /usr/local/share/catmode/background.png
		cp catsheet.png /usr/local/share/catmode/catsheet.png


install:
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(APPDIR)
	install -d $(DESTDIR)$(ICONDIR)

	install -m755 catmode $(DESTDIR)$(BINDIR)
	install -m644 assets/catmode.png $(DESTDIR)$(DATADIR)
	install -m644 assets/catsheet.png $(DESTDIR)$(DATADIR)
	install -m644 assets/catmode.png $(DESTDIR)$(ICONDIR)
	install -m644 catmode.desktop $(DESTDIR)$(APPDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/catmode
	rm -rf $(DESTDIR)$(DATADIR)
	rm -f $(DESTDIR)$(ICONDIR)/catmode.png
	rm -f $(DESTDIR)$(APPDIR)/catmode.desktop

.PHONY: all clean run
