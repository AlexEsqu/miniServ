# include "doctest.h"
# include "server.hpp"

TEST_CASE("Request constructor extracts correct values") {

	ServerConf	config;

	SUBCASE("Valid GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(config, HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "/");
		CHECK(request.getProtocol() == "HTTP/1.1");
		CHECK(request.getAdditionalHeaderInfo()["Host"] == "localhost:8080");
	}

	SUBCASE("Valid minimalist GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"\r\n";

		Request	request(config, HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "/");
		CHECK(request.getProtocol() == "HTTP/1.1");
	}

	SUBCASE("Valid GET page request") {

		const char* HTTPRequest =
		"GET /pages/error.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n"
		"Body: Hello world\r\n";

		Request	request(config, HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "/pages/error.html");
		CHECK(request.getProtocol() == "HTTP/1.1");
		CHECK(request.getAdditionalHeaderInfo()["Host"] == "localhost:8080");
	}

	SUBCASE("Invalid Protocol Request") {

		const char* HTTPRequest =
		"GET / HTTP/1.3\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(config, HTTPRequest), badProtocol);
	}

}


TEST_CASE("Request constructor use config to check method") {

	ServerConf	config;

	SUBCASE("Unknown Method Request") {

		const char* HTTPRequest =
		"LAUNCH / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(config, HTTPRequest), forbiddenMethod);
	}

	SUBCASE("Unsupported Method Request") {

		const char* HTTPRequest =
		"UPDATE / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(config, HTTPRequest), forbiddenMethod);
	}

}
