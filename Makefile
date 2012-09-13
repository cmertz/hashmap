CC_FLAGS = -o3 -Wall -pedantic -std=c99

.PHONY: clean doxygen

all: release

release:
	$(CC) $(CC_FLAGS) -DNDEBUG -c hashmap.c -o hashmap.o
	$(CC) $(CC_FLAGS) -DNDEBUG -c bobhash.c -o bobhash.o

debug:
	$(CC) $(CC_FLAGS) -g -c hashmap.c -o hashmap_debug.o
	$(CC) $(CC_FLAGS) -g -c bobhash.c -o bobhash_debug.o
	
clean:
	@rm -rf *.o
