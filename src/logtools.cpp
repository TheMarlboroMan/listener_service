#include "logtools.h"

using namespace tools;

log& tools::error(tools::log& srvlog) {
	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::error;
	return srvlog;
}

log& tools::warning(tools::log& srvlog) {
	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::warning;
	return srvlog;
}

log& tools::info(tools::log& srvlog) {

	srvlog<<tools::lop::lock<<tools::ltime::datetime<<tools::lin::info;
	return srvlog;
}

lop tools::endl(tools::log& srvlog) {
	srvlog<<std::endl<<lop::unlock;
	return lop::unlock;
}
