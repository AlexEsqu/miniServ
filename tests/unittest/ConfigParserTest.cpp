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

TEST_CASE("ConfigParser utility functions") {

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

TEST_CASE("ConfigParser parseServerBlock function") {

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

TEST_CASE("ConfigParser parseConfigFile function") {

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

TEST_CASE("ConfigParser edge cases and error handling") {

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

TEST_CASE("ConfigParser parses server block with nested location blocks") {
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
