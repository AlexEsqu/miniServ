#include "Router.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Router::Router()
{

}

Router::Router(const Router&)
{

}

//--------------------------- DESTRUCTORS -----------------------------------//

Router::~Router()
{

}

//---------------------------- OPERATORS ------------------------------------//

Router&		Router::operator=(const Router &other)
{

}

//---------------------------- GUETTERS -------------------------------------//

//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//

const Route*	Router::findMatchingRoute(const std::string& requestPath, const ServerConf& conf) const
{
	const Route*		result = NULL;
	size_t				lenMatch = 0;

	for (size_t i = 0; i < conf.getRoutes().size(); i++)
	{
		const Route&	route = conf.getRoutes()[i];

		if (route.isPathMatch(requestPath))
		{
			std::string routePath = route.getURLPath();
			if (routePath.size() > lenMatch)
			{
				lenMatch = routePath.size();
				result = &route;
			}
		}
	}

	if (result == NULL)
	{
		std::cout << "404: No route for: " << requestPath << std::endl;
		return NULL;
	}
	std::cout << "Route selected: " << result->getURLPath() << std::endl;
	return result;
}

void			Router::routeRequest(Request* request, Response* response)
{
	const Route* route = request->getRoute();
	std::string requestedURL = request->getRequestedURL();

	validateRequest(request, response);
	if (response->hasError())
		return;

	std::string path;
	if (request->getMethodCode() == GET || request->getMethodCode() == HEAD)
	{
		path = routeFilePathToGet(requestedURL, route);
	}
	else
	{
		path = replaceRoutePathByUploadDirectory(requestedURL, route);
	}
	if (path.empty())
		response->setError(NOT_FOUND);
	else
		response->setRoutedUrl(path);
}

std::string		Router::routeFilePathToGet(const std::string& url, const Route* route) const
{
	if (route == NULL)
		return "";

	std::string	routedURL = replaceRoutePathByRootDirectory(url, route);

	// if the file exist, it is the valid routed path
	if (isValidFilePath(routedURL))
		return routedURL;

	// else if might be a directory, so check if default file or auto index exist
	return routeFilePathToGetAsDirectory(routedURL, route);
}

std::string		Router::routeFilePathToGetAsDirectory(std::string routedURL, const Route* route) const
{
	// a directory uri is acceptable even without a trailing '/' so adding it if missing
	if (!routedURL.empty() && routedURL.back() != '/')
		routedURL += '/';

	// if it is not a directory tho, GET is impossible
	if (!isDirectory(routedURL))
		return "";

	// check if any default file exist, and can be valid address
	for (size_t i = 0; i < route->getDefaultFiles().size(); i++)
	{
		std::string possiblePath = joinPaths(routedURL, route->getDefaultFiles()[i]);

		if (isValidFilePath(possiblePath))
			return possiblePath;
	}

	// as last resort, check if autoindex, which allows a directory as uri
	if (route->isAutoIndex())
		return routedURL;
	else
		return "";
}

std::string		Router::routeFilePathToPost(const std::string& url, const Route* route) const
{
	if (route == NULL)
		return "";

	std::string	routedURL = replaceRoutePathByUploadDirectory(url, route);
}



//-------------------------- UTILS -----------------------------------//

bool			Router::isDirectory(const std::string& path) const
{
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) != 0)
		return false;
	return S_ISDIR(path_stat.st_mode);
}

bool			Router::isValidFilePath(const std::string& path) const
{
	std::ifstream in(path.c_str(), std::ios::binary);
	if (in.is_open() && !isDirectory(path))
		return true;
	return false;
}

bool			isRootPath(const std::string& uri)
{
	return uri == "/" || uri.empty();
}

std::string		joinPaths(const std::string& base, const std::string& path)
{
	if (base.empty())
		return path;
	if (path.empty())
		return base;

	bool baseEndsWithSlash = base.back() == '/';
	bool pathStartsWithSlash = path.front() == '/';

	if (baseEndsWithSlash && pathStartsWithSlash)
		return base + path.substr(1);
	else if (!baseEndsWithSlash && !pathStartsWithSlash)
		return base + '/' + path;
	else
		return base + path;
}

std::string		Router::replaceRoutePathByRootDirectory(const std::string& url, const Route* route) const
{
	std::string	result = url;

	result.replace(0, route->getURLPath().size(), route->getRootDirectory());

	return result;
}

std::string		Router::replaceRoutePathByUploadDirectory(const std::string& url, const Route* route) const
{
	std::string	result = url;

	result.replace(0, route->getURLPath().size(), route->getUploadDirectory());

	return result;
}

// checks for the existence of a route, on which many following function depend
void			Router::validateRequest(Request* request, Response* response)
{
	const Route* route = request->getRoute();
	std::string requestedURL = request->getRequestedURL();

	if (route == NULL)
		response->setError(NOT_FOUND);

	else if (!route->isAllowedMethod(request->getMethodAsString()))
		response->setError(METHOD_NOT_ALLOWED);
}
