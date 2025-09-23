# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# include "doctest.h"

# include "server.hpp"

TEST_CASE("Testing : HTTP Request Class can extract the correct values") {

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


TEST_CASE("Testing : HTTP Request Class can use config to check if the method is allowed") {

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




// TEST_CASE("Testing : Config Parser is able to parse a config file") {

// 	SUBCASE("Config File can open a file") {

// 		std::string configFilePath = "./tests/nginx/conf/nginx.conf";


// 		CHECK(foundNull == true);

// 	}

// }
