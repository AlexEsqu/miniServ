#include "doctest.h"
#include "Route.hpp"
#include "Router.hpp"
#include "ServerConf.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>

// Helper to find the best matching route from a ServerConf object,
// mimicking the logic the server should use.
static const Route* findBestMatchInConf(const ServerConf& conf, const std::string& path) {
	const std::vector<Route>& routes = conf.getRoutes();
	const Route* bestMatch = NULL;

	for (size_t i = 0; i < routes.size(); ++i) {
		if (routes[i].isPathMatch(path)) {
			if (bestMatch == NULL || routes[i].getURLPath().length() > bestMatch->getURLPath().length()) {
				bestMatch = &routes[i];
			}
		}
	}
	return bestMatch;
}


// Helper to create a mock ServerConf with a realistic route setup
static ServerConf createMockRoutingServerConf() {
	std::map<std::string, std::string> serverParams;
	serverParams["listen"] = "8080";
	serverParams["root"] = "pages";
	serverParams["index"] = "index.html";
	serverParams["client_max_body_size"] = "1000000";
	ServerConf conf(serverParams);

	// The default route "/" is created by the ServerConf constructor.
	// We can retrieve and modify it for tests if needed.
	std::vector<Route>& routes = const_cast<std::vector<Route>&>(conf.getRoutes());
	for (size_t i = 0; i < routes.size(); ++i) {
		if (routes[i].getURLPath() == "/") {
			routes[i].setRootDirectory("pages");
			std::vector<std::string> defaultFiles;
			defaultFiles.push_back("index.html");
			routes[i].setDefaultFiles(defaultFiles);
			break;
		}
	}

	// Add specific routes for testing
	Route apiRoute;
	apiRoute.setURLPath("/api");
	apiRoute.setRootDirectory("api_root");
	std::vector<std::string> apiMethods;
	apiMethods.push_back("GET");
	apiMethods.push_back("POST");
	apiMethods.push_back("DELETE");
	apiRoute.setAllowedMethods(apiMethods);
	conf.addRoute(apiRoute);

	Route postRoute;
	postRoute.setURLPath("/post");
	postRoute.setUploadDirectory("uploads");
	std::vector<std::string> postMethods;
	postMethods.push_back("POST");
	postRoute.setAllowedMethods(postMethods);
	conf.addRoute(postRoute);

	Route privateRoute;
	privateRoute.setURLPath("/private");
	privateRoute.setRootDirectory("private_pages");
	privateRoute.setAutoIndex(false);
	conf.addRoute(privateRoute);

	return conf;
}

// Helper to create a dummy file system for tests
static void createMockFileSystem() {
	mkdir("pages", 0755);
	mkdir("api_root", 0755);
	mkdir("uploads", 0755);
	mkdir("private_pages", 0755);
	std::ofstream("pages/index.html").close();
	std::ofstream("pages/style.css").close();
	std::ofstream("api_root/data.json").close();
}

// Helper to clean up the dummy file system
static void cleanupMockFileSystem() {
	remove("pages/index.html");
	remove("pages/style.css");
	remove("api_root/data.json");
	rmdir("pages");
	rmdir("api_root");
	rmdir("uploads");
	rmdir("private_pages");
}

TEST_CASE("Route and Router Tests") {
	createMockFileSystem();

	ServerConf conf = createMockRoutingServerConf();
	Status status;

	SUBCASE("Router Utility Functions") {
		SUBCASE("replaceRoutePathByRootDirectory") {
			const Route* apiRoute = findBestMatchInConf(conf, "/api/resource");
			REQUIRE(apiRoute != NULL);
			std::string result = Router::replaceRoutePathByRootDirectory("/api/resource", apiRoute);
			CHECK(result == "api_root/resource");
		}
	}

	SUBCASE("Router Core Logic") {
		SUBCASE("Finding best top-level match in ServerConf") {
			CHECK(findBestMatchInConf(conf, "/api/v1/users")->getURLPath() == "/api");
			CHECK(findBestMatchInConf(conf, "/post/image.jpg")->getURLPath() == "/post");
			CHECK(findBestMatchInConf(conf, "/style.css")->getURLPath() == "/");
			CHECK(findBestMatchInConf(conf, "/private/file")->getURLPath() == "/private");
		}

		SUBCASE("routeFilePathForGet") {
			// Test 1: Direct file path
			Request req1(conf, status);
			req1.addRequestChunk("GET /style.css HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req1.setRoute(findBestMatchInConf(conf, req1.getRequestedURL()));
			CHECK(Router::routeFilePathForGet(req1.getRequestedURL(), req1) == "pages/style.css");

			// Test 2: Directory with default file
			Request req2(conf, status);
			req2.addRequestChunk("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req2.setRoute(findBestMatchInConf(conf, req2.getRequestedURL()));
			CHECK(Router::routeFilePathForGet(req2.getRequestedURL(), req2) == "pages/index.html");

			// Test 3: Directory with autoindex on (no default file)
			Request req3(conf, status);
			req3.addRequestChunk("GET /api/ HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req3.setRoute(findBestMatchInConf(conf, req3.getRequestedURL()));
			CHECK(Router::routeFilePathForGet(req3.getRequestedURL(), req3) == "api_root/");

			// Test 4: Directory with autoindex off (no default file)
			Request req4(conf, status);
			req4.addRequestChunk("GET /private/ HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req4.setRoute(findBestMatchInConf(conf, req4.getRequestedURL()));
			CHECK(Router::routeFilePathForGet(req4.getRequestedURL(), req4).empty());
		}

		SUBCASE("validateRequestWithRoute") {

			{	// Valid request
			Request req1(conf, status);
			Response res(req1, status);
			req1.addRequestChunk("GET /api HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req1.setRoute(findBestMatchInConf(conf, req1.getRequestedURL()));
			Router::validateRequestWithRoute(&req1, &res);
			CHECK_FALSE(res.hasError());
			}
			{
			// Invalid method
			Request req2(conf, status);
			Response res(req2, status);
			// The /api route in the mock config does not allow DELETE
			req2.addRequestChunk("DELETE /api HTTP/1.1\r\nHost: localhost\r\n\r\n");
			req2.setRoute(findBestMatchInConf(conf, req2.getRequestedURL()));
			Router::validateRequestWithRoute(&req2, &res);
			CHECK(res.hasError());
			CHECK(res.getStatus().getStatusCode() == METHOD_NOT_ALLOWED);
			}
		}
	cleanupMockFileSystem();
	}
}
