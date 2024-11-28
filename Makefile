all: test_heap

test_heap: heap_malloc.c test_heap_malloc.c
	gcc -Wall -pthread -o test_heap heap_malloc.c test_heap_malloc.c

run: test_heap
	./test_heap

clean:
	rm -f test_heap
