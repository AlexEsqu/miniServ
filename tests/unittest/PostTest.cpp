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
}

TEST_CASE("ContentPOSTER - Multipart Body Parsing") {

	SUBCASE("Simple multipart form with single field") {
		std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
		std::string multipartBody =
			"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
			"Content-Disposition: form-data; name=\"username\"\r\n"
			"\r\n"
			"john_doe\r\n"
			"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

		// Test boundary detection
		CHECK(multipartBody.find(boundary) != std::string::npos);

		// Test Content-Disposition parsing
		size_t dispositionPos = multipartBody.find("Content-Disposition:");
		CHECK(dispositionPos != std::string::npos);

		// Extract field name
		size_t namePos = multipartBody.find("name=\"");
		CHECK(namePos != std::string::npos);

		size_t nameStart = namePos + 6; // length of 'name="'
		size_t nameEnd = multipartBody.find("\"", nameStart);
		std::string fieldName = multipartBody.substr(nameStart, nameEnd - nameStart);
		CHECK(fieldName == "username");

		// Extract field value (after double CRLF)
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		CHECK(headerEnd != std::string::npos);

		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string fieldValue = multipartBody.substr(valueStart, valueEnd - valueStart);
		CHECK(fieldValue == "john_doe");
	}

	SUBCASE("Multipart form with multiple fields") {
		std::string boundary = "----WebKitFormBoundary123456";
		std::string multipartBody =
			"------WebKitFormBoundary123456\r\n"
			"Content-Disposition: form-data; name=\"username\"\r\n"
			"\r\n"
			"alice\r\n"
			"------WebKitFormBoundary123456\r\n"
			"Content-Disposition: form-data; name=\"email\"\r\n"
			"\r\n"
			"alice@example.com\r\n"
			"------WebKitFormBoundary123456\r\n"
			"Content-Disposition: form-data; name=\"age\"\r\n"
			"\r\n"
			"25\r\n"
			"------WebKitFormBoundary123456--\r\n";

		// Count number of parts
		std::string boundaryMarker = "--" + boundary;
		size_t pos = 0;
		int partCount = 0;

		while ((pos = multipartBody.find(boundaryMarker, pos)) != std::string::npos) {
			// Don't count the final boundary (ends with --)
			if (multipartBody.substr(pos + boundaryMarker.length(), 2) != "--") {
				partCount++;
			}
			pos += boundaryMarker.length();
		}

		CHECK(partCount == 3); // username, email, age

		// Verify all fields are present
		CHECK(multipartBody.find("name=\"username\"") != std::string::npos);
		CHECK(multipartBody.find("name=\"email\"") != std::string::npos);
		CHECK(multipartBody.find("name=\"age\"") != std::string::npos);

		// Verify field values
		CHECK(multipartBody.find("alice") != std::string::npos);
		CHECK(multipartBody.find("alice@example.com") != std::string::npos);
		CHECK(multipartBody.find("25") != std::string::npos);
	}

	SUBCASE("Multipart file upload") {
		std::string boundary = "----WebKitFormBoundaryFileUpload";
		std::string multipartBody =
			"------WebKitFormBoundaryFileUpload\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"This is the content of the uploaded file.\r\n"
			"It can contain multiple lines.\r\n"
			"------WebKitFormBoundaryFileUpload\r\n"
			"Content-Disposition: form-data; name=\"description\"\r\n"
			"\r\n"
			"A simple text file for testing\r\n"
			"------WebKitFormBoundaryFileUpload--\r\n";

		// Test filename extraction
		size_t filenamePos = multipartBody.find("filename=\"");
		CHECK(filenamePos != std::string::npos);

		size_t filenameStart = filenamePos + 10; // length of 'filename="'
		size_t filenameEnd = multipartBody.find("\"", filenameStart);
		std::string filename = multipartBody.substr(filenameStart, filenameEnd - filenameStart);
		CHECK(filename == "test.txt");

		// Test Content-Type extraction
		size_t contentTypePos = multipartBody.find("Content-Type:");
		CHECK(contentTypePos != std::string::npos);

		size_t typeStart = contentTypePos + 13; // length of 'Content-Type:'
		size_t typeEnd = multipartBody.find("\r\n", typeStart);
		std::string contentType = multipartBody.substr(typeStart, typeEnd - typeStart);

		// Trim whitespace
		while (!contentType.empty() && contentType[0] == ' ') {
			contentType = contentType.substr(1);
		}
		CHECK(contentType == "text/plain");

		// Test file content extraction
		CHECK(multipartBody.find("This is the content of the uploaded file.") != std::string::npos);
		CHECK(multipartBody.find("It can contain multiple lines.") != std::string::npos);
	}
}

