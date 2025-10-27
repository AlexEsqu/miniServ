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
	Router(const Router& other);

	//----------------- DESTRUCTOR -----------------------//

	~Router();

	//------------------- OPERATORS ----------------------//

	Router&			operator=(const Router &other);

	//--------------- MEMBER FUNCTION --------------------//

	// UTILS

	static bool			isDirectory(const std::string& path);
	static bool			isExisting(const char* path);
	static bool			isAllowed(const char* path);

	static bool			isValidGetFilePath(const std::string& path) ;
	static bool			isRootPath(const std::string& uri) ;

	static bool			hasStartingSlash(const std::string& uri) ;
	static bool			hasTrailingSlash(const std::string& uri) ;

	static std::string	joinPaths(const std::string& base, const std::string& path) ;
	static std::string	replaceRoutePathByRootDirectory(const std::string& url, const Route* route) ;
	static std::string	replaceRoutePathByUploadDirectory(const std::string& url, const Route* route) ;

	// File path resolution methods

	static const Route*	findMatchingRoute(const std::string& requestPath, const ServerConf& conf) ;
	static void			routeRequest(Request* request, Response* response);
	static void			validateRequestWithRoute(Request* request, Response* response);
	static std::string	routeFilePathForGet(const std::string& url, const Route* route) ;
	static std::string	routeFilePathForGetAsDirectory(std::string routedPath, const Route* route) ;
	static std::string	routeFilePathForPost(const std::string& url, const Route* route) ;
};

#endif
