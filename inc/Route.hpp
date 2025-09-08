#pragma once
#include <iostream>
#include <vector>

class Route
{

private:

	std::string					_rootDirectory;
	std::string					_defaultFile;
	bool						_directoryListing;
	std::vector<std::string>	_allowedCGI;
	std::string					_uploadDirectory;
	bool						_isGetAllowed;
	bool						_isPostAllowed;
	bool						_isDeleteAllowed;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Route();
	Route(const Route &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Route();

	//------------------- OPERATORS ----------------------//

	Route &operator=(const Route &other);


	//--------------- MEMBER FUNCTIONS -------------------//

};
