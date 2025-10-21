# include "doctest.h"
# include "server.hpp"

static Route createDefaultRoute() {
	Route defaultRoute;
	defaultRoute.setURLPath("/");
	defaultRoute.setAllowedMethods({"GET", "POST", "PUT", "DELETE"});
	defaultRoute.setRootDirectory("pages");
	defaultRoute.setDefaultFiles({"index.html"});
	defaultRoute.setAllowedCGI({".php", ".py"});
	defaultRoute.setUploadDirectory("uploads");
	return defaultRoute;
}

static ServerConf generateDefaultConfig() {
	ServerConf config;
	config.setPort(8080);
	config.setRoot("pages");
	config.addRoute(createDefaultRoute());
	return config;
}

TEST_CASE("Request constructor extracts correct values") {

	ServerConf	config;
	Status		stat;

	SUBCASE("Valid GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(config, stat, HTTPRequest);
		CHECK(request.getMethodAsString() == "GET");
		CHECK(request.getRequestedURL() == "/");
		CHECK(request.getProtocol() == "HTTP/1.1");
		CHECK(request.getAdditionalHeaderInfo()["host"] == "localhost:8080");
	}

	SUBCASE("Valid minimalist GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"\r\n";

		Request	request(config, stat, HTTPRequest);
		CHECK(request.getMethodAsString() == "GET");
		CHECK(request.getMethodCode() == GET);
		CHECK(request.getRequestedURL() == "/");
		CHECK(request.getProtocol() == "HTTP/1.1");
	}

	SUBCASE("Valid GET page request") {

		const char* HTTPRequest =
		"GET /pages/error.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n"
		"Body: Hello world\r\n";

		Request	request(config, stat, HTTPRequest);
		CHECK(request.getMethodAsString() == "GET");
		CHECK(request.getMethodCode() == GET);
		CHECK(request.getRequestedURL() == "/pages/error.html");
		CHECK(request.getProtocol() == "HTTP/1.1");
		CHECK(request.getAdditionalHeaderInfo()["host"] == "localhost:8080");
	}

	SUBCASE("Invalid Protocol Request") {

		const char* HTTPRequest =
		"GET / HTTP/1.3\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request request(config, stat, HTTPRequest);
		CHECK(request.getStatus().getStatusCode() == HTTP_VERSION_NOT_SUPPORTED);
	}

}


TEST_CASE("Request constructor use config to check method") {

	ServerConf	config;
	Status		stat;

	SUBCASE("Unknown Method Request") {

		const char* HTTPRequest =
		"LAUNCH / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request request(config, stat, HTTPRequest);
		CHECK(request.getMethodAsString() == "LAUNCH");
		CHECK(request.getMethodCode() == UNSUPPORTED);
		CHECK(request.getStatus().getStatusCode() == METHOD_NOT_ALLOWED);
	}

	SUBCASE("Unsupported Method Request") {

		const char* HTTPRequest =
		"UPDATE / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request request(config, stat, HTTPRequest);
		CHECK(request.getMethodAsString() == "UPDATE");
		CHECK(request.getMethodCode() == UNSUPPORTED);
		CHECK(request.getStatus().getStatusCode() == METHOD_NOT_ALLOWED);
	}

}

TEST_CASE("Incremental parsing of normal HTTP request") {
	ServerConf config;
	Status		stat;
	Request req(config, stat, "");

	// Simulate receiving the request line in two chunks
	req.addRequestChunk("GET /index.html HTTP/1.1\r\nHo");
	CHECK(req.getParsingState() == PARSING_HEADERS);
	CHECK(req.getMethodAsString() == "GET");
	CHECK(req.getRequestedURL() == "/index.html");
	CHECK(req.getProtocol() == "HTTP/1.1");

	// Simulate receiving the rest of the headers and body
	req.addRequestChunk("st: localhost\r\nContent-Length: 5\r\n\r\nHello");
	CHECK(req.getAdditionalHeaderInfo()["host"] == "localhost");
	CHECK(req.getAdditionalHeaderInfo()["content-length"] == "5");
	CHECK(req.getParsingState() == PARSING_DONE);
}

TEST_CASE("Incremental parsing of chunked HTTP request") {
	ServerConf config = generateDefaultConfig();
	Status		stat;
	Request req(config, stat, "");

	// Simulate receiving request line and headers in chunks
	req.addRequestChunk("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n");
	CHECK(req.getParsingState() == PARSING_BODY);

	// Simulate receiving chunked body in two chunks
	// Chunk: "5\r\nHello\r\n0\r\n\r\n"
	req.addRequestChunk("5\r\nHe");
	CHECK(req.getParsingState() == PARSING_BODY);

	req.addRequestChunk("llo\r\n0\r\n\r\n");
	CHECK(req.getParsingState() == PARSING_DONE);
}

TEST_CASE("Parsing request with headers arriving one line at a time") {
	ServerConf config;
	Status		stat;
	Request req(config, stat, "");

	req.addRequestChunk("GET / HTTP/1.1\r\n");
	CHECK(req.getParsingState() == PARSING_HEADERS);

	req.addRequestChunk("Host: localhost\r\n");
	CHECK(req.getAdditionalHeaderInfo()["host"] == "localhost");

	req.addRequestChunk("Content-Length: 4\r\n");
	CHECK(req.getAdditionalHeaderInfo()["content-length"] == "4");

	req.addRequestChunk("\r\nTest");
	CHECK(req.getParsingState() == PARSING_DONE);
}

TEST_CASE("Parsing request with no body") {
	ServerConf config;
	Status		stat;
	Request req(config, stat, "");

	req.addRequestChunk("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
	CHECK(req.getParsingState() == PARSING_DONE);
	CHECK(req.getMethodAsString() == "GET");
	CHECK(req.getMethodCode() == GET);
	CHECK(req.getRequestedURL() == "/");
	CHECK(req.getProtocol() == "HTTP/1.1");
	CHECK(req.getAdditionalHeaderInfo()["host"] == "localhost");
}

TEST_CASE("Parsing HTTP request with delimiter in body") {
	ServerConf config = generateDefaultConfig();
	Status		stat;
	Request req(config, stat, "");

	req.addRequestChunk(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Length: 70\r\n"
		"\r\n"
	);
	CHECK(req.getMethodAsString() == "POST");
	CHECK(req.getMethodCode() == POST);
	CHECK(req.getParsingState() == PARSING_BODY);

	req.addRequestChunk(
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
		"\r\n"
		"Hello, world!\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n"
	);

	CHECK(req.getParsingState() == PARSING_DONE);

	// Optionally, check that the body contains the delimiter
	// CHECK(req.getBody().find("------WebKitFormBoundary7MA4YWxkTrZu0gW") != std::string::npos);
}
