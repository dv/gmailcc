#ifndef CLIENTEXCEPTION_H_
#define CLIENTEXCEPTION_H_

#include <exception>

class Client;

class ClientException: public std::exception
{
public:
	ClientException(Client *client);
private:
	Client *client;
};

class AuthClientException: public ClientException
{
public:
	AuthClientException(Client *client): ClientException(client) {};	
};

#endif /*CLIENTEXCEPTION_H_*/
