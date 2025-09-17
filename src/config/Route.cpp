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
	setIsGetAllowed(true);
	setIsPostAllowed(false);
	setIsDeleteAllowed(false);



	#ifdef DEBUG
		std::cout << "Route Constructor called" << std::endl;
	#endif
}

Route::Route(const Route &copy)
{
	#ifdef DEBUG
		std::cout << "Route copy Constructor called" << std::endl;
	#endif
	*this = copy;
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
	// code
	(void)other;
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

bool Route::getIsGetAllowed() const
{
	return (this->_isGetAllowed);
}

bool Route::getIsPostAllowed() const
{
	return (this->_isPostAllowed);
}

bool Route::getIsDeleteAllowed() const
{
	return (this->_isDeleteAllowed);
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

void Route::setIsGetAllowed(bool isGetAllowed)
{
	this->_isGetAllowed = isGetAllowed;
}

void Route::setIsPostAllowed(bool isPostAllowed)
{
	this->_isPostAllowed = isPostAllowed;
}

void Route::setIsDeleteAllowed(bool isDeleteAllowed)
{
	this->_isDeleteAllowed = isDeleteAllowed;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//
