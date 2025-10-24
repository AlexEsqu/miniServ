#include "doctest.h"
#include "FileHandler.hpp"
#include <cstring>

TEST_CASE("FILE HANDLING")
{
	SUBCASE("FileHandler creates and writes to temp file") {
		FileHandler fh;
		fh.createFile();
		CHECK(fh.isInitialized());
		CHECK(fh.isWriting());
		CHECK(fh.size() == 0);

		std::string data = "Hello, FileHandler!";
		fh.writeToFile(data);
		CHECK(fh.size() == data.size());

		fh.finishWriting();
		CHECK(!fh.isWriting());
	}

	SUBCASE("FileHandler reads from file") {
		FileHandler fh;
		fh.createFile();
		std::string data = "Read this!";
		fh.writeToFile(data);
		fh.finishWriting();

		char buffer[32] = {0};
		size_t bytesRead = fh.readFromFile(buffer, sizeof(buffer));
		CHECK(bytesRead == data.size());
		CHECK(std::string(buffer, bytesRead) == data);
	}

	SUBCASE("FileHandler clearFile resets state") {
		FileHandler fh;
		fh.createFile();
		std::string data = "To be cleared";
		fh.writeToFile(data);
		fh.finishWriting();

		std::string path = fh.getFilePath();
		fh.clearFile();
		CHECK(!fh.isInitialized());
		CHECK(fh.isEmpty());
		CHECK(fh.getFilePath().empty());
	}

	SUBCASE("FileHandler throws on write when not in writing mode") {
		FileHandler fh;
		fh.createFile();
		fh.finishWriting();
		CHECK_THROWS_AS(fh.writeToFile("fail"), std::runtime_error);
	}

	SUBCASE("FileHandler throws on read when not initialized") {
		FileHandler fh;
		char buffer[8];
		CHECK_THROWS_AS(fh.readFromFile(buffer, sizeof(buffer)), std::runtime_error);
	}
}
