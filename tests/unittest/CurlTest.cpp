#include "doctest.h"
#include "server.hpp"
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <ctime>
#include <unistd.h>
#include <sstream>
#include <cstdlib>

// NOTE: These tests invoke curl and expect a running server at BASE_URL.
// Adjust BASE_URL and endpoint paths to match your server configuration.

static std::string execCommand(const std::string& cmd) {
	std::array<char, 4096> buffer;
	std::string result;
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) return result;
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
		result += buffer.data();
	}
	pclose(pipe);
	return result;
}

static std::pair<int, std::string> curlRequest(const std::string& method, const std::string& url, const std::string& data = "") {
	// Build curl command: capture body and append HTTP status code at the end
	std::ostringstream cmd;
	cmd << "curl -s -X " << method << " ";
	if (!data.empty()) {
		// Simple single-quote safe encoding: replace single quotes with '"'"'
		std::string safe = data;
		size_t pos = 0;
		while ((pos = safe.find('\'', pos)) != std::string::npos) {
			safe.replace(pos, 1, "'\"'\"'");
			pos += 5;
		}
		cmd << "-d '" << safe << "' ";
	}
	cmd << "-H \"Expect:\" -H \"Content-Type: text/plain\" ";
	cmd << "-w \"%{http_code}\" ";
	cmd << url;
	std::cout << cmd.str() << std::endl;
	std::string out = execCommand(cmd.str());
	if (out.size() < 3) {
		return {0, out};
	}
	std::string codeStr = out.substr(out.size() - 3);
	int code = std::atoi(codeStr.c_str());
	std::string body = out.substr(0, out.size() - 3);
	return {code, body};
}

TEST_CASE("CURLing") {
	// Configuration: adjust as needed for your server
	const std::string BASE_URL = "http://localhost:8080"; // change port if required
	const std::string UPLOAD_ENDPOINT = "/upload"; // adjust to match server route that accepts POST for uploads
	const std::string RESOURCE_ENDPOINT = "/files"; // adjust to match GET/DELETE base route if different

	// Build a unique filename to avoid collisions
	pid_t pid = getpid();
	std::time_t t = std::time(0);
	std::ostringstream name;
	name << "testfile_" << pid << "_" << t << ".txt";
	std::string filename = name.str();

	// full URLs
	std::string postUrl = BASE_URL + UPLOAD_ENDPOINT + "/" + filename;
	std::string resourceUrl = BASE_URL + RESOURCE_ENDPOINT + "/" + filename;

	const std::string payload = "hello-from-doctest-" + filename;

	// 1) POST - create/upload resource
	SUBCASE("POST create resource") {
		auto res = curlRequest("POST", postUrl, payload);
		// Accept 200 or 201 for creation
		REQUIRE((res.first == 200 || res.first == 201));
	}

	// 2) GET - retrieve resource
	SUBCASE("GET retrieve resource after POST") {
		// Ensure resource exists: attempt to GET (server must have created resource by POST step)
		auto res = curlRequest("GET", resourceUrl);
		REQUIRE(res.first == 200);
		// Body should contain payload
		REQUIRE(res.second.find(payload) != std::string::npos);
	}

	// 3) DELETE - remove resource
	SUBCASE("DELETE resource") {
		auto res = curlRequest("DELETE", resourceUrl);
		// Accept 200 or 204 for deletion
		REQUIRE((res.first == 200 || res.first == 204));
	}

	// 4) GET after DELETE - expect not found
	SUBCASE("GET after DELETE should return 404") {
		auto res = curlRequest("GET", resourceUrl);
		// Allow 404 or 410 (gone). Some servers return 404 for missing resources.
		REQUIRE((res.first == 404 || res.first == 410));
	}
}
