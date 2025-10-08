#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <cstring>

#include "FileHandler.hpp"

#define MEM_LIMIT 6400

class Buffer {
private:

	//------------------ ATTRIBUTES ----------------------//

	std::string			_memBuffer;
	FileHandler			_fileBuffer;
	size_t				_threshold;
	bool				_usingFile;
	std::istringstream	_memStream;
	std::ifstream		_fileStream;

	//-------------- INTERNAL FUNCTIONS -------------------//

	void			writeToBuffer(const char* data, size_t size);
	size_t			readFile(char* buffer, size_t size);

public:

	//----------------- CONSTRUCTORS ---------------------//

	Buffer();
	Buffer(const Buffer& original);

	//----------------- DESTRUCTOR -----------------------//

	~Buffer();

	//------------------- OPERATORS ----------------------//

	Buffer&			operator=(const Buffer& other);

	//-------------------- SETTER ------------------------//

	//-------------------- GETTERS -----------------------//

	size_t			getBufferSize() const;
	bool			isUsingFile() const;
	std::string		getMemoryBuffer() const;
	std::istream&	getStream();

	//--------------- MEMBER FUNCTIONS -------------------//

	void			writeToBuffer(const std::string& data);
	size_t			readFromBuffer(char* buffer, size_t size);
	void			clearBuffer();
};
