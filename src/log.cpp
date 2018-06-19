#include "log.h"

using namespace tools;

//!Converts integer to lcut values.

lcut	log::int_to_lcut(int v)
{
	if(v < 0 || v > 3)
	{
		throw std::runtime_error("Invalid cut level "+std::to_string(v)+" specified for int_to_lcut");
	}

	switch(v)
	{
		case error: 	return lcut::error; break;
		case warning: 	return lcut::warning; break;
		case info: return lcut::info; break;
		case all: 	return lcut::all; break;
	}	

	//Compiler: just shut up.
	return lcut::all;
}

log::log()
	:s(), entry_level(levels::all), min_level(levels::all), 
	active(false)
{

}

log::log(const char * filename)
	:s(), entry_level(levels::all), min_level(levels::all), 
	active(true)
{
	init(filename);
}

//!Class destructor.

//!Adds a timestamp.

log::~log()
{
	if(is_usable())
	{
		(*this)<<"Session ends "<<ltime::datetime<<std::endl;
		s.close();
	}
}

//!Activates the log.

//!Adds a timestamp.

void log::activate() 
{
	active=true;
	if(is_usable())
	{
		(*this)<<"Session starts "<<ltime::datetime<<std::endl;
	}
}

//!Dectivates the log.

//!Adds a timestamp.

void log::deactivate() 
{
	if(is_usable())
	{
		(*this)<<"Session ends "<<ltime::datetime<<std::endl;
	}

	active=false;
}

//!Inits the log, opening the file.

//!Adds a timestamp.

void log::init(const char * filename)
{
	s.open(filename);

	if(is_usable())
	{
		(*this)<<"Session starts "<<ltime::datetime<<std::endl;
	}
}

//!Locks or unlocks the log for multithreading environments.

//!In multithreaded environments, the log must be locked before writing
//!and unlocked when done.

log& log::operator<<(lop op)
{
	switch(op)
	{
		case lop::lock: 	mtx.lock(); break;
		case lop::unlock: 	mtx.unlock(); break;
	}
	return *this;
}

//!Sets the level of input.

//!Additionally adds the level to the output.

log& log::operator<<(lin lvl)
{
	switch(lvl)
	{
		case lin::error:
			s<<"[ERROR] ";
			entry_level=error; 
		break;
		case lin::warning:
			s<<"[WARNING] ";
			entry_level=warning; 
		break;
		case lin::info:
			s<<"[INFO] ";
			entry_level=info; 
		break;
	}
	return *this;
}

//!Sets the minimum level. 

//!Error will show only errors, warning will show errors and warnings, etc.

log& log::operator<<(lcut lvl)
{
	switch(lvl)
	{
		case lcut::error: 	min_level=error; break;
		case lcut::warning:	min_level=warning; break;
		case lcut::info:	min_level=info; break;
		case lcut::all:		min_level=all; break;
	}
	return *this;
}

//!Adds the time, date or both to the output.

log& log::operator<<(ltime v)
{
	if(is_usable())
 	{
		switch(v)
		{
			case ltime::time:
				operator<<("[")<<time()<<"] ";
			break;
			case ltime::date:
				operator<<("[")<<date()<<"] ";
			break;
			case ltime::datetime:
				operator<<("[")<<date()<<" "<<time()<<"] ";
			break;
		}
	}
	
	return *this;
}

//!Creates a string with the system time.

std::string log::time() const
{
	char * t=new char[14];
	memset(t, '\0', 14);
	time_t tiempo=std::time(nullptr);
	strftime(t, 14, "%H:%M:%S", localtime(&tiempo));
	std::string res(t);
	delete [] t ;
	return res;
}

//!Creates a string with the system date.

std::string log::date() const
{
	char * t=new char[14];
	memset(t, '\0', 14);
	time_t tiempo=std::time(nullptr);
	strftime(t, 14, "%F", localtime(&tiempo));
	std::string res(t);
	delete [] t ;
	return res;
}

//!Accepts std::ostream modificators.

log& log::operator<<(std::ostream& ( *pf )(std::ostream&))
{
	if(is_usable() && check_levels())
	{
		s<<pf;
	}
	return *this;
}

//!Accepts std::ios modificators.

log& log::operator<<(std::ios& ( *pf )(std::ios&))
{
	if(is_usable() && check_levels())
	{
		s<<pf;
	}
	return *this;
}

//!Accepts std::ios_base modificators.

log& log::operator<<(std::ios_base& ( *pf )(std::ios_base&))
{
	if(is_usable() && check_levels())
	{
		s<<pf;
	}
	return *this;
}
