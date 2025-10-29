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
		this->_rootDirectory		= other._rootDirectory;
		this->_urlPath			= other._urlPath;
		this->_defaultFiles		= other._defaultFiles;
		this->_autoindex		= other._autoindex;
		this->_allowedCGI		= other._allowedCGI;
		this->_uploadDirectory	= other._uploadDirectory;
		this->_allow_methods	= other._allow_methods;

		_nestedRoutes.clear();
		for (size_t i = 0; i < other._nestedRoutes.size(); i++)
			this->_nestedRoutes.push_back(other._nestedRoutes[i]);
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

std::string Route::getRootDirectory() const
{
	return (this->_rootDirectory);
}

const std::vector<std::string> Route::getDefaultFiles() const
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

const std::vector<Route>&	Route::getRoutes() const
{
	return _nestedRoutes;
}

const std::vector<std::string>	Route::getAllowedMethods() const
{
	return _allow_methods;
}

bool	Route::isAllowedMethod(const std::string& methodAsString) const
{
	if (_allow_methods.empty())
		return false;
	return (std::find(_allow_methods.begin(), _allow_methods.end(), methodAsString) != _allow_methods.end());
}

bool	Route::isPathMatch(const std::string& requestPath) const
{
	// std::cout << "Checking if " << requestPath << " matches " << _urlPath << std::endl;

	if (_urlPath == "/")
		return true;

	if (requestPath.find(_urlPath) == 0)
	{
		if (requestPath.length() == _urlPath.length())
			return true;

		if (requestPath.length() > _urlPath.length() &&
			requestPath[_urlPath.length()] == '/')
			return true;
	}

	return false;
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
	if (isPathMatch(requestPath))
	{
		for (size_t i = 0; i < _nestedRoutes.size(); i++)
		{
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
	return NULL;
}

//---------------------------- SETTERS --------------------------------------//

void	Route::setURLPath(std::string path)
{
	_urlPath = path;
}

void	Route::setRouteParam(std::map<std::string, std::string> paramMap)
{
	if (paramMap.find("root") != paramMap.end())
		_rootDirectory = paramMap.at("root");

	if (paramMap.find("index") != paramMap.end())
		_defaultFiles = split(paramMap.at("index"), ' ');

	if (paramMap.find("autoindex") != paramMap.end())
		_autoindex = (paramMap.at("autoindex") == "on");
	else
		_autoindex = true;

	if (paramMap.find("allowed_methods") != paramMap.end())
		_allow_methods = split(paramMap.at("allowed_methods"), ' ');

	if (paramMap.find("upload") != paramMap.end())
		_uploadDirectory = paramMap.at("upload");
	else
		_uploadDirectory = "pages/upload";

	if (paramMap.find("cgi_extension") != paramMap.end())
		_allowedCGI = split(paramMap.at("cgi_extension"), ' ');
}

void	Route::setRootDirectory(std::string path)
{
	_rootDirectory = path;
} // TO DO : sat root of server or parent if none given

void	Route::setUploadDirectory(std::string path)
{
	_uploadDirectory = path;
}

void 	Route::setAllowedMethods(std::vector<std::string> methodVector)
{
	_allow_methods = methodVector;
}

void	Route::addNestedRoute(Route& route)
{
	_nestedRoutes.push_back(route);
}

void	Route::setDefaultFiles(std::vector<std::string> defaultFilesVector)
{
	_defaultFiles = defaultFilesVector;
}

void	Route::setAllowedCGI(std::vector<std::string> allowedCGIVector)
{
	_allowedCGI = allowedCGIVector;
}

void	Route::setAutoIndex(bool value)
{
	_autoindex = value;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//
