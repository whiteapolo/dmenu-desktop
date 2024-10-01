# Variables
CC = cc
CFLAGS = -Wall -Wextra -O3
TARGET = dmenu-desktop
PREFIX = ~/.local/bin
SRC = main.c

# Default rule
all: $(TARGET)

# Compile the program with multiple source files
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean rule to remove the executable and object files
clean:
	rm -f $(TARGET)

# Install rule to move the executable to ~/.local/bin/
install: $(TARGET)
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/$(TARGET)

# Uninstall rule to remove the executable from ~/.local/bin/
uninstall:
	rm -f $(PREFIX)/$(TARGET)

# Phony targets to avoid conflicts with file names
.PHONY: all clean install uninstall
