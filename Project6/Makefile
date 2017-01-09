
files = main.c

http_server: $(files)
	gcc -std=gnu11 $(files) -o http_server -O2 -march=native -Wextra -Wnull-dereference -fsanitize=bounds-strict

.PHONY: clean
clean:
	@rm -f http_server
