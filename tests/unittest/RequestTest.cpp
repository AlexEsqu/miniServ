#include "doctest.h"
#include "Request.hpp"
#include "ServerConf.hpp"
#include "Status.hpp"
#include "Route.hpp"
#include "Router.hpp"

// Helper to create a mock ServerConf for testing purposes
static ServerConf createMockServerConf() {
	std::map<std::string, std::string> params;
	params["listen"] = "8080";
	params["root"] = "pages";
	params["client_max_body_size"] = "1000"; // 1KB limit for tests
	ServerConf conf(params);

	// Add a default route
	Route defaultRoute;
	defaultRoute.setURLPath("/");
	defaultRoute.setRootDirectory("pages");
	std::vector<std::string> methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	defaultRoute.setAllowedMethods(methods);
	conf.addRoute(defaultRoute);

	return conf;
}

TEST_CASE("Request Class Tests") {
	ServerConf conf = createMockServerConf();
	Status status;
	Request req(conf, status);

	SUBCASE("Initial State") {
		CHECK(req.getParsingState() == EMPTY);
		CHECK_FALSE(req.hasError());
		CHECK(req.getContentLength() == 0);
		CHECK(req.getMethodAsString().empty());
	}

	SUBCASE("Reset Functionality") {
		req.addRequestChunk("GET / HTTP/1.1\r\nHost: a\r\n\r\n");
		CHECK(req.getParsingState() == PARSING_DONE);
		CHECK(req.getMethodAsString() == "GET");

		req.reset();
		CHECK(req.getParsingState() == EMPTY);
		CHECK_FALSE(req.hasError());
		CHECK(req.getMethodAsString().empty());
		CHECK(req.getRequestedURL().empty());
		CHECK(req.getBody().empty());
	}

	SUBCASE("Request Line Parsing") {
		SUBCASE("Valid GET request line") {
			req.addRequestChunk("GET /index.html HTTP/1.1\r\n\r\n");
			CHECK_FALSE(req.hasError());
			CHECK(req.getMethodAsString() == "GET");
			CHECK(req.getMethodCode() == GET);
			CHECK(req.getRequestedURL() == "/index.html");
			CHECK(req.getProtocol() == "HTTP/1.1");
			CHECK(req.getParsingState() == PARSING_DONE);
		}

		SUBCASE("Request line with query string") {
			req.addRequestChunk("GET /script.php?user=alex HTTP/1.1\r\n\r\n");
			CHECK_FALSE(req.hasError());
			CHECK(req.getRequestedURL() == "/script.php");
			CHECK(req.getCgiParam() == "user=alex");
		}

		SUBCASE("Invalid request line - too few parts") {
			req.addRequestChunk("GET /index.html\r\n\r\n");
			CHECK(req.hasError());
			CHECK(req.getStatus().getStatusCode() == BAD_REQUEST);
		}

		SUBCASE("Invalid protocol") {
			req.addRequestChunk("GET /index.html HTTP/1.0\r\n\r\n");
			CHECK(req.hasError());
			CHECK(req.getStatus().getStatusCode() == HTTP_VERSION_NOT_SUPPORTED);
		}

		SUBCASE("Unsupported method") {
			req.addRequestChunk("OPTIONS /index.html HTTP/1.1\r\n\r\n");
			CHECK(req.hasError());
			CHECK(req.getStatus().getStatusCode() == METHOD_NOT_ALLOWED);
		}
	}

	SUBCASE("Header Parsing") {
		req.addRequestChunk("POST /submit HTTP/1.1\r\n");
		req.addRequestChunk("Host: localhost\r\n");
		req.addRequestChunk("Content-Length: 50\r\n");
		req.addRequestChunk("Content-Type: text/plain\r\n");
		req.addRequestChunk("Connection: keep-alive\r\n");
		req.addRequestChunk("\r\n");

		CHECK_FALSE(req.hasError());
		CHECK(req.getParsingState() == PARSING_BODY);
		CHECK(req.getContentLength() == 50);
		CHECK(req.getContentType() == "text/plain");
		CHECK(req.isKeepAlive());
	}

	SUBCASE("Body Assembly - Unchunked (Content-Length)") {
		SUBCASE("Body received in one chunk") {
			req.addRequestChunk("POST / HTTP/1.1\r\nContent-Length: 11\r\n\r\nHello World");
			CHECK(req.getParsingState() == PARSING_DONE);
			CHECK(req.getBody() == "Hello World");
			CHECK(req.getBodyBuffer().getBufferSize() == 11);
		}

		SUBCASE("Body received in multiple chunks") {
			req.addRequestChunk("POST / HTTP/1.1\r\nContent-Length: 11\r\n\r\n");
			req.addRequestChunk("Hello");
			CHECK(req.getParsingState() == PARSING_BODY);
			req.addRequestChunk(" World");
			CHECK(req.getParsingState() == PARSING_DONE);
			CHECK(req.getBody() == "Hello World");
		}

		SUBCASE("Payload Too Large") {
			// max size is 1000
			req.addRequestChunk("POST / HTTP/1.1\r\nContent-Length: 1001\r\n\r\n");
			CHECK(req.hasError());
			CHECK(req.getStatus().getStatusCode() == PAYLOAD_TOO_LARGE);
			// Even though the error is set, the state becomes PARSING_DONE
			CHECK(req.getParsingState() == PARSING_DONE);
		}
	}

	SUBCASE("Body Assembly - Chunked") {
		SUBCASE("Body with multiple chunks") {
			req.addRequestChunk("POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
			CHECK(req.getParsingState() == PARSING_BODY);

			req.addRequestChunk("7\r\nchunk 1\r\n");
			req.addRequestChunk("8\r\n chunk 2\r\n");
			req.addRequestChunk("0\r\n\r\n");

			CHECK(req.getParsingState() == PARSING_DONE);
			CHECK(req.getBody() == "chunk 1 chunk 2");
		}

		SUBCASE("Chunk data split across reads") {
			req.addRequestChunk("POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
			req.addRequestChunk("10\r\n"); // 16 bytes
			req.addRequestChunk("01234567");
			CHECK(req.getParsingState() == PARSING_BODY);
			req.addRequestChunk("89abcdef\r\n");
			req.addRequestChunk("0\r\n\r\n");

			CHECK(req.getParsingState() == PARSING_DONE);
			CHECK(req.getBody() == "0123456789abcdef");
		}

		SUBCASE("Payload Too Large - Chunked") {
			req.addRequestChunk("POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
			req.addRequestChunk("400\r\n"); // 1024 in hex
			std::string largeChunk(1024, 'A');
			req.addRequestChunk(largeChunk + "\r\n");

			CHECK(req.hasError());
			CHECK(req.getStatus().getStatusCode() == PAYLOAD_TOO_LARGE);
			CHECK(req.getParsingState() == PARSING_DONE);
		}
	}

	SUBCASE("Session and Cookie Handling") {
		SUBCASE("Request with Session: yes header creates a session") {
			req.addRequestChunk("GET / HTTP/1.1\r\nSession: yes\r\n\r\n");
			CHECK(req.hasSessionId());
			CHECK(req.getSessionId() != 0);
			CHECK_FALSE(conf.getSessionMap().empty());
			CHECK(conf.getSessionMap().count(req.getSessionId()) == 1);
		}

		SUBCASE("Request with existing session_id cookie reuses session") {
			// Manually create a session to simulate a previous visit
			size_t existingSessionId = 12345;
			conf.getSessionMap()[existingSessionId] = Session(existingSessionId);
			conf.getSessionMap()[existingSessionId].addCookie("some_data=value");

			std::stringstream request;
			request << "GET / HTTP/1.1\r\nCookie: session_id=" << existingSessionId << "\r\n\r\n";
			req.addRequestChunk(request.str());

			CHECK(req.hasSessionId());
			CHECK(req.getSessionId() == existingSessionId);
			// Check that the original session map was modified
			CHECK(conf.getSessionMap()[existingSessionId].getCookies().count("some_data") == 1);
		}
	}
}
