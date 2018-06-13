HASH_SRC = xxhash.c hashtable.c test_hash.c
EPOLL_SERVER_SRC = xxhash.h xxhash.c hashtable.h hashtable.c epoll_event.h epoll_event.c example_ep_server.c
DEBUG =	-O0 -Wall -Wextra -ggdb -DEPOLL_DEBUG
SAMPLE = tar -xjf sample.tar.bz2
SHELL_EXIST = $(shell if [ ! -e sample0.txt ]; then $(SAMPLE); fi;)

epoll_server_test_debug: $(EPOLL_SERVER_SRC)
	gcc $(DEBUG) -o $@ $^ 
	
epoll_server_test: $(EPOLL_SERVER_SRC)
	gcc -O2 -o $@ $^

hash_test: $(SRC)
	$(SHELL_EXIST)
	gcc -O2 -o $@ $^

hash_test_debug: $(SRC)
	$(SHELL_EXIST)
	gcc $(DEBUG) -o $@ $^
	
.PHONY: clean
clean:
	-rm hash_test epoll_server_test hash_test_debug -f
