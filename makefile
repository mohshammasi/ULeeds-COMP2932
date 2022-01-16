# project name (generate executable with this name)
TARGET   = compiler

CC       = g++ -std=c++11 -Wall
# compiling flags here
CFLAGS   = -Wall

LINKER   = g++ -o
# linking flags here
LFLAGS   = -lm -Wall -std=c++11

SOURCES  := $(wildcard *.cpp)
INCLUDES := $(wildcard *.h)
OBJECTS  := $(SOURCES:.c=*.o)
rm       = rm -f

$(TARGET): obj
	@$(LINKER) $(TARGET) $(LFLAGS) $(OBJECTS)

obj: $(SOURCES) $(INCLUDES)
	@$(CC) $(CFLAGS) $(SOURCES)

clean:
	@$(rm) $(TARGET) $(OBJECTS)
