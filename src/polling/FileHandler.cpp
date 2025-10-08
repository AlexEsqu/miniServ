#include "FileHandler.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

FileHandler::FileHandler()
	: _filePath("")
	, _totalSize(0)
	, _isInitialized(false)
	, _isWriting(false)
	, _isReading(false)
{
}

FileHandler::FileHandler(const FileHandler& original)
	: _filePath(original._filePath)
	, _totalSize(original._totalSize)
	, _isInitialized(original._isInitialized)
	, _isWriting(false)
	, _isReading(false)
{}

//--------------------------- DESTRUCTORS -----------------------------------//

FileHandler::~FileHandler()
{
	clearFile();
}

//----------------------------- OPERATOR -------------------------------------//


FileHandler&		FileHandler::operator=(const FileHandler& original)
{
	if (this != &original) {
		clearFile();
		_filePath = original._filePath;
		_totalSize = original._totalSize;
		_isInitialized = original._isInitialized;
		_isWriting = false;
		_isReading = false;
	}
	return *this;
}

//------------------------------ SETTER --------------------------------------//


//------------------------------ GETTER --------------------------------------//

size_t				FileHandler::size() const
{
	return _totalSize;
}

const std::string&	FileHandler::getFilePath()
{
	return _filePath;
}

bool				FileHandler::isInitialized() const
{
	return _isInitialized;
}

bool				FileHandler::isEmpty() const
{
	return (_totalSize == 0);
}

bool				FileHandler::isReading() const
{
	return _isReading;
}

bool				FileHandler::isWriting() const
{
	return _isWriting;
}

//------------------------- MEMBER FUNCTIONS ---------------------------------//

std::string FileHandler::generateTempFileName(const std::string& prefix)
{
	std::stringstream oss;
	oss << prefix << "_"
		<< time(0) << "_"
		<< rand() % 100000 << ".tmp";
	return oss.str();
}

void	FileHandler::createFile()
{
	_filePath = generateTempFileName(TMP_PREFIX);

	createFile(_filePath);
}

void	FileHandler::createFile(std::string& filePath)
{
	if (_isInitialized)
		clearFile();

	_filePath = filePath;

	_writeStream.open(_filePath, std::ios::binary | std::ios::trunc);
	if (!_writeStream.is_open())
		throw std::runtime_error("Failed to open file for writing");
	_isInitialized = true;
	_isWriting = true;
	_totalSize = 0;
}

void	FileHandler::clearFile()
{
	if (_writeStream.is_open())
		_writeStream.close();
	if (_readStream.is_open())
		_readStream.close();

	int fd = open(_filePath.c_str(), O_TRUNC);
	close(fd);
	// would unset here if it was allowed function...

	_filePath.clear();
	_isInitialized = false;
	_isWriting = false;
	_isReading = false;
	_totalSize = 0;
}

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

size_t	FileHandler::readFromFile(char* buffer, size_t size)
{
	if (!_isInitialized)
		throw std::runtime_error("File not initialized");
	if (!_isReading) {
		_readStream.open(_filePath, std::ios::binary);
		if (!_readStream.is_open())
			throw std::runtime_error("Failed to open file for reading");
		_isReading = true;
	}
	_readStream.read(buffer, size);
	return _readStream.gcount();
}

void	FileHandler::finishWriting()
{
	if (_writeStream.is_open())
		_writeStream.close();
	_isWriting = false;
}

void	FileHandler::flush()
{
	if (_writeStream.is_open())
		_writeStream.flush();
}

//------------------------------ EXCEPTIONS ----------------------------------//

