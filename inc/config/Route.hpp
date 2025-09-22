#pragma once
#include <iostream>
#include <vector>

class Route
{

private:

	std::string					_rootDirectory;
	std::vector<std::string>	_defaultFiles;
	bool						_directoryListing; // what does that mean?
	std::vector<std::string>	_allowedCGI;
	std::string					_uploadDirectory;
	bool						_isGetAllowed;
	bool						_isPostAllowed;
	bool						_isDeleteAllowed;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Route();
	Route(std::string root);
	Route(const Route &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Route();

	//------------------- OPERATORS ----------------------//

	Route &operator=(const Route &other);


	//--------------- MEMBER FUNCTIONS -------------------//
	void setRootDirectory(std::string rootDirectory);
	void setDefaultFiles(std::vector<std::string> &defaultFiles);
	void setDirectoryListing(bool directoryListing);
	void setAllowedCGI(std::vector<std::string> &allowedCGI);
	void setUploadDirectory(std::string uploadDirectory);
	void setIsGetAllowed(bool isGetAllowed);
	void setIsPostAllowed(bool isPostAllowed);
	void setIsDeleteAllowed(bool isDeleteAllowed);

	std::string getRootDirectory() const;
	std::vector<std::string> getDefaultFiles() const;
	bool getDirectoryListing() const;
	std::vector<std::string> getAllowedCGI() const;
	std::string getUploadDirectory() const;
	bool getIsGetAllowed() const;
	bool getIsPostAllowed() const;
	bool getIsDeleteAllowed() const;
};
