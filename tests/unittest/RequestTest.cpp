# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# include "doctest.h"

# include "server.hpp"

TEST_CASE("Testing : HTTP Request Class can extract the correct values") {

	SUBCASE("Valid GET Request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "./");
		CHECK(request.getProtocol() == "HTTP/1.1");

	}

	SUBCASE("Invalid GET Request") {

		const char* HTTPRequest =
		"GET / HTTP/1.3\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(HTTPRequest), Request::badProtocol);
	}

	// SUBCASE("Valid GET Cookie Request") {

	// const char* HTTPRequest =
	// "GET / HTTP/1.1\r\n"
	// "Host: localhost:8080\r\n"
	// "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:142.0) Gecko/20100101 Firefox/142.0\r\n"
	// "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
	// "Accept-Language: en-US,en;q=0.5\r\n"
	// "Accept-Encoding: gzip, deflate, br, zstd\r\n"
	// "Sec-GPC: 1\r\n"
	// "Connection: keep-alive\r\n"
	// "Cookie: wp-settings-time-1=1755175009\r\n"
	// "Priority: u=0, i\r\n"
	// "\r\n";

	// Request	request(HTTPRequest);


	// }



	// CHECK(factorial(0) == 1);
}
