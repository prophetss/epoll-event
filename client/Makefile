client_test:client.c
	gcc -O2 -o $@ $^ -lpthread
	
client_test_debug:client.c
	gcc  -O0 -Wall -Wextra -ggdb -DEPOLL_DEBUG  -o $@ $^ -lpthread
	
.PHONY: clean
clean:
	-rm client_test client_test_debug -f
	