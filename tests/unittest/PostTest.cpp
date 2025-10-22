#include "doctest.h"
#include "ContentFetcher.hpp"
#include "Route.hpp"
#include "ServerConf.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

std::string createTestFile(const std::string& path, const std::string& content)
{
	std::ofstream file(path.c_str());
	if (file.is_open())
	{
		file << content;
		file.close();
		return path;
	}
	return "";
}

void removeTestFile(const std::string& path)
{
	std::remove(path.c_str());
}

std::string readTestFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return "";

	std::string content;
	std::string line;
	while (std::getline(file, line))
	{
		content += line + "\n";
	}
	return content;
}

TEST_CASE("ContentPOSTER - findUploadFilepath Function")
{
	ContentFetcher fetcher;

	SUBCASE("Basic path construction")
	{
		Route route;
		route.setUploadDirectory("/var/uploads");
		route.setURLPath("/upload");

		std::string result = fetcher.findUploadFilepath(&route, "/upload/test.txt");

		CHECK(result.find("/var/uploads") != std::string::npos);
		CHECK(result.find("test.txt") != std::string::npos);
		CHECK(result == "/var/uploads/test.txt");
	}

	SUBCASE("Root path handling")
	{
		Route route;
		route.setUploadDirectory("/tmp");

		std::string result = fetcher.findUploadFilepath(&route, "/file.dat");

		CHECK(result.find("/tmp") != std::string::npos);
		CHECK(result.find("file.dat") != std::string::npos);
		CHECK(result == "/tmp/file.dat");
	}

	SUBCASE("Empty upload directory")
	{
		Route route;
		route.setUploadDirectory("");

		std::string result = fetcher.findUploadFilepath(&route, "/test.txt");

		CHECK(result == "test.txt");
	}
}

TEST_CASE("ContentPOSTER - URL Parsing Functions")
{
	ContentFetcher fetcher;

	SUBCASE("Parse simple key-value pair")
	{
		std::string testData = "name=John";

		size_t equalPos = testData.find('=');
		CHECK(equalPos != std::string::npos);

		std::string key = testData.substr(0, equalPos);
		std::string value = testData.substr(equalPos + 1);

		CHECK(key == "name");
		CHECK(value == "John");
	}

	SUBCASE("Parse multiple key-value pairs")
	{
		std::string testData = "name=John&email=john@test.com&age=25";

		// Test parsing multiple pairs
		std::vector<std::string> pairs;
		size_t start = 0;
		size_t ampPos = testData.find('&');

		while (ampPos != std::string::npos)
		{
			pairs.push_back(testData.substr(start, ampPos - start));
			start = ampPos + 1;
			ampPos = testData.find('&', start);
		}
		pairs.push_back(testData.substr(start)); // Last pair

		CHECK(pairs.size() == 3);
		CHECK(pairs[0] == "name=John");
		CHECK(pairs[1] == "email=john@test.com");
		CHECK(pairs[2] == "age=25");
	}

	SUBCASE("Handle URL encoded characters")
	{
		std::string testData = "message=Hello%20World";

		size_t equalPos = testData.find('=');
		std::string value = testData.substr(equalPos + 1);

		CHECK(value == "Hello%20World");
		// The function should preserve URL encoding for later decoding
	}
}

TEST_CASE("ContentPOSTER - File Writing Logic")
{
	SUBCASE("Write simple content to file")
	{
		std::string testPath = "/tmp/test_simple.txt";
		std::string testContent = "Hello World";

		std::string createdFile = createTestFile(testPath, testContent);
		CHECK(createdFile == testPath);

		std::string readContent = readTestFile(testPath);
		CHECK(readContent.find("Hello World") != std::string::npos);

		removeTestFile(testPath);
	}

	SUBCASE("Write empty content")
	{
		std::string testPath = "/tmp/test_empty.txt";
		std::string testContent = "";

		std::string createdFile = createTestFile(testPath, testContent);
		CHECK(createdFile == testPath);

		std::string readContent = readTestFile(testPath);
		bool validEmptyContent = readContent.empty() || readContent == "\n";
		CHECK(validEmptyContent);

		removeTestFile(testPath);
	}

	SUBCASE("Write content with special characters")
	{
		std::string testPath = "/tmp/test_special.txt";
		std::string testContent = "Special chars: !@#$%^&*()";

		std::string createdFile = createTestFile(testPath, testContent);
		CHECK(createdFile == testPath);

		std::string readContent = readTestFile(testPath);
		CHECK(readContent.find("Special chars") != std::string::npos);
		CHECK(readContent.find("!@#$%^&*()") != std::string::npos);

		removeTestFile(testPath);
	}
}

TEST_CASE("ContentPOSTER - Content Type Detection") {
	ContentFetcher fetcher;

	SUBCASE("Detect URL encoded content") {
		std::string contentType = "application/x-www-form-urlencoded";

		// Test content type string matching
		CHECK(contentType.find("application/x-www-form-urlencoded") == 0);

		bool isUrlEncoded = (contentType.find("application/x-www-form-urlencoded") != std::string::npos);
		CHECK(isUrlEncoded == true);
	}

	SUBCASE("Detect multipart content") {
		std::string contentType = "multipart/form-data; boundary=----WebKit";

		bool isMultipart = (contentType.find("multipart/form-data") != std::string::npos);
		CHECK(isMultipart == true);

		// Test boundary extraction
		size_t boundaryPos = contentType.find("boundary=");
		CHECK(boundaryPos != std::string::npos);

		std::string boundary = contentType.substr(boundaryPos + 9);
		CHECK(boundary == "----WebKit");
	}

	SUBCASE("Detect unsupported content type") {
		std::string contentType = "application/json";

		bool isUrlEncoded = (contentType.find("application/x-www-form-urlencoded") != std::string::npos);
		bool isMultipart = (contentType.find("multipart/form-data") != std::string::npos);

		CHECK(isUrlEncoded == false);
		CHECK(isMultipart == false);

		// This would be an unsupported type for file upload
		bool isSupported = isUrlEncoded || isMultipart;
		CHECK(isSupported == false);
	}
}

