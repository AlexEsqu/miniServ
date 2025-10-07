#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

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

public:

	//----------------- CONSTRUCTORS ---------------------//

	FileHandler();
	FileHandler(const FileHandler& original);

	//----------------- DESTRUCTOR -----------------------//

	~FileHandler();

	//----------------- OPERATOR -------------------------//

	FileHandler&		operator=(const FileHandler& original);

	//----------------------- SETTER ---------------------//


	//----------------------- GETTER ---------------------//

	size_t				size() const;
	const std::string&	getFilePath();
	bool				isInitialized() const;
	bool				isEmpty() const;
	bool				isReading() const;
	bool				isWriting() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				createFile();
	void				clearFile();

	void				writeToFile(const std::string& data);
	void				writeToFile(const char* data, size_t size);
	size_t				readFromFile(char* buffer, size_t size);

	void				finishWriting();
	void				reset();
	void				clear();
	void				flush();

	//----------------- EXCEPTION ------------------//

};
