#include "Route.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Route::Route()
{
	setRootDirectory("./pages/");

	std::vector<std::string> defaultFiles;
	defaultFiles.push_back("index.html");
	defaultFiles.push_back("index.php");
	setDefaultFiles(defaultFiles);


	std::vector<std::string> allowedCGI;
	allowedCGI.push_back(".php");
	allowedCGI.push_back(".py");
	setAllowedCGI(allowedCGI);

	setUploadDirectory("/pages/upload/");
	std::vector<std::string> allowedMethods;
	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("DELETE");

	#ifdef DEBUG
		std::cout << "Route Constructor called" << std::endl;
	#endif
}

Route::Route(std::string root, std::vector<std::string> defaultFiles,
		std::vector<std::string> allowedCGI, std::vector<std::string> allowedMethods,
		std::string uploadDirectory)
	: _rootDirectory(root)
	, _defaultFiles(defaultFiles)
	, _allowedCGI(allowedCGI)
	, _uploadDirectory(uploadDirectory)
	, _allowedMethods(allowedMethods)
{
	#ifdef DEBUG
		std::cout << "Route Constructor called" << std::endl;
	#endif
}

Route::Route(std::string root)
{
	setRootDirectory(root);
	std::vector<std::string> defaultFiles;
	defaultFiles.push_back("index.html");
	defaultFiles.push_back("index.php");
	setDefaultFiles(defaultFiles);


	std::vector<std::string> allowedCGI;
	allowedCGI.push_back(".php");
	allowedCGI.push_back(".py");
	setAllowedCGI(allowedCGI);

	setUploadDirectory("/pages/upload/");

	#ifdef DEBUG
		std::cout << "Route Constructor called" << std::endl;
	#endif
}

Route::Route(const Route &copy)
{
	this->_rootDirectory = copy._rootDirectory;
	this->_defaultFiles = copy._defaultFiles;
	this->_directoryListing = copy._directoryListing;
	this->_allowedCGI = copy._allowedCGI;
	this->_uploadDirectory = copy._uploadDirectory;

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

Route &Route::operator=(const Route &other)
{
	if (this != &other)
	{
		this->_rootDirectory	= other._rootDirectory;
		this->_defaultFiles		= other._defaultFiles;
		this->_directoryListing	= other._directoryListing;
		this->_allowedCGI		= other._allowedCGI;
		this->_uploadDirectory	= other._uploadDirectory;
		this->_allowedMethods	= other._allowedMethods;
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

std::string Route::getRootDirectory() const
{
	return (this->_rootDirectory);
}

std::vector<std::string> Route::getDefaultFiles() const
{
	return (this->_defaultFiles);
}

bool Route::getDirectoryListing() const
{
	return (this->_directoryListing);
}

std::vector<std::string> Route::getAllowedCGI() const
{
	return (this->_allowedCGI);
}

std::string Route::getUploadDirectory() const
{
	return (this->_uploadDirectory);
}


//---------------------------- SETTERS --------------------------------------//

void Route::setRootDirectory(std::string rootDirectory)
{
	this->_rootDirectory = rootDirectory;
}

void Route::setDefaultFiles(std::vector<std::string> &defaultFiles)
{
	this->_defaultFiles = defaultFiles;
}

void Route::setDirectoryListing(bool directoryListing)
{
	this->_directoryListing = directoryListing;
}

void Route::setAllowedCGI(std::vector<std::string> &allowedCGI)
{
	this->_allowedCGI = allowedCGI;
}

void Route::setUploadDirectory(std::string uploadDirectory)
{
	this->_uploadDirectory = uploadDirectory;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//
