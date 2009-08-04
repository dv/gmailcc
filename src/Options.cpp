/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#include "Options.h"

#include <iostream>


Options::Options(int argc, char* argv[])
{
	desc = new bo::options_description("Allowed options");
	vm = new bo::variables_map();
	
	desc->add_options()
		("help,?,h", "produce help message")
		("version,v", "show version information")
		("loglevel", bo::value<int>()->default_value(2), "set loglevel")
		("logfile", bo::value<std::string>(), "file to log to")
		("username,u", bo::value<std::string>(), "username to be used to log in. If you're using Gmail, you can omit @gmail.com. If you're using Google Apps, please add your domain, e.g. john@smith.com")
		("password,p", bo::value<std::string>(), "password to log in with")
		("maildir,d", bo::value<std::string>(), "folder to backup your mails to")
	;
	
	bo::store(bo::parse_command_line(argc, argv, *desc), *vm);
	bo::notify(*vm);
}

Options::~Options()
{
	delete vm;
	delete desc;
}


void Options::show_help()
{
	std::cout << *desc << "\n";
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
