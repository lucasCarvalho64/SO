all: folders server client
server: monitor
client: tracer
folders:
	@mkdir -p src fifos include data
monitor.o: ./src/*.c
	gcc -std=gnu11 -Wall -Wextra -pedantic-errors -O -c src/*.c -lm
monitor: ./src/*.c
	gcc -o monitor ./src/*.c
-o tracer.o: src/*.c
	gcc -std=gnu11 -Wall -Wextra -pedantic-errors -O -c src/*.c -lm
tracer: ./src/*.c
	gcc -o tracer ./src/*.c
clean:
	rm -f fifos/* data/* tracer monitor
