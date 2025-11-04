#include "doctest.h"
#include "server.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>

static bool fileExists(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

TEST_CASE("FileHandler Tests") {
	// Ensure tmp directory exists for tests
	mkdir("tmp", 0755);

	SUBCASE("Temporary FileHandler (Default Constructor)") {
		std::string tempFilePath;
		{
			FileHandler fh;
			CHECK_FALSE(fh.isInitialized());
			CHECK(fh.isEmpty());

			// First write should create the file
			std::string content = "hello temporary file";
			fh.writeToFile(content);

			CHECK(fh.isInitialized());
			CHECK_FALSE(fh.isEmpty());
			CHECK(fh.size() == content.length());
			tempFilePath = fh.getFilePath();
			CHECK(fileExists(tempFilePath));
		} // fh goes out of scope here, destructor should be called

		// File should be deleted after destruction
		CHECK_FALSE(fileExists(tempFilePath));
	}

	SUBCASE("Permanent FileHandler (Path Constructor)") {
		std::string permanentPath = "tmp/permanent_test_file.txt";
		// Cleanup before test, in case of previous failure
		if (fileExists(permanentPath)) {
			std::remove(permanentPath.c_str());
		}

		{
			FileHandler fh(permanentPath);
			CHECK(fh.isInitialized());
			CHECK(fileExists(permanentPath));

			std::string content = "this file should remain";
			fh.writeToFile(content);
			CHECK(fh.size() == content.length());
		} // fh goes out of scope, but file should NOT be deleted

		CHECK(fileExists(permanentPath));

		// Cleanup after test
		std::remove(permanentPath.c_str());
	}

	SUBCASE("Write and Read Operations") {
		FileHandler fh;
		std::string content1 = "first part. ";
		std::string content2 = "second part.";
		fh.writeToFile(content1);
		fh.writeToFile(content2);

		CHECK(fh.size() == content1.length() + content2.length());

		char readBuffer[100] = {0};
		size_t bytesRead = fh.readFromFile(readBuffer, 50);

		CHECK(bytesRead == content1.length() + content2.length());
		CHECK(std::string(readBuffer) == (content1 + content2));

		// Test reading again from the beginning
		memset(readBuffer, 0, 100);
		bytesRead = fh.readFromFile(readBuffer, 5);
		CHECK(bytesRead == 5);
		CHECK(std::string(readBuffer, 5) == "first");
	}

	SUBCASE("clearFile functionality") {
		FileHandler fh;
		std::string content = "some data to be cleared";
		fh.writeToFile(content);

		std::string path = fh.getFilePath();
		CHECK(fh.isInitialized());
		CHECK(fileExists(path));

		fh.clearFile();

		CHECK_FALSE(fh.isInitialized());
		CHECK(fh.isEmpty());
		CHECK(fh.getFilePath().empty());
		CHECK_FALSE(fileExists(path));
	}
}

TEST_CASE("Buffer Tests") {
	SUBCASE("In-memory buffering") {
		Buffer buf;
		CHECK_FALSE(buf.isUsingFile());
		CHECK(buf.getBufferSize() == 0);

		std::string content1 = "small content";
		buf.writeToBuffer(content1);

		CHECK_FALSE(buf.isUsingFile());
		CHECK(buf.getBufferSize() == content1.length());
		CHECK(buf.getAllContent() == content1);

		char readArr[50] = {0};
		size_t bytesRead = buf.readFromBuffer(readArr, 50);
		CHECK(bytesRead == content1.length());
		CHECK(std::string(readArr) == content1);
	}

	SUBCASE("Switch to file buffering when threshold is exceeded") {
		Buffer buf;
		// Use a string that is smaller than the threshold
		std::string initialContent(MEM_LIMIT - 100, 'A');
		buf.writeToBuffer(initialContent);

		CHECK_FALSE(buf.isUsingFile());
		CHECK(buf.getBufferSize() == initialContent.length());

		// Write more data to exceed the threshold
		std::string additionalContent(200, 'B');
		buf.writeToBuffer(additionalContent);

		CHECK(buf.isUsingFile());
		CHECK(buf.getBufferSize() == initialContent.length() + additionalContent.length());

		// Check that the content is correct
		std::string totalContent = buf.getAllContent();
		CHECK(totalContent.length() == initialContent.length() + additionalContent.length());
		CHECK(totalContent.substr(0, initialContent.length()) == initialContent);
		CHECK(totalContent.substr(initialContent.length()) == additionalContent);
	}

	SUBCASE("getStream functionality") {
		// Test with memory buffer
		Buffer memBuf;
		std::string memContent = "stream from memory";
		memBuf.writeToBuffer(memContent);
		std::istream& memStream = memBuf.getStream();
		std::string streamContent;
		memStream >> streamContent;
		CHECK(streamContent == "stream");

		// Test with file buffer
		Buffer fileBuf;
		std::string fileContent(MEM_LIMIT + 1, 'X');
		fileBuf.writeToBuffer(fileContent);
		CHECK(fileBuf.isUsingFile());
		std::istream& fileStream = fileBuf.getStream();
		char readChar;
		fileStream.get(readChar);
		CHECK(readChar == 'X');
	}

	SUBCASE("getReadableFd functionality") {
		// Test with memory buffer (using a pipe)
		Buffer memBuf;
		std::string memContent = "pipe test";
		memBuf.writeToBuffer(memContent);

		int fd = memBuf.getReadableFd();
		CHECK(fd > 2); // Should be a valid file descriptor

		char readArr[20] = {0};
		ssize_t bytesRead = read(fd, readArr, 19);
		close(fd); // IMPORTANT: close the fd returned by getReadableFd

		CHECK(bytesRead == memContent.length());
		CHECK(std::string(readArr) == memContent);

		// Test with file buffer
		Buffer fileBuf;
		std::string fileContent(MEM_LIMIT + 1, 'F');
		fileBuf.writeToBuffer(fileContent);
		CHECK(fileBuf.isUsingFile());

		fd = fileBuf.getReadableFd();
		CHECK(fd > 2);

		memset(readArr, 0, 20);
		bytesRead = read(fd, readArr, 10);
		close(fd);

		CHECK(bytesRead == 10);
		CHECK(std::string(readArr, 10) == std::string(10, 'F'));
	}

	SUBCASE("clearBuffer functionality") {
		// Test clearing a memory buffer
		Buffer memBuf;
		memBuf.writeToBuffer("some data");
		memBuf.clearBuffer();
		CHECK(memBuf.getBufferSize() == 0);
		CHECK_FALSE(memBuf.isUsingFile());
		CHECK(memBuf.getAllContent().empty());

		// Test clearing a file buffer
		Buffer fileBuf;
		fileBuf.writeToBuffer(std::string(MEM_LIMIT + 1, 'Z'));
		CHECK(fileBuf.isUsingFile());
		CHECK(fileBuf.getBufferSize() > 0);

		fileBuf.clearBuffer();
		CHECK(fileBuf.getBufferSize() == 0);
		CHECK_FALSE(fileBuf.isUsingFile());
		CHECK(fileBuf.getAllContent().empty());
	}
}
