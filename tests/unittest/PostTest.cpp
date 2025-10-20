#include "doctest.h"
#include "server.hpp"
#include <sstream>
#include <fstream>

ServerConf createMockServerConfig() {
	std::map<std::string, std::string> params;
	params["listen"] = "8080";
	params["root"] = "/var/www";

	ServerConf config(params);
	config.setPort(8080);
	config.setRoot("/var/www");

	// Add a default route for uploads
	Route uploadRoute;
	uploadRoute.setURLPath("/upload");
	uploadRoute.setRootDirectory("/var/www/uploads");
	std::vector<std::string> methods;
	methods.push_back("POST");
	methods.push_back("GET");
	uploadRoute.setAllowedMethods(methods);

	config.addRoute(uploadRoute);

	return config;
}

std::string intToString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string createLargeString(size_t size, char c) {
	std::string result;
	result.reserve(size);
	for (size_t i = 0; i < size; ++i) {
		result += c;
	}
	return result;
}

TEST_CASE("POST Method - Basic Form Data Parsing") {

	SUBCASE("Simple form data POST request") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string postRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: 27\r\n"
			"\r\n"
			"name=John&email=john@test.com";

		Request request(config, status, postRequest);

		// Verify request parsing
		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getRequestedURL() == "/upload");
		CHECK(request.getProtocol() == "HTTP/1.1");
		CHECK(request.getParsingState() == PARSING_DONE);

		// Verify body content
		std::string body = request.getBody();
		CHECK(body == "name=John&email=john@test.com");
		CHECK(body.length() == 27);

		// Verify headers
		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["content-type"] == "application/x-www-form-urlencoded");
		CHECK(headers["content-length"] == "27");
		CHECK(headers["host"] == "localhost:8080");
	}

	SUBCASE("POST with chunked transfer encoding") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string chunkedRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"1a\r\n"
			"This is chunked content!\r\n"
			"0\r\n"
			"\r\n";

		Request request(config, status, chunkedRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);

		std::string body = request.getBody();
		CHECK(body == "This is chunked content!");
	}

	SUBCASE("Large POST data") {
		ServerConf config = createMockServerConfig();
		Status status;

		// Create large content (1KB) - C++98 compatible
		std::string largeContent = createLargeString(1024, 'X');
		std::string lengthStr = intToString(static_cast<int>(largeContent.length()));

		std::string largePostRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Content-Length: " + lengthStr + "\r\n"
			"\r\n" + largeContent;

		Request request(config, status, largePostRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);

		std::string body = request.getBody();
		CHECK(body.length() == 1024);
		CHECK(body == largeContent);
	}
}

TEST_CASE("POST Method - ContentFetcher Response Generation") {

	SUBCASE("File upload handling") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string fileUploadRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
			"Content-Length: 196\r\n"
			"\r\n"
			"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

		Request request(config, status, fileUploadRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);
		CHECK(!request.hasError());

		// Verify multipart content is captured
		std::string body = request.getBody();
		CHECK(body.find("Content-Disposition: form-data") != std::string::npos);
		CHECK(body.find("Hello, World!") != std::string::npos);
	}

	SUBCASE("Form submission response") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string formRequest =
			"POST /contact HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: 45\r\n"
			"\r\n"
			"name=Jane+Doe&email=jane%40example.com&age=25";

		Request request(config, status, formRequest);
		Response response(&request);

		CHECK(response.getRequest()->getStatus().getStatusCode() == 201);

		// Verify response content contains success message
		std::string responseContent = response.getHTTPResponse();
		CHECK(responseContent.find("uploaded successfully") != std::string::npos);
		CHECK(responseContent.find("text/html") != std::string::npos);
	}
}

