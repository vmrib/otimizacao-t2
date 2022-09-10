CC 		:= g++
CFLAGS 	:= -Wall

SOURCES := main.cpp
TARGET 	:= elenco

.PHONY: all clean debug

all: $(TARGET)

debug: CFLAGS := -g -DDEBUG $(CFLAGS)
debug: $(TARGET)

clean:
	$(RM) $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)