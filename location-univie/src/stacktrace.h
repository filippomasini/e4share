/**
 * \file
 * \brief This is a helper, that helps making sense out of a SIGSEGV
 *        or unhandled exception although it is not perfect ...
 *
 * \author Max Resch
 * \date 2012-11-04
 * 
 * 
 * \todo
 * 	- Check if addr2line is even accessible
 * 	- Only build this file when compiled with -g
 * 	- Look into threading
 */

#ifndef STACKTRACE_H_
#define STACKTRACE_H_

//#define threads

#include <stdio.h>
#include <unistd.h>
#ifdef __gnu_linux__
#include <execinfo.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#ifdef threads
#include <pthread.h>
#endif
#endif
#include <stdlib.h>
#include <signal.h>
#ifdef __GNU_LIBRARY__
#include <cxxabi.h>
#endif

#ifdef __cplusplus
#include <exception>
#include <typeinfo>

#ifdef CPLEX
// allow CPLEX exceptions to be parsed, only if cplex debug is defined
#include <ilconcert/ilosys.h>
#endif // CPLEX

extern "C"
{

#endif // __cplusplus

/**
 * \brief This function registers the handler.
 * 
 * \details
 * 	The handler should be registered on top of the
 * 	main function.
 * 
 * \code
 * int main(int argc, char** argv)
 * {
 * 	register_handler();
 * ...
 */
void register_handler ();

#ifdef __gnu_linux__
/**
 * \brief This function prints the stacktrace to stderr
 */
void __trace ();

//----------------------------------------------------------------------------//
//------------------------------ IMPLEMENTATION ------------------------------//
//----------------------------------------------------------------------------//

// maximum size of the stacktrace
#define __MAX_STACK 256

// maximum size of function names, file names, calls etc.
// basically all strings.
#define __MAX_FUNC 1024

#ifdef threads
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t bar;
#endif

pid_t gettid()
{
	return syscall(SYS_gettid);
}

/**
 * \brief Get the functionname, sourcefile name and line number corresponding to
 *        the address.
 * 
 * \details
 * 	This function uses the tool addr2line to determine the data, the program
 * 	must be compiled with -g to include these debug symbols.
 */
int __get_source_line (size_t addr, const char* cmd, char* file, const size_t file_len, char* func, size_t func_len, int* line)
{
	char buf[__MAX_FUNC] = "";
	char* p;

	snprintf(buf, __MAX_FUNC, "/usr/bin/addr2line -Cfe '%s' -i 0x%lx", cmd, addr);
	FILE* f = popen(buf, "r");

	if (f == NULL)
		return 1;

	fgets(buf, __MAX_FUNC, f);
	if (buf[0] != '?')
	{
		for (char* a = buf; *a != '\0'; ++a)
			if (*a == '\n')
			{
				*a = '\0';
				break;
			}
		strncpy(func, buf, func_len);
	}
	else
	{
		fclose(f);
		return 2;
	}

	fgets(buf, __MAX_FUNC, f);
	fclose(f);
	if (buf[0] != '?')
	{
		p = buf;
		while (*p != ':')
		{
			p++;
		}
		*p++ = '\0';
		*line = atol(p);
		strncpy(file, buf, file_len);
	}
	else
	{
		return 2;
	}
	return 0;
}

/**
 * \brief Resolve shared library function names according to memory map
 * 
 * \details
 * 	Functions in shared libs cannot be read statically by addr2line. So the
 * 	address of the function in the library must be calculated according to
 * 	the memory map in /proc/self/maps
 */
void __get_address_map (size_t addr, char* file, size_t file_len, char* func, size_t func_len, int* line)
{
	char buff[__MAX_FUNC] = "/proc/self/maps";
	//snprintf(buff, __MAX_FUNC, "/proc/self/maps");

	FILE *f = fopen(buff, "r");
	if (f == NULL)
		return;
	for (fgets(buff, __MAX_FUNC, f); buff[0] != '\0'; fgets(buff, __MAX_FUNC, f))
	{
		size_t begin = 0, end = 0; //, size = 0;

		sscanf(buff, "%lx-%lx", &begin, &end);

		if (addr >= begin && addr <= end)
		{
			char* p;
			for (p = buff + 1; *p != '\0'; ++p)
				if (*p == ' ' && *(p - 1) == ' ')
					break;
			if (*p != '\0')
			{
				for (; *p != '\0'; ++p)
					if (*p != ' ')
						break;
				if (*p != '\0')
				{
					p[strlen(p) - 1] = '\0';
					__get_source_line(addr - begin, p, file, file_len, func, func_len, line);
				}
			}
			fclose(f);
			return;
		}
	}
	fclose(f);
}
#endif // __gnu_linux__

#ifdef __cplusplus
void __exception_handling ()
{
	std::exception_ptr eptr = std::current_exception();
	try
	{
		std::rethrow_exception(eptr);
	}
	catch (std::exception& e)
	{
		const char* name = typeid(e).name();
#ifdef __GNU_LIBRARY__
		int status;
		char* name2 = abi::__cxa_demangle(name, 0, 0, &status);
		if (status == 0)
		{
			fprintf(stderr, "\nCaught unhandled exception of type %s\n", name2);
			free(name2);
		}
		else
#endif
			fprintf(stderr, "\nCaught unhandled exception of type %s: \n", name);
		fprintf(stderr, "%s\n", e.what());
	}
#ifdef CPLEX
	catch (IloException& e)
	{
		fprintf(stderr, "\nCaught unhandled IloException. what():\n");
		fprintf(stderr, "%s\n", e.getMessage());
	}
#endif
	catch (...)
	{
		fprintf(stderr, "\nCaught unknown unhandled exception.\n");
	}
#ifdef __gnu_linux__
	__trace();
	_exit(6);
#else
	_exit(6);
#endif // __gnu_linux__
}
#endif // __cplusplus

#ifdef __gnu_linux__
void __trace ()
{
	void* address[__MAX_STACK];
	size_t addr_size = 0;
	addr_size = backtrace(address, __MAX_STACK);

	if (addr_size == 0)
	{
		fprintf(stderr, "\nInvalid stacktrace\n");
		return;
	}

	fprintf(stderr, "\nStacktrace:\n");

	//char** symbollist = backtrace_symbols(address, __MAX_STACK);
	char file[__MAX_FUNC] = "";
	char recent_file[__MAX_FUNC] = "";
	char function[__MAX_FUNC] = "";
	char this_file[__MAX_FUNC] = "";
	struct stat stat_buff;
	//size_t function_len = __MAX_FUNC;

	pid_t pid = getpid();
	snprintf(this_file, __MAX_FUNC, "/proc/%d/exe", pid);
	int ret = stat(this_file, &stat_buff);
	if (ret < 0)
		return;
	for (size_t i = 0; i < addr_size; i++)
	{
		size_t inst_pointer = (size_t) address[i];
		int line;
		int status = __get_source_line(inst_pointer, this_file, recent_file, __MAX_FUNC, function, __MAX_FUNC, &line);
		if (status == 2)
		{
			__get_address_map(inst_pointer, recent_file, __MAX_FUNC, function, __MAX_FUNC, &line);
			status = 0;
		}

		if (strncmp(recent_file, __FILE__, __MAX_FUNC) == 0)
			continue;

		if (strncmp(file, recent_file, __MAX_FUNC) != 0)
		{
			if (status == 0)
			{
				strncpy(file, recent_file, __MAX_FUNC);
			}
			fprintf(stderr, "In %s\n", file);
		}

		fprintf(stderr, "(%d)  %s: %d\n", gettid(), function, line);
	}
}
#endif // __gnu_linux__

void __handler_signal (int sig)
{
#ifdef __gnu_linux__
#ifdef threads
	pthread_mutex_lock(&mtx);
#endif // threads
	fprintf(stderr, "\nCaught signal:(%d) %s\n", gettid(), strsignal(sig));
	__trace();
#ifdef threads
	pthread_mutex_unlock(&mtx);
	pthread_barrier_wait(&bar);
#endif // threads
#else
	fprintf(stderr, "\nCaught signal: %s\n", strsignal(sig));
#endif // __gnu_linux__
	_exit(6);
}

void register_handler ()
{
#ifdef threads
	pthread_mutexattr_t mtx_attr;
	pthread_mutexattr_init(&mtx_attr);
	pthread_mutex_init(&mtx, &mtx_attr);
	pthread_barrierattr_t bar_attr;
	pthread_barrierattr_init(&bar_attr);
	pthread_barrier_init(&bar, &bar_attr, 4);
#endif
	signal(SIGSEGV, __handler_signal);
	signal(SIGABRT, __handler_signal);
	signal(SIGFPE, __handler_signal);
	signal(SIGILL, __handler_signal);
	//signal(SIGINT, __handler_signal);
#ifdef __cplusplus
	std::set_terminate(__exception_handling);
#endif // __cplusplus
}

#ifdef __cplusplus
}
 // extern "C"
#endif // __cplusplus
#endif // STACKTRACE_H_

