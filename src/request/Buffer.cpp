#include "Buffer.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Buffer::Buffer()
	: _threshold(MEM_LIMIT)
	, _usingFile(false)
{

}

Buffer::Buffer(const Buffer& original)
	: _memBuffer(original._memBuffer)
	, _fileBuffer(original._fileBuffer)
	, _threshold(original._threshold)
	, _usingFile(original._usingFile)
{

}

//--------------------------- DESTRUCTORS -----------------------------------//

Buffer::~Buffer()
{

}

//--------------------------- OPERATORS -------------------------------------//

Buffer&			Buffer::operator=(const Buffer& other)
{
	if (this == &other)
		return *this;

	_memBuffer = other._memBuffer;
	_fileBuffer = other._fileBuffer;
	_threshold = other._threshold;
	_usingFile = other._usingFile;
	// beware: not copying the streams
	return *this;
}

//------------------------- INTERNAL FUNCTIONS ------------------------------//



//------------------------------ GETTER --------------------------------------//

size_t			Buffer::getBufferSize() const
{
	if (_usingFile)
		return _fileBuffer.size();
	else
		return _memBuffer.size();
}

bool			Buffer::isUsingFile() const
{
	return _usingFile;
}

// Reads from the memory held buffer string for quicker access
std::string		Buffer::getMemoryBuffer() const
{
	return _memBuffer;
}

// Read from file buffer for less memory use
size_t			Buffer::readFile(char* buffer, size_t size)
{
	if (_usingFile)
		return _fileBuffer.readFromFile(buffer, size);
	return 0;
}

// Creates a new stream pointer out of the buffer, be it string or file
std::istream&	Buffer::getStream()
{
	if (_usingFile) {
		if (!_fileStream.is_open())
			_fileStream.open(_fileBuffer.getFilePath().c_str(), std::ios::binary);
		return _fileStream;
	} else {
		_memStream.str(_memBuffer);
		_memStream.clear();
		return _memStream;
	}
}

//------------------------- MEMBER FUNCTIONS ---------------------------------//


void			Buffer::writeToBuffer(const std::string& data)
{
	writeToBuffer(data.c_str(), data.size());
}

void			Buffer::writeToBuffer(const char* data, size_t size)
{
	if (!_usingFile && (_memBuffer.size() + size > _threshold))
	{
		_fileBuffer.createFile();
		_fileBuffer.writeToFile(_memBuffer);
		_memBuffer.clear();
		_usingFile = true;
	}
	if (_usingFile)
	{
		_fileBuffer.writeToFile(data, size);
	}
	else
	{
		_memBuffer.append(data, size);
	}
}

void			Buffer::clearBuffer()
{
	if (_usingFile)
	{
		_fileBuffer.clearFile();
		_usingFile = false;
	}
	_memBuffer.clear();
}

size_t			Buffer::readFromBuffer(char* buffer, size_t size)
{
	if (_usingFile)
		return _fileBuffer.readFromFile(buffer, size);
	else
	{
		size_t toRead = std::min(size, _memBuffer.size());
		std::memcpy(buffer, _memBuffer.data(), toRead);
		return toRead;
	}
}

size_t Buffer::readFromBuffer(char* buffer, size_t size, size_t offset) const
{
	if (_usingFile) {
		std::ifstream file(_fileBuffer.getFilePath().c_str(), std::ios::binary);
		file.seekg(offset);
		file.read(buffer, size);
		return file.gcount();
	} else {
		if (offset >= _memBuffer.size()) return 0;

		size_t toRead = std::min(size, _memBuffer.size() - offset);
		std::memcpy(buffer, _memBuffer.data() + offset, toRead);
		return toRead;
	}
}

std::string Buffer::getAllContent() const
{
	if (_usingFile)
	{
		std::ifstream file(_fileBuffer.getFilePath().c_str(), std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "failed to open buffer\n";
			return ("");
		}

		std::ostringstream oss;
		oss << file.rdbuf();
		return (oss.str());
	}
	else
		return (_memBuffer);
}

// size_t			Buffer::findInBuffer(const std::string& signature) const
// {
// 	if (_usingFile)
// 	{
// 		// inefficient as hell
// 		// TO DO: need better logic / checking when appending
// 		std::ifstream file(_fileBuffer.getFilePath().c_str(), std::ios::binary);
// 		std::string temp((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
// 		return temp.find(signature);
// 	} else
// 	{
// 		return _memBuffer.find(signature);
// 	}
// }