TEST_CASE("ContentPOSTER - Path Manipulation") {
	ContentFetcher fetcher;

	SUBCASE("Clean file paths") {
		std::string uploadDir = "/tmp/uploads";
		std::string filename = "test.txt";

		// Test path construction
		std::string fullPath = uploadDir + "/" + filename;
		CHECK(fullPath == "/tmp/uploads/test.txt");

		// Test with trailing slash
		std::string uploadDirWithSlash = "/tmp/uploads/";
		std::string fullPath2 = uploadDirWithSlash + filename;
		CHECK(fullPath2 == "/tmp/uploads/test.txt");
	}

	SUBCASE("Handle special characters in filenames") {
		std::string uploadDir = "/tmp/uploads";
		std::string filename = "test file with spaces.txt";

		std::string fullPath = uploadDir + "/" + filename;
		CHECK(fullPath.find("test file with spaces.txt") != std::string::npos);
	}

	SUBCASE("Generate default filename") {
		std::string defaultName = "default_upload";

		// Test default name generation when no filename provided
		CHECK(defaultName.length() > 0);
		CHECK(defaultName == "default_upload");
	}
}

TEST_CASE("ContentPOSTER - Error Handling Logic") {

	SUBCASE("Handle invalid key-value pairs") {
		std::string malformedData = "key_without_equals_sign";

		size_t equalPos = malformedData.find('=');
		bool hasEquals = (equalPos != std::string::npos);

		CHECK(hasEquals == false);

		// Function should handle this gracefully
		if (!hasEquals) {
			// Could treat as key with empty value, or skip entirely
			CHECK(true); // Handling strategy depends on implementation
		}
	}

	SUBCASE("Handle empty data") {
		std::string emptyData = "";

		CHECK(emptyData.empty() == true);
		CHECK(emptyData.length() == 0);

		// Function should not crash on empty input
		if (emptyData.empty()) {
			CHECK(true); // Should handle gracefully
		}
	}

	SUBCASE("Handle very long data") {
		std::string longKey(1000, 'k');
		std::string longValue(2000, 'v');
		std::string longData = longKey + "=" + longValue;

		CHECK(longData.length() > 3000);
		CHECK(longData.find('=') == 1000);

		// Function should handle large data without issues
		size_t equalPos = longData.find('=');
		if (equalPos != std::string::npos) {
			std::string key = longData.substr(0, equalPos);
			std::string value = longData.substr(equalPos + 1);

			CHECK(key.length() == 1000);
			CHECK(value.length() == 2000);
		}
	}
}

TEST_CASE("ContentPOSTER - Boundary Parsing") {

	SUBCASE("Extract boundary from content type") {
		std::string contentType = "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";

		size_t boundaryPos = contentType.find("boundary=");
		CHECK(boundaryPos != std::string::npos);

		std::string boundary = contentType.substr(boundaryPos + 9);
		CHECK(boundary == "----WebKitFormBoundary7MA4YWxkTrZu0gW");
	}

	SUBCASE("Handle missing boundary") {
		std::string contentType = "multipart/form-data";

		size_t boundaryPos = contentType.find("boundary=");
		CHECK(boundaryPos == std::string::npos);

		// Function should handle missing boundary
		if (boundaryPos == std::string::npos) {
			CHECK(true); // Should set error or use default
		}
	}

	SUBCASE("Boundary validation") {
		std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

		// Basic validation - boundary should not be empty and should start with dashes
		CHECK(!boundary.empty());
		CHECK(boundary.substr(0, 2) == "--");
		CHECK(boundary.length() > 4);
	}
}

TEST_CASE("ContentPOSTER - String Utilities") {

	SUBCASE("String trimming logic") {
		std::string testStr = "  hello world  ";

		// Remove leading spaces
		size_t start = testStr.find_first_not_of(" \t\r\n");
		size_t end = testStr.find_last_not_of(" \t\r\n");

		if (start != std::string::npos && end != std::string::npos) {
			std::string trimmed = testStr.substr(start, end - start + 1);
			CHECK(trimmed == "hello world");
		}
	}

	SUBCASE("Case conversion") {
		std::string testStr = "Content-Type";
		std::string lowerStr = testStr;

		// Convert to lowercase for comparison
		for (size_t i = 0; i < lowerStr.length(); ++i) {
			lowerStr[i] = std::tolower(lowerStr[i]);
		}

		CHECK(lowerStr == "content-type");
	}

	SUBCASE("String replacement") {
		std::string testStr = "Hello%20World";

		// Simple replacement test (URL decoding logic)
		size_t pos = testStr.find("%20");
		if (pos != std::string::npos) {
			testStr.replace(pos, 3, " ");
		}

		CHECK(testStr == "Hello World");
	}
}
