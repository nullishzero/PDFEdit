#include "utils.h"
const char *file_name;
int parse_cmd_line(int argc, char **argv)
{
	if(argc<2)
	{
		std::cerr << "Bad usage. Filename parameter expected" << std::endl;
		return 1;
	}

	// TODO use getopt instead and support several files
	file_name = argv[1];
	return 0;
	return 0;
}

// result is in miliseconds
double time_diff(time_stamp_t &start, time_stamp_t &end)
{
	time_stamp_t diff;
	diff.tv_sec = end.tv_sec - start.tv_sec;
	diff.tv_usec = end.tv_usec - start.tv_usec;
	if(start.tv_usec > end.tv_usec)
	{
		--diff.tv_sec;
		diff.tv_usec += 1000000;
	}

	return diff.tv_sec*1000 + (double)diff.tv_usec/(double)1000;
}

void update_result(double time, struct result & result)
{
	if(time >= result.max_time)
		result.max_time = time;
	if (time <= result.min_time)
		result.min_time = time;
	result.sum_time += time;
	++(result.count);
	result.valid = true;
}

void print_results(struct result ** results)
{
using namespace std;
	for(struct result **iter=results; *iter; ++iter)
	{
		struct result * curr = *iter;
		cout << curr->name;
		if(!curr->valid)
		{
			cout << ":NO_RESULTS" << endl;
			continue;
		}
		double avg = (double)curr->sum_time/(double)curr->count;
		cout << ":max="<<curr->max_time<<":min="
			<<curr->min_time<<":avg="<<avg<<":count="<<curr->count
			<<endl;
	}
}