TEST_CASE("POST Method - HTTP/1.1 Compliance") {

	SUBCASE("Keep-Alive connection handling") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string keepAliveRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 12\r\n"
			"\r\n"
			"Test content";

		Request request(config, status, keepAliveRequest);
		Response response(&request);

		CHECK(request.isKeepAlive() == true);

		response.createHTTPHeaders();
		std::string headers = response.getHTTPHeaders();

		CHECK(headers.find("Connection: keep-alive") != std::string::npos);
	}

	SUBCASE("Connection: close handling") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string closeRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Connection: close\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 12\r\n"
			"\r\n"
			"Test content";

		Request request(config, status, closeRequest);
		Response response(&request);

		CHECK(request.isKeepAlive() == false);

		response.createHTTPHeaders();
		std::string headers = response.getHTTPHeaders();

		CHECK(headers.find("Connection: close") != std::string::npos);
	}

	SUBCASE("Content-Length validation") {
		ServerConf config = createMockServerConfig();
		Status status;

		SUBCASE("Correct Content-Length") {
			std::string correctLengthRequest =
				"POST /upload HTTP/1.1\r\n"
				"Host: localhost:8080\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 5\r\n"
				"\r\n"
				"Hello";

			Request request(config, status, correctLengthRequest);

			CHECK(request.getParsingState() == PARSING_DONE);
			CHECK(request.getBody() == "Hello");
			CHECK(!request.hasError());
		}

		SUBCASE("Missing Content-Length for POST") {
			std::string noLengthRequest =
				"POST /upload HTTP/1.1\r\n"
				"Host: localhost:8080\r\n"
				"Content-Type: text/plain\r\n"
				"\r\n"
				"Hello";

			Request request(config, status, noLengthRequest);

			// Should still parse but body should be empty or handled appropriately
			CHECK(request.getParsingState() == PARSING_DONE);
		}
	}
}

TEST_CASE("POST Method - Error Handling") {

	SUBCASE("Method not allowed") {
		std::map<std::string, std::string> params;
		params["listen"] = "8080";
		params["root"] = "/var/www";

		ServerConf config(params);
		Route getOnlyRoute;
		getOnlyRoute.setURLPath("/api");
		std::vector<std::string> getMethods;
		getMethods.push_back("GET");
		getOnlyRoute.setAllowedMethods(getMethods);
		config.addRoute(getOnlyRoute);

		Status status;

		std::string postRequest =
			"POST /api HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		Request request(config, status, postRequest);

		CHECK(request.hasError() == true);
		CHECK(request.getStatus().getStatusCode() == 405);
	}

	SUBCASE("Route not found") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string postRequest =
			"POST /nonexistent HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		Request request(config, status, postRequest);

		CHECK(request.hasError() == true);
		CHECK(request.getStatus().getStatusCode() == 404);
	}

	SUBCASE("Invalid request format") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string malformedRequest =
			"POST HTTP/1.1\r\n"  // Missing URL
			"Host: localhost:8080\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		Request request(config, status, malformedRequest);

		CHECK(request.hasError() == true);
		CHECK(request.getStatus().getStatusCode() == 400);
	}
}

TEST_CASE("POST Method - Real-world Scenarios") {

	SUBCASE("JSON API endpoint") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string jsonRequest =
			"POST /api/users HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: 45\r\n"
			"Accept: application/json\r\n"
			"\r\n"
			"{\"name\": \"John Doe\", \"email\": \"john@test.com\"}";

		Request request(config, status, jsonRequest);

		CHECK(request.getMethodAsString() == "POST");

		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["content-type"] == "application/json");
		CHECK(headers["accept"] == "application/json");

		std::string body = request.getBody();
		CHECK(body.find("\"name\": \"John Doe\"") != std::string::npos);
		CHECK(body.find("\"email\": \"john@test.com\"") != std::string::npos);
	}

	SUBCASE("File upload with progress") {
		ServerConf config = createMockServerConfig();
		Status status;

		// Simulate a file upload - C++98 compatible
		std::string binaryData = createLargeString(512, '\x42'); // 512 bytes of 'B'
		std::string lengthStr = intToString(static_cast<int>(binaryData.length()));

		std::string uploadRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Content-Length: " + lengthStr + "\r\n"
			"X-File-Name: test-binary.dat\r\n"
			"\r\n" + binaryData;

		Request request(config, status, uploadRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);
		CHECK(!request.hasError());

		std::string body = request.getBody();
		CHECK(body.length() == 512);
		CHECK(body == binaryData);

		// Verify custom header
		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["x-file-name"] == "test-binary.dat");
	}
}

