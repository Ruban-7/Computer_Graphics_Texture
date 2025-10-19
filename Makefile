EXE = hw6

ifeq "$(OS)" "Windows_NT"
    CFLG = -O3 -Wall -std=c99
    LIBS = -lfreeglut -lglu32 -lopengl32 -lm
    CLEAN = rm -f *.exe *.o *.a
else
    ifeq "$(shell uname)" "Darwin"
        CFLG = -O3 -Wall -Wno-deprecated-declarations -std=c99
        LIBS = -framework GLUT -framework OpenGL
    else
        CFLG = -O3 -Wall -std=c99
        LIBS = -lglut -lGLU -lGL -lm
    endif
    CLEAN = rm -f $(EXE) *.o *.a
endif

all: $(EXE)

.c.o:
	gcc -c $(CFLG) $<

.cpp.o:
	g++ -c $(CFLG) $<

$(EXE): hw6.o
	gcc $(CFLG) -o $@ $^ $(LIBS)

clean:
	$(CLEAN)

.PHONY: all clean