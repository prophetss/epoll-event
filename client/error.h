#ifndef _ERROR_H_
#define _ERROR_H_


#include <stdio.h>
#include <errno.h>


#ifdef _DEBUG


#define info_display(...)	fprintf(stdout, __VA_ARGS__)

/*一般错误打印*/
#define error_display(...)	fprintf(stderr, __VA_ARGS__)

/*致命错误打印*/
#define exit_throw(...)	do {                                             	    \
    error_display("Error defined at %s, line %i : \n", __FILE__, __LINE__); 	\
    error_display(__VA_ARGS__);                                         		\
    exit(0);                                                           		    \
} while(0)

/*系统错误打印*/
#define sys_exit_throw(...)	do {                                            	\
    error_display("Errno : %d, msg : %s\n", errno, strerror(errno));            \
    exit_throw(__VA_ARGS__);                                                    \
} while(0)

#define _STR(X) _VAL(X)

#define _VAL(X) #X

void _err_throw(char *mesg)
{
	fputs(mesg, stderr);
    fputs("----------assertion failed \n",stderr);  
    abort();
}

#define general_assert(expression) (void)((!!(expression)) || (_err_throw(__FILE__":"_STR(__LINE__)" " #expression), 0))


#else


#define info_display(...)

#define error_display(...)

#define exit_throw(...)	 do { exit(0); } while (0)																

#define sys_exit_throw(...)	do {              								    \
   fprintf(stderr, "Errno : %d, msg : %s\n", errno, strerror(errno));     		\
   exit(errno);             													\
} while (0)

#define general_assert(expression, message)


#endif /*_DEBUG*/


#endif /*!_ERROR_H_*/