// Integration test that simulates the full request-response cycle
TEST_CASE("POST Method - Full Integration Test") {

	SUBCASE("Complete POST workflow") {
		ServerConf config = createMockServerConfig();
		Status status;

		// 1. Parse request
		std::string postRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: 23\r\n"
			"Connection: keep-alive\r\n"
			"\r\n"
			"title=Test&content=Body";

		Request request(config, status, postRequest);

		// 2. Verify parsing completed successfully
		REQUIRE(request.getParsingState() == PARSING_DONE);
		REQUIRE(!request.hasError());
		REQUIRE(request.getMethodAsString() == "POST");

		// 3. Create response
		Response response(&request);

		// 4. Process with ContentFetcher (simplified)
		response.setContentType("text/html");
		std::string successContent =
			"<!DOCTYPE html>"
			"<html><head><title>Success</title></head>"
			"<body><h1>POST Successful</h1></body></html>";
		response.addToContent(successContent);
		response.setStatus(OK);

		// 5. Generate HTTP headers
		response.createHTTPHeaders();

		// 6. Verify final response
		std::string fullResponse = response.getHTTPResponse();

		CHECK(fullResponse.find("HTTP/1.1 200") != std::string::npos);
		CHECK(fullResponse.find("Content-Type: text/html") != std::string::npos);
		CHECK(fullResponse.find("Connection: keep-alive") != std::string::npos);
		CHECK(fullResponse.find("POST Successful") != std::string::npos);

		// Verify content length is correctly calculated
		CHECK(fullResponse.find("Content-Length:") != std::string::npos);
	}
}

// Test to simulate what curl would send
TEST_CASE("POST Method - Curl Simulation") {

	SUBCASE("curl -X POST -d 'name=test&value=123' http://localhost:8080/upload") {
		ServerConf config = createMockServerConfig();
		Status status;

		// This is what curl sends for the above command
		std::string curlRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"User-Agent: curl/7.68.0\r\n"
			"Accept: */*\r\n"
			"Content-Length: 18\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"\r\n"
			"name=test&value=123";

		Request request(config, status, curlRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getRequestedURL() == "/upload");
		CHECK(request.getBody() == "name=test&value=123");

		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["user-agent"] == "curl/7.68.0");
		CHECK(headers["accept"] == "*/*");
	}

	SUBCASE("curl -X POST -H 'Content-Type: application/json' -d '{\"key\":\"value\"}' http://localhost:8080/api") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string curlJsonRequest =
			"POST /api HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"User-Agent: curl/7.68.0\r\n"
			"Accept: */*\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: 15\r\n"
			"\r\n"
			"{\"key\":\"value\"}";

		Request request(config, status, curlJsonRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getRequestedURL() == "/api");
		CHECK(request.getBody() == "{\"key\":\"value\"}");

		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["content-type"] == "application/json");
	}
}

TEST_CASE("POST Method - Edge Cases") {

	SUBCASE("Empty POST body") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string emptyPostRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		Request request(config, status, emptyPostRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);
		CHECK(request.getBody().empty());
		CHECK(!request.hasError());
	}

	SUBCASE("POST with special characters in body") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string specialCharsBody = "name=Test%20User&message=Hello%2C%20World%21";
		std::string lengthStr = intToString(static_cast<int>(specialCharsBody.length()));

		std::string specialCharsRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: " + lengthStr + "\r\n"
			"\r\n" + specialCharsBody;

		Request request(config, status, specialCharsRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);
		CHECK(request.getBody() == specialCharsBody);
		CHECK(!request.hasError());
	}

	SUBCASE("Multiple header values") {
		ServerConf config = createMockServerConfig();
		Status status;

		std::string multiHeaderRequest =
			"POST /upload HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"Content-Type: application/json\r\n"
			"Accept: application/json, text/plain\r\n"
			"Accept-Language: en-US, en\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"{\"test\":true}";

		Request request(config, status, multiHeaderRequest);

		CHECK(request.getMethodAsString() == "POST");
		CHECK(request.getParsingState() == PARSING_DONE);
		CHECK(!request.hasError());

		std::map<std::string, std::string> headers = request.getAdditionalHeaderInfo();
		CHECK(headers["accept"] == "application/json, text/plain");
		CHECK(headers["accept-language"] == "en-us, en");
	}
}
