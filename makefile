# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Executable name
TARGET = shell

# Source files
SRCS = main.c hop.c io_redirect.c neonate.c pipe.c proclore.c reveal.c seek.c signal.c 

# Header files
HDRS = hop.h io_redirect.h neonate.h pipe.h proclore.h reveal.h seek.h signal.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile each source file into an object file
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
run:
	./$(TARGET)
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to avoid conflicts with files named clean or all
.PHONY: all clean