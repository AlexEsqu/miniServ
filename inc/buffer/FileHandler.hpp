#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

#define TMP_PREFIX "tmp/"

//class to create buffering files for receiving and sending operation
// and also upload when method is POST
class FileHandler {
private:

	std::string		_filePath;
	std::ofstream	_writeStream;
	std::ifstream	_readStream;
	size_t			_totalSize;

	bool			_isInitialized;
	bool			_isWriting;
	bool			_isReading;

	bool			_isPermanent;

public:

	//----------------- CONSTRUCTORS ---------------------//

	FileHandler();
	FileHandler(const std::string& filePath); // for uploads
	FileHandler(const FileHandler& original);

	//----------------- DESTRUCTOR -----------------------//

	~FileHandler();

	//----------------- OPERATOR -------------------------//

	FileHandler&		operator=(const FileHandler& original);

	//----------------------- SETTER ---------------------//


	//----------------------- GETTER ---------------------//

	size_t				size() const;
	const std::string&	getFilePath() const;
	bool				isInitialized() const;
	bool				isEmpty() const;
	bool				isReading() const;
	bool				isWriting() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				createFile();
	static std::string	generateRandomFileName(const std::string& prefix);
	std::string			generateTempFileName(const std::string& prefix);
	void				createFile(std::string& filePath);
	void				clearFile();

	void				writeToFile(const std::string& data);
	void				writeToFile(const char* data, size_t size);

	size_t				readFromFile(char* buffer, size_t size);

	void				finishWriting();
	void				flush();

	//----------------- EXCEPTION ------------------//

};
