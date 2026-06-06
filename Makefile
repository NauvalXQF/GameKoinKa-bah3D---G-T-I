CC = g++
CFLAGS = -Wall -std=c++11
LDFLAGS = -lopengl32 -lglu32 -lfreeglut -lm -lwinmm

OBJS = main.o environment.o player.o
TARGET = Tawaf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET).exe $(OBJS) $(LDFLAGS)
	powershell -Command "Unblock-File '$(TARGET).exe'" 2>nul || true
	powershell -Command "Copy-Item '$(TARGET).exe' 'C:\Users\nauva\Desktop\$(TARGET).exe' -Force; Copy-Item '*.bmp','*.wav' 'C:\Users\nauva\Desktop\' -Force" 2>nul || true

main.o: main.cpp globals.h environment.h player.h
	$(CC) $(CFLAGS) -c main.cpp

environment.o: environment.cpp environment.h globals.h
	$(CC) $(CFLAGS) -c environment.cpp

player.o: player.cpp player.h globals.h
	$(CC) $(CFLAGS) -c player.cpp

clean:
	powershell -Command "Remove-Item -Force *.o, $(TARGET).exe -ErrorAction SilentlyContinue"

run: $(TARGET)
	.\$(TARGET).exe

.PHONY: all clean run
