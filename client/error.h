#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <errno.h>


/*一般错误打印*/
#define _error_display(...)	fprintf(stderr, __VA_ARGS__)

/*系统错误打印*/
#define _exit_throw(...) do {                                             		\
    _error_display("Error defined at %s, line %i : \n", __FILE__, __LINE__); 	\
    _error_display("Errno : %d, msg : %s\n", errno, strerror(errno));           \
    _error_display("%s\n", __VA_ARGS__);                                        \
    exit(errno);                                                           		\
} while(0)


#endif /*!_ERROR_H_*/