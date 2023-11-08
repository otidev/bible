all: compile run

OBJS = $(patsubst src/%.c,obj/%.o,$(wildcard src/*.c))

obj/%.o: src/%.c
	gcc $^ -c -I src/include -I lib/include -Wall --std=c99 -o $@

compile: $(OBJS)
	gcc $^ -I src/include -I lib/include -Wall -L lib -lSDL2main -lSDL2 -lSDL2_mixer.dll -lSDL2_ttf.dll -lrr --std=c99 -o bin/bible.exe

run: bin/bible.exe
	./$^

reset: clean all

clean:
	rm bin/*.exe obj/*.o
