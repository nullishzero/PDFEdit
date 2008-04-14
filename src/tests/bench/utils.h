#ifndef _BENCH_UTILS_H_
#define _BENCH_UTILS_H_

#include <kernel/static.h>
#include <kernel/cpdf.h>
#include <sys/time.h>
#include <time.h>
#include <boost/shared_ptr.hpp>
#include <limits.h>


extern const char * file_name;

int parse_cmd_line(int argc, char ** argv);

// redefine if something different than gettimeofday should
// be used for time measuring
typedef struct timeval time_stamp_t;
double time_diff(time_stamp_t &start, time_stamp_t &end);
// gets current time stamp - pointer to timeval struct
#define get_time_stamp(val)	gettimeofday(val, NULL)

struct result
{
	double max_time;
	double min_time;
	double sum_time;
	unsigned count;
	const char * name;
	bool valid;
};

#define DEFINE_RESULTS(var, name) struct result var = {0,LONG_MAX,0,0,name,false}

void update_result(double time, struct result & result);
void print_results(struct result ** results);


// TODO remove when CPdf::getInstance is reworked to return shared_ptr
struct pdf_deleter
{
	void operator() (pdfobjects::CPdf* p)
	{assert (p); p->close();}
};

static inline boost::shared_ptr<pdfobjects::CPdf> open_file(
		const char * name, 
		pdfobjects::CPdf::OpenMode mode = pdfobjects::CPdf::Advanced)
{
	return boost::shared_ptr<pdfobjects::CPdf>(pdfobjects::CPdf::getInstance(name, mode), pdf_deleter());
}

#endif

