#ifndef LOGTOOLS_H
#define LOGTOOLS_H

#include "../log/log.h"

namespace tools {
//Pairs of "lock" and "unlock" quick accesors: tools::error()<<"my stuff "<<errno<<tools::endl();
log& error(tools::log& srvlog);
log& warning(tools::log& srvlog);
log& info(tools::log& srvlog);
log& debug(tools::log& srvlog);
lop endl(tools::log& srvlog);
}

#endif
