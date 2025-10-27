#include "doctest.h"
#include "server.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>

// Helper function to create temporary config files for testing
static void createTempConfigFile(const std::string& filename, const std::string& content) {
	std::ofstream file(filename);
	file << content;
	file.close();
}

TEST_CASE("CONFIG PARSING")
{

	SUBCASE("ConfigParser utility functions") {

		SUBCASE("isClosedCurlyBrace function") {
			std::string closingBrace = "}";
			std::string closingBraceWithSpaces = "  }  ";
			std::string notClosingBrace = "{ something";
			std::string empty = "";

			CHECK(ConfigParser::isClosedCurlyBrace(closingBrace) == true);
			CHECK(ConfigParser::isClosedCurlyBrace(closingBraceWithSpaces) == true);
			CHECK(ConfigParser::isClosedCurlyBrace(notClosingBrace) == false);
			CHECK(ConfigParser::isClosedCurlyBrace(empty) == false);
		}

		SUBCASE("addLineAsServerKeyValue function") {
			std::map<std::string, std::string> paramMap;

			// Test normal key-value pair
			std::string line1 = "listen 8080;";
			ConfigParser::addLineAsServerKeyValue(line1, paramMap);
			CHECK(paramMap["listen"] == "8080");

			// Test key-value with spaces
			std::string line2 = "  server_name   example.com  ;  ";
			ConfigParser::addLineAsServerKeyValue(line2, paramMap);
			CHECK(paramMap["server_name"] == "example.com");

			// Test without semicolon
			std::string line3 = "root /var/www/html";
			ConfigParser::addLineAsServerKeyValue(line3, paramMap);
			CHECK(paramMap["root"] == "/var/www/html");

			// Test empty line (should be ignored)
			std::string line4 = "";
			size_t sizeBefore = paramMap.size();
			ConfigParser::addLineAsServerKeyValue(line4, paramMap);
			CHECK(paramMap.size() == sizeBefore);
		}
	}

	SUBCASE("ConfigParser parseServerBlock function") {

		SUBCASE("Simple server block parsing") {
			std::string configContent =
				"listen 8080;\n"
				"root /var/www/html;\n"
				"}\n";

			std::stringstream ss(configContent);
			std::ifstream mockStream;

			createTempConfigFile("test_simple.conf",
				"server {\n" + configContent);

			std::ifstream configFile("test_simple.conf");
			std::string line;
			getline(configFile, line);

			ServerConf result = ConfigParser::parseServerBlock(configFile);

			CHECK(result.getPort() == 8080);
			CHECK(result.getRoot() == "/var/www/html");

			std::remove("test_simple.conf");
		}

		SUBCASE("Server block with comments and empty lines") {
			createTempConfigFile("test_comments.conf",
				"server {\n"
				"    # This is a comment\n"
				"    listen 9000;\n"
				"    \n"
				"    # Another comment\n"
				"    root /home/user/www;\n"
				"}\n");

			std::ifstream configFile("test_comments.conf");
			std::string line;
			getline(configFile, line);

			ServerConf result = ConfigParser::parseServerBlock(configFile);

			CHECK(result.getPort() == 9000);
			CHECK(result.getRoot() == "/home/user/www");

			std::remove("test_comments.conf");
		}

		SUBCASE("Server block with location block (should be skipped)") {
			createTempConfigFile("test_location.conf",
				"server {\n"
				"    listen 3000;\n"
				"    root /app;\n"
				"    location / {\n"
				"        try_files $uri $uri/ =404;\n"
				"    }\n"
				"}\n");

			std::ifstream configFile("test_location.conf");
			std::string line;
			getline(configFile, line);

			ServerConf result = ConfigParser::parseServerBlock(configFile);

			CHECK(result.getPort() == 3000);
			CHECK(result.getRoot() == "/app");

			std::remove("test_location.conf");
		}
	}

	SUBCASE("ConfigParser parseConfigFile function") {

		SUBCASE("Single server configuration") {
			createTempConfigFile("test_single_server.conf",
				"# Nginx configuration\n"
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"}\n");

			std::string configPath = "test_single_server.conf";
			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 1);
			CHECK(configs[0].getPort() == 8080);
			CHECK(configs[0].getRoot() == "/var/www");

			std::remove("test_single_server.conf");
		}

		SUBCASE("Multiple server configurations") {
			createTempConfigFile("test_multiple_servers.conf",
				"# First server\n"
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www/site1;\n"
				"}\n"
				"\n"
				"# Second server\n"
				"server {\n"
				"    listen 8081;\n"
				"    root /var/www/site2;\n"
				"}\n");

			const std::string configPath = "test_multiple_servers.conf";
			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 2);
			CHECK(configs[0].getPort() == 8080);
			CHECK(configs[0].getRoot() == "/var/www/site1");
			CHECK(configs[1].getPort() == 8081);
			CHECK(configs[1].getRoot() == "/var/www/site2");

			std::remove("test_multiple_servers.conf");
		}

		SUBCASE("Configuration with global directives (should be ignored)") {
			createTempConfigFile("test_global_directives.conf",
				"user nginx;\n"
				"worker_processes 1;\n"
				"\n"
				"events {\n"
				"    worker_connections 1024;\n"
				"}\n"
				"\n"
				"http {\n"
				"    server {\n"
				"        listen 8080;\n"
				"        root /var/www;\n"
				"    }\n"
				"}\n");

			ConfigParser parser;
			std::string configPath = "test_global_directives.conf";
			std::vector<ServerConf> configs = parser.parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 1);
			CHECK(configs[0].getPort() == 8080);

			std::remove("test_global_directives.conf");
		}

		SUBCASE("Empty configuration file") {
			createTempConfigFile("test_empty.conf", "");

			ConfigParser parser;
			std::string configPath = "test_empty.conf";
			std::vector<ServerConf> configs = parser.parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 0);

			std::remove("test_empty.conf");
		}

		SUBCASE("Configuration file with only comments") {
			createTempConfigFile("test_only_comments.conf",
				"# This is a comment\n"
				"# Another comment\n"
				"# No server blocks here\n");

			ConfigParser parser;
			std::string configPath = "test_only_comments.conf";
			std::vector<ServerConf> configs = parser.parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 0);

			std::remove("test_only_comments.conf");
		}

		SUBCASE("Non-existent configuration file") {
			std::string configPath = "non_existent_file.conf";

			CHECK_THROWS_AS(ConfigParser::parseConfigFile(configPath.c_str()), std::runtime_error);
		}
	}

	SUBCASE("ConfigParser edge cases and error handling") {

		SUBCASE("Server block without closing brace") {
			createTempConfigFile("test_no_closing_brace.conf",
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n");

			std::string configPath = "test_no_closing_brace.conf";

			CHECK_THROWS_AS(ConfigParser::parseConfigFile(configPath.c_str()), std::runtime_error);

			std::remove("test_no_closing_brace.conf");
		}

		SUBCASE("Server block with nested location blocks") {
			createTempConfigFile("test_nested_locations.conf",
				"server {\n"
				"    listen 8080;\n"
				"    location /api {\n"
				"        proxy_pass http://backend;\n"
				"        location /api/v1 {\n"
				"            auth_basic \"API\";\n"
				"        }\n"
				"    }\n"
				"    root /var/www;\n"
				"}\n");

			std::string configPath = "test_nested_locations.conf";
			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 1);
			CHECK(configs[0].getPort() == 8080);
			CHECK(configs[0].getRoot() == "/var/www");

			std::remove("test_nested_locations.conf");
		}

		SUBCASE("Server block with various directive formats") {
			createTempConfigFile("test_various_formats.conf",
				"server {\n"
				"    listen    8080   ;\n"
				"    root\t/var/www\t;\n"
				"    index index.html index.htm\n"
				"}\n");

			std::string configPath = "test_various_formats.conf";
			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			CHECK(configs.size() == 1);
			CHECK(configs[0].getPort() == 8080);
			CHECK(configs[0].getRoot() == "/var/www");

			std::remove("test_various_formats.conf");
		}
	}

	SUBCASE("ConfigParser parses server block with nested location blocks") {
		const std::string configContent =
			"server {\n"
			"    listen 8080;\n"
			"    location /api {\n"
			"        proxy_pass http://backend;\n"
			"        location /api/v1 {\n"
			"            index \"beep.html\";\n"
			"        }\n"
			"    }\n"
			"    root /var/www;\n"
			"}\n";

		const std::string configPath = "test_nested_location_block.conf";
		createTempConfigFile(configPath, configContent);

		std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

		CHECK(configs.size() == 1);
		CHECK(configs[0].getPort() == 8080);
		CHECK(configs[0].getRoot() == "/var/www");
		// CHECK(configs[0].getRoutes()[0].getRoutes()[0].getDefaultFiles() == "beep.html");

		std::remove(configPath.c_str());
	}

	SUBCASE("Route creation and accessibility from ServerConf") {

		SUBCASE("Single location block creates route correctly") {
			const std::string configContent =
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"    location /api {\n"
				"        root /var/www/api;\n"
				"        index api.html api.json;\n"
				"        autoindex on;\n"
				"        allowed_methods GET POST;\n"
				"    }\n"
				"}\n";

			const std::string configPath = "test_single_route.conf";
			createTempConfigFile(configPath, configContent);

			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			REQUIRE(configs.size() == 1);

			// Check that routes are accessible
			const std::vector<Route>& routes = configs[0].getRoutes();
			CHECK(routes.size() == 2); // 1 location block + 1 default route

			// Find the API route
			const Route* apiRoute = NULL;
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/api") {
					apiRoute = &routes[i];
					break;
				}
			}

			REQUIRE(apiRoute != NULL);
			CHECK(apiRoute->getRootDirectory() == "/var/www/api");
			CHECK(apiRoute->getDefaultFiles().size() == 2);
			CHECK(apiRoute->getDefaultFiles()[0] == "api.html");
			CHECK(apiRoute->getDefaultFiles()[1] == "api.json");
			CHECK(apiRoute->isAutoIndex() == true);

			std::remove(configPath.c_str());
		}

		SUBCASE("Multiple location blocks create multiple routes") {
			const std::string configContent =
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"    location /api {\n"
				"        root /var/www/api;\n"
				"        allowed_methods GET POST PUT DELETE;\n"
				"    }\n"
				"    location /static {\n"
				"        root /var/www/static;\n"
				"        autoindex on;\n"
				"        allowed_methods GET;\n"
				"    }\n"
				"    location /upload {\n"
				"        root /var/www/uploads;\n"
				"        allowed_methods POST DELETE;\n"
				"        autoindex off;\n"
				"    }\n"
				"}\n";

			const std::string configPath = "test_multiple_routes.conf";
			createTempConfigFile(configPath, configContent);

			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			REQUIRE(configs.size() == 1);

			const std::vector<Route>& routes = configs[0].getRoutes();
			CHECK(routes.size() == 4); // 3 location blocks + 1 default route

			// Helper function to find route by path
			const Route* findRouteByPath = NULL;

			// Find API route
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/api") {
					findRouteByPath = &routes[i];
					break;
				}
			}
			REQUIRE(findRouteByPath != NULL);
			CHECK(findRouteByPath->getRootDirectory() == "/var/www/api");

			// Find static route
			findRouteByPath = NULL;
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/static") {
					findRouteByPath = &routes[i];
					break;
				}
			}
			REQUIRE(findRouteByPath != NULL);
			CHECK(findRouteByPath->getRootDirectory() == "/var/www/static");
			CHECK(findRouteByPath->isAutoIndex() == true);

			// Find upload route
			findRouteByPath = NULL;
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/upload") {
					findRouteByPath = &routes[i];
					break;
				}
			}
			REQUIRE(findRouteByPath != NULL);
			CHECK(findRouteByPath->getRootDirectory() == "/var/www/uploads");
			CHECK(findRouteByPath->isAutoIndex() == false);

			std::remove(configPath.c_str());
		}

		SUBCASE("Nested location blocks create nested routes") {
			const std::string configContent =
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"    location /api {\n"
				"        root /var/www/api;\n"
				"        location /api/v1 {\n"
				"            root /var/www/api/v1;\n"
				"            index v1.html;\n"
				"            location /api/v1/auth {\n"
				"                root /var/www/api/v1/auth;\n"
				"                allowed_methods POST;\n"
				"            }\n"
				"        }\n"
				"        location /api/v2 {\n"
				"            root /var/www/api/v2;\n"
				"            index v2.html;\n"
				"        }\n"
				"    }\n"
				"}\n";

			const std::string configPath = "test_nested_routes.conf";
			createTempConfigFile(configPath, configContent);

			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			REQUIRE(configs.size() == 1);

			const std::vector<Route>& routes = configs[0].getRoutes();
			CHECK(routes.size() == 2); // 1 location block + 1 default route

			// Find API route
			const Route* apiRoute = NULL;
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/api") {
					apiRoute = &routes[i];
					break;
				}
			}

			REQUIRE(apiRoute != NULL);
			CHECK(apiRoute->getRootDirectory() == "/var/www/api");

			// Check nested routes
			const std::vector<Route>& nestedRoutes = apiRoute->getRoutes();
			CHECK(nestedRoutes.size() == 2); // /api/v1 and /api/v2

			// Find v1 route
			const Route* v1Route = NULL;
			for (size_t i = 0; i < nestedRoutes.size(); i++) {
				if (nestedRoutes[i].getURLPath() == "/api/v1") {
					v1Route = &nestedRoutes[i];
					break;
				}
			}

			REQUIRE(v1Route != NULL);
			CHECK(v1Route->getRootDirectory() == "/var/www/api/v1");
			CHECK(v1Route->getDefaultFiles().size() == 1);
			CHECK(v1Route->getDefaultFiles()[0] == "v1.html");

			// Check double-nested route (auth)
			const std::vector<Route>& v1NestedRoutes = v1Route->getRoutes();
			CHECK(v1NestedRoutes.size() == 1);
			CHECK(v1NestedRoutes[0].getURLPath() == "/api/v1/auth");
			CHECK(v1NestedRoutes[0].getRootDirectory() == "/var/www/api/v1/auth");

			// Find v2 route
			const Route* v2Route = NULL;
			for (size_t i = 0; i < nestedRoutes.size(); i++) {
				if (nestedRoutes[i].getURLPath() == "/api/v2") {
					v2Route = &nestedRoutes[i];
					break;
				}
			}

			REQUIRE(v2Route != NULL);
			CHECK(v2Route->getRootDirectory() == "/var/www/api/v2");
			CHECK(v2Route->getDefaultFiles().size() == 1);
			CHECK(v2Route->getDefaultFiles()[0] == "v2.html");

			std::remove(configPath.c_str());
		}

		SUBCASE("Default route is always created") {
			const std::string configContent =
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"    index custom.html;\n"
				"    allowed_methods GET POST;\n"
				"}\n";

			const std::string configPath = "test_default_route.conf";
			createTempConfigFile(configPath, configContent);

			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());

			REQUIRE(configs.size() == 1);

			const std::vector<Route>& routes = configs[0].getRoutes();
			CHECK(routes.size() == 1); // Only default route

			// Find default route
			const Route* defaultRoute = NULL;
			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/") {
					defaultRoute = &routes[i];
					break;
				}
			}

			REQUIRE(defaultRoute != NULL);
			CHECK(defaultRoute->getRootDirectory() == "/var/www");
			CHECK(defaultRoute->getDefaultFiles().size() == 1);
			CHECK(defaultRoute->getDefaultFiles()[0] == "custom.html");

			std::remove(configPath.c_str());
		}
	}

	SUBCASE("Route path matching functionality") {

		SUBCASE("Basic path matching works correctly") {
			Route route;
			route.setURLPath("/api");

			CHECK(route.isPathMatch("/api") == true);
			CHECK(route.isPathMatch("/api/") == true);
			CHECK(route.isPathMatch("/api/users") == true);
			CHECK(route.isPathMatch("/api/v1/users") == true);
			CHECK(route.isPathMatch("/different") == false);
			CHECK(route.isPathMatch("/ap") == false);
		}

		SUBCASE("Root path matching works correctly") {
			Route route;
			route.setURLPath("/");

			CHECK(route.isPathMatch("/") == true);
			CHECK(route.isPathMatch("/index.html") == true);
			CHECK(route.isPathMatch("/any/path") == true);
			CHECK(route.isPathMatch("/api/test") == true);
		}

		SUBCASE("Specific path matching works correctly") {
			Route route;
			route.setURLPath("/static/css");

			CHECK(route.isPathMatch("/static/css") == true);
			CHECK(route.isPathMatch("/static/css/") == true);
			CHECK(route.isPathMatch("/static/css/style.css") == true);
			CHECK(route.isPathMatch("/static/js") == false);
			CHECK(route.isPathMatch("/static") == false);
		}
	}

	SUBCASE("Route getMatchingRoute functionality") {

		SUBCASE("Simple route matching") {
			const std::string configContent =
				"server {\n"
				"    listen 8080;\n"
				"    root /var/www;\n"
				"    location /api {\n"
				"        root /var/www/api;\n"
				"    }\n"
				"    location /static {\n"
				"        root /var/www/static;\n"
				"    }\n"
				"}\n";

			const std::string configPath = "test_route_matching.conf";
			createTempConfigFile(configPath, configContent);

			std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());
			REQUIRE(configs.size() == 1);

			const std::vector<Route>& routes = configs[0].getRoutes();

			// Find specific routes for testing
			const Route* apiRoute = NULL;
			const Route* staticRoute = NULL;
			const Route* defaultRoute = NULL;

			for (size_t i = 0; i < routes.size(); i++) {
				if (routes[i].getURLPath() == "/api") {
					apiRoute = &routes[i];
				} else if (routes[i].getURLPath() == "/static") {
					staticRoute = &routes[i];
				} else if (routes[i].getURLPath() == "/") {
					defaultRoute = &routes[i];
				}
			}

			REQUIRE(apiRoute != NULL);
			REQUIRE(staticRoute != NULL);
			REQUIRE(defaultRoute != NULL);

			// Test route matching
			try {
				const Route* matched = apiRoute->getMatchingRoute("/api/users");
				CHECK(matched->getURLPath() == "/api");
			} catch (const std::runtime_error&) {
				FAIL("Should have found matching route");
			}

			try {
				const Route* matched = staticRoute->getMatchingRoute("/static/css/style.css");
				CHECK(matched->getURLPath() == "/static");
			} catch (const std::runtime_error&) {
				FAIL("Should have found matching route");
			}

			try {
				const Route* matched = defaultRoute->getMatchingRoute("/index.html");
				CHECK(matched->getURLPath() == "/");
			} catch (const std::runtime_error&) {
				FAIL("Should have found matching route");
			}

			std::remove(configPath.c_str());
		}

		// TO DO : Check if nested route is worth doing as a bonus
		// SUBCASE("Nested route matching") {
		// 	const std::string configContent =
		// 		"server {\n"
		// 		"    listen 8080;\n"
		// 		"    root /var/www;\n"
		// 		"    location /api {\n"
		// 		"        root /var/www/api;\n"
		// 		"        location /api/v1 {\n"
		// 		"            root /var/www/api/v1;\n"
		// 		"        }\n"
		// 		"    }\n"
		// 		"}\n";

		// 	const std::string configPath = "test_nested_matching.conf";
		// 	createTempConfigFile(configPath, configContent);

		// 	std::vector<ServerConf> configs = ConfigParser::parseConfigFile(configPath.c_str());
		// 	REQUIRE(configs.size() == 1);

		// 	const std::vector<Route>& routes = configs[0].getRoutes();

		// 	// Find API route
		// 	const Route* apiRoute = NULL;
		// 	for (size_t i = 0; i < routes.size(); i++) {
		// 		if (routes[i].getURLPath() == "/api") {
		// 			apiRoute = &routes[i];
		// 			break;
		// 		}
		// 	}

		// 	REQUIRE(apiRoute != NULL);

		// 	// Test that nested route is found
		// 	try {
		// 		const Route* matched = apiRoute->getMatchingRoute("/api/v1/users");
		// 		CHECK(matched->getURLPath() == "/api/v1");
		// 	} catch (const std::runtime_error&) {
		// 		FAIL("Should have found nested matching route");
		// 	}

		// 	// Test that parent route is found when no nested match
		// 	try {
		// 		const Route* matched = apiRoute->getMatchingRoute("/api/v2/test");
		// 		CHECK(matched->getURLPath() == "/api");
		// 	} catch (const std::runtime_error&) {
		// 		FAIL("Should have found parent matching route");
		// 	}

		// 	std::remove(configPath.c_str());
		// }
	}

	SUBCASE("Route parameter setting") {

		SUBCASE("Route parameters are set correctly from config") {
			std::map<std::string, std::string> paramMap;
			paramMap["root"] = "/var/www/test";
			paramMap["index"] = "test.html test.php";
			paramMap["autoindex"] = "on";
			paramMap["allowed_methods"] = "GET POST PUT";

			Route route;
			route.setRouteParam(paramMap);

			CHECK(route.getRootDirectory() == "/var/www/test");
			CHECK(route.getDefaultFiles().size() == 2);
			CHECK(route.getDefaultFiles()[0] == "test.html");
			CHECK(route.getDefaultFiles()[1] == "test.php");
			CHECK(route.isAutoIndex() == true);
		}

		SUBCASE("Route parameters have sensible defaults") {
			std::map<std::string, std::string> paramMap;
			paramMap["root"] = "/var/www/minimal";

			Route route;
			route.setRouteParam(paramMap);

			CHECK(route.getRootDirectory() == "/var/www/minimal");
			CHECK(route.getDefaultFiles().size() == 0);
			CHECK(route.getDefaultFiles().empty());
			CHECK(route.isAutoIndex() == true);
		}

		SUBCASE("Autoindex off setting works") {
			std::map<std::string, std::string> paramMap;
			paramMap["root"] = "/var/www/test";
			paramMap["autoindex"] = "off";

			Route route;
			route.setRouteParam(paramMap);

			CHECK(route.isAutoIndex() == false);
		}
	}
}
