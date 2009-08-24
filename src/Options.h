/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <string>

#include <boost/program_options.hpp>

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
	Options(int argc, char* argv[]);
	virtual ~Options();
	
	void show_help();
	
	bool incomplete();
	bool get_help();
	bool get_version();
	int get_loglevel();
	std::string get_logfile();
	std::string get_maildir_path();
	std::string get_username();
	std::string get_password();
	bool silent();

private:
	bo::variables_map* vm;
	bo::options_description* generic;
	bo::options_description* config;
	bo::options_description* cmdline_options;	
	
	void load_config_file(const char* filename);
};

#endif /*OPTIONS_H_*/
