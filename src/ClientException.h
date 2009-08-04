/*
 * Copyright (c) 2009 David Verhasselt <david@crowdway.com>
 * Licensed under The MIT License - http://gmailcc.crowdway.com/license.txt
 */

#ifndef CLIENTEXCEPTION_H_
#define CLIENTEXCEPTION_H_

#include <exception>

class Client;

class ClientException: public std::exception
{
public:
	ClientException(Client *client): client(client) {};
private:
	Client *client;
};

class AuthClientException: public ClientException
{
public:
	AuthClientException(Client *client): ClientException(client) {};	
};

class InvalidAuthClientException: public AuthClientException
{
public:
	InvalidAuthClientException(Client *client): AuthClientException(client) {};
};

class WebAuthClientException: public AuthClientException
{
public:
	WebAuthClientException(Client *client): AuthClientException(client) {};
};

#endif /*CLIENTEXCEPTION_H_*/
