#include "Route.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Route::Route()
{
	#ifdef DEBUG
		std::cout << "Route Generic Constructor called" << std::endl;
	#endif
}

Route::Route(const Route &copy)
{
	*this = copy;

	#ifdef DEBUG
		std::cout << "Route copy Constructor called" << std::endl;
	#endif
}

//--------------------------- DESTRUCTORS -----------------------------------//

Route::~Route()
{
	#ifdef DEBUG
		std::cout << "Route Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Route&	Route::operator=(const Route &other)
{
	if (this != &other)
	{
		this->_routedPath		= other._routedPath;
		this->_urlPath			= other._urlPath;
		this->_defaultFiles		= other._defaultFiles;
		this->_autoindex		= other._autoindex;
		this->_allowedCGI		= other._allowedCGI;
		this->_uploadDirectory	= other._uploadDirectory;
		this->_allowedMethods	= other._allowedMethods;
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

std::string Route::getRootDirectory() const
{
	return (this->_routedPath);
}

std::vector<std::string> Route::getDefaultFiles() const
{
	return (this->_defaultFiles);
}

bool Route::isAutoIndex() const
{
	return (this->_autoindex);
}

std::vector<std::string> Route::getAllowedCGI() const
{
	return (this->_allowedCGI);
}

std::string Route::getUploadDirectory() const
{
	return (this->_uploadDirectory);
}

std::string Route::getURLPath() const
{
	return (this->_urlPath);
}

const std::vector<Route>&	Route::getRoutes() const {
	return _nestedRoutes;
}

bool	Route::isPathMatch(const std::string& requestPath) const
{
	std::cout << "is it close to " << _urlPath << "\n";

	// // exact match: "= /exact"
	// if (_urlPath[0] == '=' && _urlPath.length() > 1) {
	// 	std::string exactPath = _urlPath.substr(2); // Skip "= "
	// 	return (requestPath == exactPath);
	// }

	// // regex match: "~ ^/api/.*$"
	// if (_urlPath[0] == '~' && _urlPath.length() > 1) {
	// 	std::string pattern = _urlPath.substr(2); // Skip "~ "
	// 	return matchesRegex(requestPath, pattern);
	// }

	// // case-insensitive regex: "~* pattern"
	// if (_urlPath.substr(0, 2) == "~*") {
	// 	std::string	lowerRequestPath = requestPath;
	// 	lowerRequestPath = strToLower(lowerRequestPath);
	// 	std::string pattern = _urlPath.substr(3); // Skip "~* "
	// 	return matchesRegex(strToLower(lowerRequestPath), strToLower(pattern));
	// }

	// // prefix match (default): "/api"
	return (requestPath.find(_urlPath) == 0);
}

bool	Route::matchesRegex(const std::string& path, const std::string& pattern) const
{
	if (pattern == ".*") return true;
	if (pattern.find(".*") != std::string::npos)
	{
		std::string prefix = pattern.substr(0, pattern.find(".*"));
		return (path.find(prefix) == 0);
	}
	return (path == pattern);
}

const Route*	Route::getMatchingRoute(std::string requestPath) const
{
	std::cout << "Matching " << requestPath <<  " with " << getRootDirectory() << "\n";

	if (isPathMatch(requestPath))
	{
		for (size_t i = 0; i < _nestedRoutes.size(); i++) {
			try
			{
				return _nestedRoutes[i].getMatchingRoute(requestPath);
			}

			catch (const std::runtime_error&)
			{
				continue;
			}
		}

		return this;
	}

	throw std::runtime_error("No matching route found");
}

//---------------------------- SETTERS --------------------------------------//

void	Route::setURLPath(std::string path)
{
	_urlPath = path;
}

void	Route::setRouteParam(std::map<std::string, std::string> paramMap)
{
	if (paramMap.find("root") != paramMap.end())
		_routedPath = paramMap.at("root");

	if (paramMap.find("index") != paramMap.end())
		_defaultFiles.push_back(paramMap.at("index"));
	else
		_defaultFiles.push_back("index.html");

	if (paramMap.find("autoindex") != paramMap.end())
		_autoindex = (paramMap.at("autoindex") == "on");
	else
		_autoindex = true;

	if (paramMap.find("allow_methods") != paramMap.end())
		_allowedMethods = split(paramMap.at("allow_methods"), ' ');

	if (paramMap.find("cgi_extension") != paramMap.end())
		_allowedMethods = split(paramMap.at("cgi_extension"), ' ');
}

void	Route::addNestedRoute(Route& route)
{
	_nestedRoutes.push_back(route);
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//
