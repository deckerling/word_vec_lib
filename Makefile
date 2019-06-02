# Makefile to compile an example program showing some of the benefits of
# "word_vec_lib".

CFLAGS := -g -Wall
SRCS := $(wildcard word_vec_lib/*.cc word_vec_lib/*.h)

example_program: $(SRCS)
	g++ example.cc $(SRCS) -o example/example $(CFLAGS)

clean:
	rm -rf example_program
