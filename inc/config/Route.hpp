#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "parsing.hpp"

class Route
{

private:

	std::string					_urlPath;
	std::string					_routedPath;
	std::string					_uploadDirectory;
	bool						_autoindex;
	std::vector<std::string>	_defaultFiles;
	std::vector<std::string>	_allowedCGI;
	std::vector<std::string>	_allowedMethods;
	std::vector<Route>			_nestedRoutes;

	bool		isPathMatch(const std::string& requestPath) const;
	bool		matchesRegex(const std::string& path, const std::string& pattern) const;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Route();
	Route(const Route &copy);

	//----------------- DESTRUCTOR -----------------------//

	virtual	~Route();

	//------------------- OPERATORS ----------------------//

	Route&			operator=(const Route &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void						setRouteParam(std::map<std::string, std::string> paramMap);
	void						setURLPath(std::string path);
	void						addNestedRoute(Route& route);

	const Route*				getMatchingRoute(std::string path) const;
	std::string					getURLPath() const;
	std::string					getRootDirectory() const;
	std::vector<std::string>	getDefaultFiles() const;
	const std::vector<Route>&	getRoutes() const;
	bool						isAutoIndex() const;
	std::vector<std::string>	getAllowedCGI() const;
	std::string					getUploadDirectory() const;

};
