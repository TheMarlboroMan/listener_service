#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstring>
#include <mutex>

namespace tools
{

//!Types of input.
enum class lin{error, warning, info};
//!Input cut.
enum class lcut{error, warning, info, all};
//!Locking for threads.
enum class lop{lock, unlock};
//!Time values.
enum class ltime{date, time, datetime};

//!A basic log to file.

//!The log allows for four levels (all, info, warning, error) of input, but
//!can be cut at any level to produce smaller files. It also has locking
//!capabilities for multithreading and date/time formats.

class log
{
	public:

	//This enum just bitwises the levels.
	enum levels{all=0, info=1, warning=2, error=3};


	lcut int_to_lcut(int v);
	log();
	log(const char * filename);
	~log();

	void activate();
	void deactivate();
	void init(const char * filename);

	//!Inserts data into the log.
	template <class X> log& operator<<(const X &input)
	{
		if(is_usable() && check_levels())
		{
			s<<input;
			s.flush();
		}
		return *this;
	}

	log& operator<<(lop);
	log& operator<<(lin);
	log& operator<<(lcut);
	log& operator<<(ltime);
	log& operator<<(std::ostream& ( *pf )(std::ostream&));
	log& operator<<(std::ios& ( *pf )(std::ios&));
	log& operator<<(std::ios_base& ( *pf )(std::ios_base&));

	private:

	//!Checks if the current level can write.
	bool					check_levels() {return entry_level >= min_level;}
	//!Indicates if the file is open and the log is active.
	bool 					is_usable() {return active && s.is_open();}
	std::string				date() const;
	std::string				time() const;

	std::mutex				mtx;
	std::ofstream 				s;	
	int 					entry_level,
						min_level;
	bool 					active;
};

}
#endif
