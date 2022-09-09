CC 		:= g++
CFLAGS 	:= -Wall

SOURCES := main.cpp
TARGET 	:= elenco

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)