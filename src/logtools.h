#ifndef LOGTOOLS_H
#define LOGTOOLS_H

#include <log.h>

namespace tools {
//Pairs of "lock" and "unlock" quick accesors: tools::error()<<"my stuff "<<errno<<tools::endl();
log& error();
log& warning();
log& info();
lop endl();
}

#endif
