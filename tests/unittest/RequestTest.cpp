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

	// SUBCASE("Forbidden Method Request") {

	// 	const char* HTTPRequest =
	// 	"LAUNCH / HTTP/1.1\r\n"
	// 	"Host: localhost:8080\r\n"
	// 	"\r\n";

	// 	CHECK_THROWS_AS(Request request(HTTPRequest), Request::forbiddenMethod);
	// }

	// SUBCASE("Unsupported Method Request") {

	// 	const char* HTTPRequest =
	// 	"UPDATE / HTTP/1.1\r\n"
	// 	"Host: localhost:8080\r\n"
	// 	"\r\n";

	// 	CHECK_THROWS_AS(Request request(HTTPRequest), Request::forbiddenMethod);
	// }

}

TEST_CASE("Testing : HTTP Request Class set if CGI is needed or not") {

	SUBCASE("root request") {

		const char* HTTPRequest =
		"GET / HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"\r\n";

		Request	request(HTTPRequest);
		request.setCGI();
		CHECK(request.getCGI() == false); //this test does not make sense because it will depend of if the root will be redirected to an html or a php file
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

TEST_CASE("Testing : Environment Builder is able to extract the correct key values") {

	SUBCASE("HTTP variable of host") {

		std::string keyValueAsString = "Host: localhost:8080";
		std::string keyValueFormattedAsString = "HTTP_HOST=localhost:8080";
		std::string key = "HTTP_HOST";
		std::string value = "localhost:8080";

		EnvironmentBuilder	testEnv;

		testEnv.cutFormatAddToEnv(keyValueAsString);

		CHECK(testEnv.getSpecificEnv(key) == keyValueFormattedAsString);
	}

	SUBCASE("Content type variable") {

		std::string keyValueAsString = "Content-type: text/plain";
		std::string keyValueFormattedAsString = "CONTENT_TYPE=text/plain";
		std::string key = "CONTENT_TYPE";
		std::string value = "text/plain";

		EnvironmentBuilder	testEnv;

		testEnv.cutFormatAddToEnv(keyValueAsString);

		CHECK(testEnv.getSpecificEnv(key) == keyValueFormattedAsString);
	}

		SUBCASE("weird non trimmed variable") {

		std::string keyValueAsString = "   Content-type:   text/plain\n";
		std::string keyValueFormattedAsString = "CONTENT_TYPE=text/plain";
		std::string key = "CONTENT_TYPE";
		std::string value = "text/plain";

		EnvironmentBuilder	testEnv;

		testEnv.cutFormatAddToEnv(keyValueAsString);

		CHECK(testEnv.getSpecificEnv(key) == keyValueFormattedAsString);
	}

}

TEST_CASE("Testing : Environment Builder is able to return Environement") {

	SUBCASE("HTTP variable of host") {

		std::string keyValueAsString = "Host: localhost:8080";
		std::string keyValueFormattedAsString = "HTTP_HOST=localhost:8080";
		std::string key = "HTTP_HOST";
		std::string value = "localhost:8080";

		EnvironmentBuilder	testEnv;

		testEnv.cutFormatAddToEnv(keyValueAsString);
		assert(testEnv.getSpecificEnv(key) == keyValueFormattedAsString);

		// Get the environment
		Environment env = testEnv.getPHPEnv();
		char** envArray = env.getEnv();

		// Check that we have at least one environment variable
		CHECK(envArray != NULL);
		CHECK(envArray[0] != NULL);

		// Check the content of the first environment variable
		std::string firstEnvVar(envArray[0]);
		CHECK(firstEnvVar == keyValueFormattedAsString);

		// Check that the array is NULL-terminated
		bool foundNull = false;
		for (int i = 0; envArray[i] != NULL && i < 100; i++) {
			if (envArray[i+1] == NULL) {
				foundNull = true;
				break;
			}
		}
		CHECK(foundNull == true);

	}

	SUBCASE("Multiple environment variables") {

		EnvironmentBuilder testEnv;

		// Add multiple variables
		std::string hostHeader = "Host: localhost:8080";
		std::string contentHeader = "Content-Type: text/plain";

		testEnv.cutFormatAddToEnv(hostHeader);
		testEnv.cutFormatAddToEnv(contentHeader);

		Environment env = testEnv.getPHPEnv();
		char** envArray = env.getEnv();

		// Check that we have environment variables
		CHECK(envArray != NULL);
		CHECK(envArray[0] != NULL);
		CHECK(envArray[1] != NULL);

		// Convert to strings for easier comparison
		std::vector<std::string> envStrings;
		for (int i = 0; envArray[i] != NULL; i++) {
			envStrings.push_back(std::string(envArray[i]));
		}

		// Check that we have the expected variables (order might vary due to map)
		bool foundHost = false;
		bool foundContent = false;

		for (size_t i = 0; i < envStrings.size(); i++) {
			if (envStrings[i] == "HTTP_HOST=localhost:8080") {
				foundHost = true;
			}
			if (envStrings[i] == "CONTENT_TYPE=text/plain") {
				foundContent = true;
			}
		}

		CHECK(foundHost == true);
		CHECK(foundContent == true);
	}

	SUBCASE("Empty environment") {

		EnvironmentBuilder testEnv;
		Environment env = testEnv.getPHPEnv();
		char** envArray = env.getEnv();

		if (envArray != NULL) {
			CHECK(envArray[0] == NULL);
		}
	}

}


TEST_CASE("Testing : Config Parser is able to parse a config file") {

	SUBCASE("Config File can open a file") {

		std::string configFilePath = "./tests/nginx/conf/nginx.conf";


		CHECK(foundNull == true);

	}

}
