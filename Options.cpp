#include "Options.h"

#include <iostream>


Options::Options(int argc, char* argv[])
{
	desc = new bo::options_description("Allowed options");
	vm = new bo::variables_map();
	
	desc->add_options()
		("help", "produce help message")
		("version", "show version information")
		("loglevel", bo::value<int>()->default_value(2), "set loglevel")
		("logfile", bo::value<std::string>(), "file to log to")
		("username,u", bo::value<std::string>(), "username to be used to log in. If you're using GMail, you can omit @gmail.com. If you're using Google Apps, please add your domain, e.g. john@smith.com")
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
	return !(vm->count("username") && vm->count("password") && vm->count("maildir"));	
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
		return NULL;
}

std::string Options::get_maildir_path()
{
	return (*vm)["maildir"].as<std::string>();
}

std::string Options::get_username()
{
	return (*vm)["username"].as<std::string>();
}

std::string Options::get_password()
{
	return (*vm)["password"].as<std::string>();
}
