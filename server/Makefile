HASHTABLE_DIR=./hashtable
EPOLL_SERVER_SRC = $(HASHTABLE_DIR)/xxhash.c $(HASHTABLE_DIR)/hashtable.c epoll_event.h epoll_event.c example_ep_server.c
DEBUG =	-O0 -Wall -Wextra -ggdb -DEPOLL_DEBUG

epoll_server_test:$(EPOLL_SERVER_SRC)
	gcc -I$(HASHTABLE_DIR) $^ -O2 -o $@
	
epoll_server_test_debug:$(EPOLL_SERVER_SRC)
	gcc -I$(HASHTABLE_DIR) $(DEBUG) $^ -o $@
	
.PHONY: clean
clean:
	-rm epoll_server_test epoll_server_test_debug -f
