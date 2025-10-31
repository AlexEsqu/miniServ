#include "Router.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Router::Router()
{
}

Router::Router(const Router &)
{
}

//--------------------------- DESTRUCTORS -----------------------------------//

Router::~Router()
{
}

//---------------------------- OPERATORS ------------------------------------//

Router &Router::operator=(const Router &other)
{
	if (&other != this)
	{
		;
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

//---------------------------- SETTERS --------------------------------------//

//------------------------ MEMBER FUNCTIONS ---------------------------------//

std::string Router::findFileInDirectoryWithExtension(std::string directory, std::string filename) // EX "/upload", "picture" => picture.jpeg
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(directory.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			char *entChar = ent->d_name;
			std::string ent(entChar);

			size_t dotPos = ent.find(".");
			if (dotPos != std::string::npos)
			{
				std::string fileWithoutExtension = ent.substr(0, dotPos);
				if (fileWithoutExtension == filename)
				{
					std::string filePath = Router::joinPaths(directory, ent);
					closedir(dir);
					return filePath;
				}
			}
		}
		closedir(dir);
	}
	else
	{
		std::cerr << ERROR_FORMAT("Could not open directory") << std::endl;
		return std::string();
	}
	return std::string();
}

const Route *Router::findMatchingRoute(const std::string &requestPath, const ServerConf &conf)
{
	const Route *result = NULL;
	size_t lenMatch = 0;

	for (size_t i = 0; i < conf.getRoutes().size(); i++)
	{
		const Route &route = conf.getRoutes()[i];

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
		verboseLog("404: No route for: " + requestPath);
		return NULL;
	}
	std::cout << "[" << result->getURLPath() << "]:[" << result->getRootDirectory() << "]";
	return result;
}

void Router::routeRequest(Request *request, Response *response)
{
	const Route *route = request->getRoute();
	std::string requestedURL;
	if (route->getRootDirectory() == route->getUploadDirectory())
	{
		requestedURL = route->getURLPath() + "/" + request->getStringSessionId() + "_";
	}
	else
		requestedURL = request->getRequestedURL();

	validateRequestWithRoute(request, response);
	if (response->hasError())
		return;
	std::string path;
	if (request->getMethodCode() == GET || request->getMethodCode() == HEAD)
	{
		std::string filename = request->getRequestedURL().substr(requestedURL.find_last_of("/") + 1);
		if (route->getRootDirectory() == route->getUploadDirectory())
			requestedURL.append(filename);
		path = routeFilePathForGet(requestedURL, route);
		if (path.empty())
			response->setError(NOT_FOUND);
		else if (!isAllowedRead(path.c_str()))
			response->setError(FORBIDDEN);
	}
	else
	{
		std::string filename = request->getRequestedURL().substr(requestedURL.find_last_of("/") + 1);
		if (route->getRootDirectory() == route->getUploadDirectory())
			requestedURL.append(filename);
		path = routeFilePathForPost(requestedURL, route);
		if (!path.empty() && !isAllowedWrite(path.c_str()))
			response->setError(FORBIDDEN);
	}
	response->setRoutedUrl(path);
}

std::string Router::routeFilePathForGet(const std::string &url, const Route *route)
{
	if (route == NULL)
		return "";

	std::string routedURL = replaceRoutePathByRootDirectory(url, route);
	std::string directory = routedURL.substr(0, routedURL.find_last_of("/"));
	std::string filename = routedURL.substr(routedURL.find_last_of("/") + 1);
	// if the file exist, it is the valid routed path
	if (isValidGetFilePath(routedURL))
		return routedURL;
	else if (!findFileInDirectoryWithExtension(directory, filename).empty()) // test by looking for filename with an extension in directory
		return (findFileInDirectoryWithExtension(directory, filename));

	// else if might be a directory, so check if default file or auto index exist
	return routeFilePathForGetAsDirectory(routedURL, route);
}

std::string Router::routeFilePathForGetAsDirectory(std::string routedURL, const Route *route)
{
	// a directory uri is acceptable even without a trailing '/' so adding it if missing
	if (!routedURL.empty() && !hasTrailingSlash(routedURL))
		routedURL += '/';

	// if it is not a file and not a directory tho, GET is impossible
	if (!isDirectory(routedURL))
		return "";

	// check if any default file exist, and can be valid address
	for (size_t i = 0; i < route->getDefaultFiles().size(); i++)
	{
		std::string possiblePath = joinPaths(routedURL, route->getDefaultFiles()[i]);

		if (isValidGetFilePath(possiblePath))
			return possiblePath;
	}

	// as last resort, check if autoindex, which allows a directory as uri
	if (route->isAutoIndex())
		return routedURL;
	else
		return "";
}

std::string Router::routeFilePathForPost(const std::string &url, const Route *route)
{
	if (route == NULL)
		return "";

	std::string routedURL = replaceRoutePathByUploadDirectory(url, route);

	return (routedURL);
}

//-------------------------- UTILS -----------------------------------//

bool Router::isDirectory(const std::string &path)
{
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) != 0)
		return false;
	return S_ISDIR(path_stat.st_mode);
}

bool Router::isExisting(const char *path)
{
	if (!path)
		return (false);
	struct stat path_stat;
	return (stat(path, &path_stat) == 0);
}

bool			Router::isValidGetFilePath(const std::string& path)
{
	std::ifstream in(path.c_str(), std::ios::binary);
	if (in.is_open() && !isDirectory(path))
		return true;
	return false;
}

bool			Router::isAllowedRead(const char* path)
{
	if (!path)
		return (false);
	return (access(path, R_OK) == 0);
}

bool			Router::isAllowedWrite(const char* path)
{
	if (!path)
		return (false);

	if (access(path, F_OK) != 0)
		return (true);

	return (access(path, W_OK) == 0);
}

bool			Router::isAllowedExecute(const char* path)
{
	if (!path)
		return (false);
	return (access(path, X_OK) == 0);
}

bool Router::isRootPath(const std::string &uri)
{
	return uri == "/" || uri.empty();
}

bool Router::hasStartingSlash(const std::string &uri)
{
	if (uri.empty())
		return (false);
	return (uri[0] == '/');
}

bool Router::hasTrailingSlash(const std::string &uri)
{
	if (uri.empty())
		return (false);
	return (uri[uri.size() - 1] == '/');
}

std::string Router::joinPaths(const std::string &base, const std::string &path)
{
	if (base.empty())
		return path;
	if (path.empty())
		return base;

	if (hasTrailingSlash(base) && hasStartingSlash(path))
		return base + path.substr(1);
	else if (!hasTrailingSlash(base) && !hasStartingSlash(path))
		return base + '/' + path;
	else
		return base + path;
}

std::string Router::replaceRoutePathByRootDirectory(const std::string &url, const Route *route)
{
	std::string result = url;

	result.erase(0, route->getURLPath().size());
	result = joinPaths(route->getRootDirectory(), result);

	return result;
}

std::string Router::replaceRoutePathByUploadDirectory(const std::string &url, const Route *route)
{
	std::string path = url;

	path.erase(0, route->getURLPath().size());
	if (hasStartingSlash(path))
		path = path.substr(1);

	std::string result = joinPaths(route->getUploadDirectory(), path);

	return result;
}

// checks for the existence of a route, on which many following function depend
void Router::validateRequestWithRoute(Request *request, Response *response)
{
	if (request->getRoute() == NULL)
		response->setError(NOT_FOUND);

	else if (!request->getRoute()->isAllowedMethod(request->getMethodAsString()))
		response->setError(METHOD_NOT_ALLOWED);
}
