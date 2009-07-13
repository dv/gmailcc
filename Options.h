#ifndef OPTIONS_H_
#define OPTIONS_H_

#include "boost/program_options.hpp"

namespace bo = boost::program_options;

/*
 * Options:
 * - use symlinks or hardlinks
 * - backup all folders or just selected folders
 * - do not back up trash
 * - do not back up spam
 * - write metadata for courier-imapd, dovecot, ...
 */

class Options
{
public:
	Options();
	virtual ~Options();

private:
	 
};

#endif /*OPTIONS_H_*/
