/*
 * @Author       : cwd
 * @Date         : 2021-4-21
 * @Place  : hust
 * 涉及知识点：
 *      1：#define 里定义函数用do {}while (0)使其成为一个语句块
 */ 
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <assert.h>


#define Log_Debug(format, ...) do {Log_Base(0,format,)}