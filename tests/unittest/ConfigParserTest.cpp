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
	conf.addRoute(apiRoute);

	Route postRoute;
	postRoute.setURLPath("/post");
	postRoute.setUploadDirectory("uploads");
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

	SUBCASE("Route::getMatchingRoute (Nested Logic)") {
		// Test the Route class's recursive matching directly
		Route parentRoute;
		parentRoute.setURLPath("/api");

		Route childRoute;
		childRoute.setURLPath("/api/v1");

		Route grandchildRoute;
		grandchildRoute.setURLPath("/api/v1/users");

		// Create the nested structure
		childRoute.addNestedRoute(grandchildRoute);
		parentRoute.addNestedRoute(childRoute);

		// Test that the most specific (longest) path is returned
		const Route* match = parentRoute.getMatchingRoute("/api/v1/users/123");
		REQUIRE(match != NULL);
		CHECK(match->getURLPath() == "/api/v1/users");

		// Test that a less specific path matches the appropriate parent
		match = parentRoute.getMatchingRoute("/api/v1/status");
		REQUIRE(match != NULL);
		CHECK(match->getURLPath() == "/api/v1");

		// Test that a path matching only the top-level route works
		match = parentRoute.getMatchingRoute("/api/info");
		REQUIRE(match != NULL);
		CHECK(match->getURLPath() == "/api");

		// Test a non-matching path
		match = parentRoute.getMatchingRoute("/other");
		CHECK(match == NULL);
	}

	SUBCASE("Finding best top-level match in ServerConf") {
		// This tests the logic of finding the best match among the ServerConf's direct routes
		CHECK(findBestMatchInConf(conf, "/api/some/path")->getURLPath() == "/api");
		CHECK(findBestMatchInConf(conf, "/post/upload")->getURLPath() == "/post");
		CHECK(findBestMatchInConf(conf, "/index.html")->getURLPath() == "/");
		CHECK(findBestMatchInConf(conf, "/private/secret.html")->getURLPath() == "/private");
	}

	SUBCASE("Router::routeFilePathForGet") {
		// Test 1: Direct file path
		Request req1(conf, status);
		req1.addRequestChunk("GET /style.css HTTP/1.1\r\n\r\n");
		req1.setRoute(findBestMatchInConf(conf, req1.getRequestedURL()));
		CHECK(Router::routeFilePathForGet(req1.getRequestedURL(), req1) == "pages/style.css");

		// Test 2: Directory with default file
		Request req2(conf, status);
		req2.addRequestChunk("GET / HTTP/1.1\r\n\r\n");
		req2.setRoute(findBestMatchInConf(conf, req2.getRequestedURL()));
		CHECK(Router::routeFilePathForGet(req2.getRequestedURL(), req2) == "pages/index.html");

		// Test 3: Directory with autoindex on (and no default file configured for this route)
		Request req3(conf, status);
		req3.addRequestChunk("GET /api/ HTTP/1.1\r\n\r\n");
		req3.setRoute(findBestMatchInConf(conf, req3.getRequestedURL()));
		CHECK(Router::routeFilePathForGet(req3.getRequestedURL(), req3) == "api_root/");

		// Test 4: Directory with autoindex off (and no default file)
		Request req4(conf, status);
		req4.addRequestChunk("GET /private/ HTTP/1.1\r\n\r\n");
		req4.setRoute(findBestMatchInConf(conf, req4.getRequestedURL()));
		CHECK(Router::routeFilePathForGet(req4.getRequestedURL(), req4).empty());
	}

	cleanupMockFileSystem();
}
