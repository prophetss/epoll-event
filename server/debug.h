#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <errno.h>

#ifdef _DEBUG

#define info_display(...) fprintf(stdout, __VA_ARGS__)

/*一般错误打印*/
#define _error_display(...)	fprintf(stderr, __VA_ARGS__)

/*系统错误打印*/
#define _exit_throw(...) do {                                             		\
    _error_display("Error defined at %s, line %i : \n", __FILE__, __LINE__); 	\
    _error_display("Errno : %d, msg : %s\n", errno, strerror(errno));           \
    _error_display("%s\n", __VA_ARGS__);                                        \
    exit(errno);                                                           		\
} while(0)

#else

#define info_display(...)

#define _error_display(...)

#define _exit_throw(...) do {                                             		\
    _error_display("Errno : %d, msg : %s\n", errno, strerror(errno));           \
    exit(errno);                                                           		\
} while(0)

#endif /*_DEBUG*/


#endif /*!_DEBUG_H_*/