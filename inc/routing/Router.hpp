#ifndef ROUTER_HANDLER_HPP
#define ROUTER_HANDLER_HPP

#include <string>
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"


class Router
{
private:

	//------------------ ATTRIBUTES ----------------------//


	//--------------- MEMBER FUNCTIONS -------------------//


public:

	//----------------- CONSTRUCTORS ---------------------//

	Router();

	//----------------- DESTRUCTOR -----------------------//

	~Router();

	//------------------- OPERATORS ----------------------//

	Router&			operator=(const Router &other);

	//--------------- MEMBER FUNCTION --------------------//

	// UTILS

	bool			isDirectory(const std::string& path) const;
	bool			isValidFilePath(const std::string& path) const;
	std::string		replaceRoutePathByRootDirectory(const std::string& url, const Route* route) const;
	std::string		replaceRoutePathByUploadDirectory(const std::string& url, const Route* route) const;

	// File path resolution methods

	const Route*	findMatchingRoute(const std::string& requestPath, const ServerConf& conf) const;
	void			routeRequest(Request* request, Response* response);
	void			validateRequest(Request* request, Response* response);
	std::string		routeFilePathToGet(const std::string& url, const Route* route) const;
	std::string		routeFilePathToGetAsDirectory(std::string routedPath, const Route* route) const;
	std::string		routeFilePathToPost(const std::string& url, const Route* route) const;
};

#endif
