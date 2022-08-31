CC := g++
CFLAGS := -Wall

TARGET := elenco

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

$(TARGET):