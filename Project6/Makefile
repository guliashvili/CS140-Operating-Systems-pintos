
files = main.c
files += map_entry.h
files += server.h
files += server.c
files += processor.c
files += processor.h
http_server: $(files)
	gcc -std=gnu11 $(files) -o http_server -O2 -march=native -Wextra -Wnull-dereference -fsanitize=bounds-strict -Winit-self -Wuninitialized -Wmisleading-indentation  -Wmissing-braces -Wmissing-include-dirs -Wsequence-point -Wunused -Wundef -fopenmp -pthread

.PHONY: clean
clean:
	@rm -f http_server
