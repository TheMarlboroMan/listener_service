#include "logtools.h"

using namespace tools;

extern tools::log srvlog;

log& tools::error() {
	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::error;
	return srvlog;
}

log& tools::warning() {
	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::warning;
	return srvlog;
}

log& tools::info() {
	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::info;
	return srvlog;
}

lop tools::endl() {
	srvlog<<std::endl;
	return lop::unlock;
}
