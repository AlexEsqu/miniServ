#include "FileHandler.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

FileHandler::FileHandler()
{

}

//--------------------------- DESTRUCTORS -----------------------------------//

FileHandler::~FileHandler()
{

}

//----------------------------- OPERATOR -------------------------------------//


FileHandler&		FileHandler::operator=(const FileHandler& original)
{

}

//------------------------------ SETTER --------------------------------------//


//------------------------------ GETTER --------------------------------------//


//------------------------- MEMBER FUNCTIONS ---------------------------------//

void	FileHandler::writeToFile(const std::string& data)
{
	writeToFile(data.c_str(), data.size());
}

void	FileHandler::writeToFile(const char* data, size_t size)
{
	if (!_isInitialized)
		createFile();

	if (!_isWriting) {
		throw std::runtime_error("Buffer is not in writing mode");
	}

	_writeStream.write(data, size);
	if (_writeStream.fail()) {
		throw std::runtime_error("Failed to write to buffer file");
	}

	_totalSize += size;
}

//------------------------------ EXCEPTIONS ----------------------------------//