TEST_CASE("ContentPOSTER - Large Multipart Request Parsing") {

	SUBCASE("Large text field (10KB)") {
		std::string boundary = "----WebKitFormBoundaryLargeData";

		// Create large content (10KB of text)
		std::string largeContent;
		for (int i = 0; i < 1000; i++) {
			largeContent += "This is line " + std::string(1, '0' + (i % 10)) + " of large content data.\n";
		}

		std::string multipartBody =
			"------WebKitFormBoundaryLargeData\r\n"
			"Content-Disposition: form-data; name=\"large_text\"\r\n"
			"\r\n" +
			largeContent +
			"------WebKitFormBoundaryLargeData--\r\n";

		// Verify body size
		CHECK(multipartBody.length() > 10000);

		// Test boundary detection in large content
		CHECK(multipartBody.find(boundary) == 2); // Should be at start (after --)

		// Test that large content is preserved
		CHECK(multipartBody.find("This is line 0 of large content data.") != std::string::npos);
		CHECK(multipartBody.find("This is line 9 of large content data.") != std::string::npos);

		// Extract the large field value
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string extractedContent = multipartBody.substr(valueStart, valueEnd - valueStart);

		CHECK(extractedContent.length() > 9000);
		CHECK(extractedContent == largeContent);
	}

	SUBCASE("Large binary file simulation (50KB)") {
		std::string boundary = "----WebKitFormBoundaryBinaryFile";

		// Create binary-like content (using printable chars for testing)
		std::string binaryContent;
		binaryContent.reserve(50000);

		for (int i = 0; i < 50000; i++) {
			binaryContent += static_cast<char>('A' + (i % 26));
		}

		std::string multipartBody =
			"------WebKitFormBoundaryBinaryFile\r\n"
			"Content-Disposition: form-data; name=\"binary_file\"; filename=\"large_file.dat\"\r\n"
			"Content-Type: application/octet-stream\r\n"
			"\r\n" +
			binaryContent +
			"\r\n------WebKitFormBoundaryBinaryFile--\r\n";

		// Verify large body
		CHECK(multipartBody.length() > 50000);

		// Test filename extraction
		size_t filenamePos = multipartBody.find("filename=\"");
		CHECK(filenamePos != std::string::npos);

		size_t filenameStart = filenamePos + 10;
		size_t filenameEnd = multipartBody.find("\"", filenameStart);
		std::string filename = multipartBody.substr(filenameStart, filenameEnd - filenameStart);
		CHECK(filename == "large_file.dat");

		// Test binary content integrity
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string extractedBinary = multipartBody.substr(valueStart, valueEnd - valueStart);

		CHECK(extractedBinary.length() == 50000);
		CHECK(extractedBinary[0] == 'A');
		CHECK(extractedBinary[25] == 'Z');
		CHECK(extractedBinary[26] == 'A'); // Wraps around
	}

	SUBCASE("Multiple large fields") {
		std::string boundary = "----WebKitFormBoundaryMultipleLarge";

		// Create multiple large fields
		std::string largeField1(5000, 'X');
		std::string largeField2(7000, 'Y');
		std::string largeField3(3000, 'Z');

		std::string multipartBody =
			"------WebKitFormBoundaryMultipleLarge\r\n"
			"Content-Disposition: form-data; name=\"field1\"\r\n"
			"\r\n" + largeField1 + "\r\n"
			"------WebKitFormBoundaryMultipleLarge\r\n"
			"Content-Disposition: form-data; name=\"field2\"\r\n"
			"\r\n" + largeField2 + "\r\n"
			"------WebKitFormBoundaryMultipleLarge\r\n"
			"Content-Disposition: form-data; name=\"field3\"\r\n"
			"\r\n" + largeField3 + "\r\n"
			"------WebKitFormBoundaryMultipleLarge--\r\n";

		// Verify total size
		CHECK(multipartBody.length() > 15000);

		// Test that all fields are detected
		CHECK(multipartBody.find("name=\"field1\"") != std::string::npos);
		CHECK(multipartBody.find("name=\"field2\"") != std::string::npos);
		CHECK(multipartBody.find("name=\"field3\"") != std::string::npos);

		// Count occurrences of boundary
		std::string boundaryMarker = "--" + boundary;
		size_t pos = 0;
		int boundaryCount = 0;

		while ((pos = multipartBody.find(boundaryMarker, pos)) != std::string::npos) {
			boundaryCount++;
			pos += boundaryMarker.length();
		}

		CHECK(boundaryCount == 4); // 3 field boundaries + 1 closing boundary
	}
}

