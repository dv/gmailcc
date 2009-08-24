/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#include "Options.h"

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include "LogSink.h"


Options::Options(int argc, char* argv[])
{
	generic = new bo::options_description("Generic options");	
	generic->add_options()
		("help,?,h", "produce help message")
		("version,v", "show version information")
		("config,c", "set config file")
	;
	
	config = new bo::options_description("Configuration");
	config->add_options()
		("loglevel", bo::value<int>()->default_value(2), "set loglevel")
		("logfile", bo::value<std::string>(), "file to log to")
		("username,u", bo::value<std::string>(), "username to be used to log in. If you're using Gmail, you can omit @gmail.com. If you're using Google Apps, please add your domain, e.g. john@smith.com")
		("password,p", bo::value<std::string>(), "password to log in with")
		("maildir,d", bo::value<std::string>(), "folder to backup your mails to")
	;
	
	cmdline_options = new bo::options_description;
	cmdline_options->add(*generic);
	cmdline_options->add(*config);
	
	vm = new bo::variables_map();	
	bo::store(bo::parse_command_line(argc, argv, *cmdline_options), *vm);
	bo::notify(*vm);
	
	if (vm->count("config"))
	{
		if (bf::exists((*vm)["config"].as<std::string>()))
		{
			load_config_file((*vm)["config"].as<char*>());
		} else {
			Log::error << "Config-file " << (*vm)["config"].as<std::string>() << "doesn't exist." << Log::endl;
		}
	} else {
		std::string paths[2] = {"~/.gmailcc.conf", "/etc/gmailcc.conf"};
		
		for(int i = 0; i < 2; i++)
		{
			if (bf::exists(paths[i]))
			{
				load_config_file(paths[i].c_str());
				break;
			}
		}
	}
}

void Options::load_config_file(const char* filename)
{
	std::ifstream ifs(filename);
	bo::store(bo::parse_config_file(ifs, *config), *vm);
	bo::notify(*vm);
}

Options::~Options()
{
	delete vm;
	delete cmdline_options;
	delete config;
	delete generic;
}


void Options::show_help()
{
	std::cout << *cmdline_options << "\n";
}


bool Options::incomplete()
{	
	return (!(vm->count("username") && vm->count("password") && vm->count("maildir")));
}

bool Options::get_help()
{
	return vm->count("help");
}

bool Options::get_version()
{
	return vm->count("version");
}

int Options::get_loglevel()
{
	return (*vm)["loglevel"].as<int>();
}

char* Options::get_logfile()
{
	if (vm->count("logfile"))
		return (*vm)["logfile"].as<char*>();
	else
		return "";
}

std::string Options::get_maildir_path()
{
	if (vm->count("maildir"))
		return (*vm)["maildir"].as<std::string>();
	else
		return "";
}

std::string Options::get_username()
{
	if (vm->count("username"))
		return (*vm)["username"].as<std::string>();
	else
		return "";
}

std::string Options::get_password()
{
	if (vm->count("password"))
		return (*vm)["password"].as<std::string>();
	else
		return "";
}
