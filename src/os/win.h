/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _WIN_H_
#define _WIN_H_

//
// windows stl hacks
//
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
//#define _HAS_ITERATOR_DEBUGGING 0


#include <time.h>
#include <io.h>

//
// localtime_r hack
//
inline struct tm*
localtime_r (const time_t *clock, struct tm *result) {
		if (!clock || !result)
			return NULL;

	// there is no localtime_r() on Windows, so for now
	// we just call localtime() and deep copy the result.
	struct tm* _ptr = localtime(clock);
		if (NULL == _ptr)
			return NULL;
	result->tm_sec   = _ptr->tm_sec;    /* seconds */
	result->tm_min   = _ptr->tm_min;    /* minutes */
	result->tm_hour  = _ptr->tm_hour;   /* hours */
	result->tm_mday  = _ptr->tm_mday;   /* day of the month */
	result->tm_mon   = _ptr->tm_mon;    /* month */
	result->tm_year  = _ptr->tm_year;   /* year */
	result->tm_wday  = _ptr->tm_wday;   /* day of the week */
	result->tm_yday  = _ptr->tm_yday;   /* day in the year */
	result->tm_isdst = _ptr->tm_isdst;  /* daylight saving time */	
	return result;
}

//
// S_ISREG hack
//
#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif

//
// ftruncate hack
//
#define ftruncate(a,b)	_chsize((a),(b))

//
// snprintf
//
#define snprintf _snprintf

#endif // _WIN_H_
