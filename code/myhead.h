#ifndef	my_head
#define	my_head

#include <mysql/mysql.h>
#include <assert.h>
void	 err_dump(const char *, ...);
void	 err_msg(const char *, ...);
void	 err_quit(const char *, ...);
void	 err_ret(const char *, ...);
void	 err_sys(const char *, ...);
#endif //my_head