# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# include "doctest.h"

# include "server.hpp"

TEST_CASE("Testing : HTTP Request Class can extract the correct values") {

	SUBCASE("Valid GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "./");
		CHECK(request.getProtocol() == "HTTP/1.1");
	}

	SUBCASE("Valid minimalist GET root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "./");
		CHECK(request.getProtocol() == "HTTP/1.1");
	}

	SUBCASE("Valid GET page request") {

		const char* HTTPRequest =
		"GET /pages/error.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		CHECK(request.getMethod() == "GET");
		CHECK(request.getRequestedURL() == "./pages/error.html");
		CHECK(request.getProtocol() == "HTTP/1.1");
	}

	SUBCASE("Invalid Protocol Request") {

		const char* HTTPRequest =
		"GET / HTTP/1.3\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(HTTPRequest), Request::badProtocol);
	}

	SUBCASE("Forbidden Method Request") {

		const char* HTTPRequest =
		"LAUNCH / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(HTTPRequest), Request::forbiddenMethod);
	}

	SUBCASE("Unsupported Method Request") {

		const char* HTTPRequest =
		"UPDATE / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		CHECK_THROWS_AS(Request request(HTTPRequest), Request::forbiddenMethod);
	}

}

TEST_CASE("Testing : HTTP Request Class set if CGI is needed or not") {

	SUBCASE("root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		request.setCGI();
		CHECK(request.getCGI() == false);
	}

	SUBCASE("html request") {

		const char* HTTPRequest =
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		request.setCGI();
		CHECK(request.getCGI() == false);
	}

	SUBCASE("python request") {

		const char* HTTPRequest =
		"GET /script.py HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		request.setCGI();
		CHECK(request.getCGI() == PY);
	}

		SUBCASE("php request") {

		const char* HTTPRequest =
		"GET /index.php HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		request.setCGI();
		CHECK(request.getCGI() == PHP);
	}
}