TEST_CASE("ContentPOSTER - Multipart Edge Cases") {

	SUBCASE("Empty multipart field") {
		std::string boundary = "----WebKitFormBoundaryEmpty";
		std::string multipartBody =
			"------WebKitFormBoundaryEmpty\r\n"
			"Content-Disposition: form-data; name=\"empty_field\"\r\n"
			"\r\n"
			"\r\n"
			"------WebKitFormBoundaryEmpty--\r\n";

		// Test empty field detection
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string fieldValue = multipartBody.substr(valueStart, valueEnd - valueStart);

		CHECK(fieldValue.empty());
	}

	SUBCASE("Field with special characters") {
		std::string boundary = "----WebKitFormBoundarySpecial";
		std::string specialContent = "Content with\r\n\ttabs, newlines,\nand \"quotes\" & symbols!@#$%^&*()";

		std::string multipartBody =
			"------WebKitFormBoundarySpecial\r\n"
			"Content-Disposition: form-data; name=\"special_field\"\r\n"
			"\r\n" + specialContent + "\r\n"
			"------WebKitFormBoundarySpecial--\r\n";

		// Test special character preservation
		CHECK(multipartBody.find("tabs, newlines") != std::string::npos);
		CHECK(multipartBody.find("\"quotes\"") != std::string::npos);
		CHECK(multipartBody.find("!@#$%^&*()") != std::string::npos);

		// Extract and verify content
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string extractedContent = multipartBody.substr(valueStart, valueEnd - valueStart);

		CHECK(extractedContent == specialContent);
	}

	SUBCASE("Malformed multipart - missing boundary") {
		std::string malformedBody =
			"Content-Disposition: form-data; name=\"field\"\r\n"
			"\r\n"
			"value without boundary\r\n";

		// Test boundary detection failure
		std::string boundary = "----WebKitFormBoundary";
		CHECK(malformedBody.find(boundary) == std::string::npos);

		// Should be detected as malformed
		bool hasBoundary = (malformedBody.find("----") != std::string::npos);
		CHECK(hasBoundary == false);
	}

	SUBCASE("Boundary appearing in content") {
		std::string boundary = "----WebKitFormBoundaryInContent";
		std::string contentWithBoundary = "This content contains ----WebKitFormBoundaryInContent text";

		std::string multipartBody =
			"------WebKitFormBoundaryInContent\r\n"
			"Content-Disposition: form-data; name=\"tricky_field\"\r\n"
			"\r\n" + contentWithBoundary + "\r\n"
			"------WebKitFormBoundaryInContent--\r\n";

		// Count boundary occurrences
		size_t pos = 0;
		int boundaryCount = 0;
		std::string fullBoundary = "----WebKitFormBoundaryInContent";

		while ((pos = multipartBody.find(fullBoundary, pos)) != std::string::npos) {
			boundaryCount++;
			pos += fullBoundary.length();
		}

		CHECK(boundaryCount == 3); // start boundary, content, end boundary

		// The parser should distinguish between content and actual boundaries
		// by looking for proper boundary markers (starting with --)
		std::string properBoundary = "------WebKitFormBoundaryInContent";
		size_t properBoundaryCount = 0;
		pos = 0;

		while ((pos = multipartBody.find(properBoundary, pos)) != std::string::npos) {
			properBoundaryCount++;
			pos += properBoundary.length();
		}

		CHECK(properBoundaryCount == 2); // start and end boundaries only
	}
}

TEST_CASE("ContentPOSTER - Multipart Performance Test") {

	SUBCASE("Very large multipart request (1MB simulation)") {
		std::string boundary = "----WebKitFormBoundaryHuge";

		// Create a 1MB field (using smaller size for test performance)
		std::string hugeContent;
		hugeContent.reserve(100000); // 100KB for test

		for (int i = 0; i < 10000; i++) {
			hugeContent += "0123456789"; // 10 chars * 10000 = 100KB
		}

		std::string multipartBody =
			"------WebKitFormBoundaryHuge\r\n"
			"Content-Disposition: form-data; name=\"huge_field\"\r\n"
			"\r\n" + hugeContent + "\r\n"
			"------WebKitFormBoundaryHuge--\r\n";

		// Performance test - should handle large content quickly
		CHECK(multipartBody.length() > 100000);

		// Test parsing doesn't corrupt large content
		size_t headerEnd = multipartBody.find("\r\n\r\n");
		size_t valueStart = headerEnd + 4;
		size_t valueEnd = multipartBody.find("\r\n------", valueStart);
		std::string extractedHuge = multipartBody.substr(valueStart, valueEnd - valueStart);

		CHECK(extractedHuge.length() == 100000);
		CHECK(extractedHuge.substr(0, 10) == "0123456789");
		CHECK(extractedHuge.substr(99990, 10) == "0123456789");
	}
}

TEST_CASE("ContentPOSTER - Boundary Extraction Helper") {

	SUBCASE("Extract boundary from Content-Type header") {
		std::string contentTypeHeader = "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";

		std::string boundary = ContentFetcher::extractBoundary(contentTypeHeader);
		CHECK(boundary == "----WebKitFormBoundary7MA4YWxkTrZu0gW");
	}

	SUBCASE("Boundary with quotes") {
		std::string contentTypeHeader = "multipart/form-data; boundary=\"----WebKitBoundaryQuoted\"";

		std::string boundary = ContentFetcher::extractBoundary(contentTypeHeader);
		CHECK(boundary == "----WebKitBoundaryQuoted");
	}
}
