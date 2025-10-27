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

// Helper: run shell command and capture stdout
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

// Curl helper: returns (http_code, full_response_without_code)
static std::pair<int, std::string> curlRequest(const std::string& method, const std::string& url, const std::string& data = "", bool includeHeaders = false) {
	std::ostringstream cmd;
	cmd << "curl -s -X " << method << " ";
	if (!data.empty()) {
		// escape single quotes simple technique
		std::string safe = data;
		size_t pos = 0;
		while ((pos = safe.find('\'', pos)) != std::string::npos) {
			safe.replace(pos, 1, "'\"'\"'");
			pos += 5;
		}
		cmd << "-d '" << safe << "' ";
	}
	cmd << "-H \"Expect:\" -H \"Content-Type: text/plain\" ";
	if (includeHeaders)
		cmd << "-i ";
	cmd << "-w \"%{http_code}\" " << url;
	std::cout << "\n\nRequest is [" << cmd.str() << "]\n";
	std::string out = execCommand(cmd.str());
	if (out.size() < 3) return {0, out};
	std::string codeStr = out.substr(out.size() - 3);
	int code = std::atoi(codeStr.c_str());
	std::string body = out.substr(0, out.size() - 3);
	return {code, body};
}

// Wait until server responds (returns true if server up)
static bool waitForServer(const std::string& baseUrl, int timeoutMs = 5000) {
	int waited = 0;
	const int step = 100;
	while (waited < timeoutMs) {
		std::ostringstream cmd;
		cmd << "curl -s -o /dev/null -w \"%{http_code}\" -m 1 " << baseUrl << " >/dev/null 2>&1";
		// simpler: try to get any response (execCommand returns empty on failure)
		std::string out = execCommand("curl -s -o /dev/null -w \"%{http_code}\" -m 1 " + baseUrl);
		if (!out.empty()) return true;
		usleep(step * 1000);
		waited += step;
	}
	return false;
}

TEST_CASE("TOTAL") {
	// configurable defaults — set environment variables to override:
	// TEST_BASE_URL (e.g. http://127.0.0.1:8080)
	// TEST_POST_PATH (e.g. /post)
	// TEST_RESOURCE_PATH (e.g. /pages/test)
	const char* env_base = std::getenv("TEST_BASE_URL");
	const char* env_post = std::getenv("TEST_POST_PATH");
	const char* env_resource = std::getenv("TEST_RESOURCE_PATH");

	std::string BASE_URL = env_base ? env_base : "http://127.0.0.1:8080";
	std::string POST_PATH = env_post ? env_post : "/post";
	std::string RESOURCE_PATH = env_resource ? env_resource : "/upload";

	// Start webserv (child). Adjust executable path / args if needed.
	pid_t pid = fork();
	REQUIRE(pid >= 0);
	if (pid == 0) {
		// child: redirect stdout/stderr to a log file
		int fd = open("webserv_integration.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd >= 0) {
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
		}
		// exec webserv with basic.conf in workspace root
		char* argv[] = { const_cast<char*>("./webserv"), const_cast<char*>("confs/basic.conf"), NULL };
		execv("./webserv", argv);
		// if execv fails, exit child
		_exit(127);
	}

	// parent: wait for server to come up
	bool up = waitForServer(BASE_URL, 8000);
	REQUIRE(up == true);

	// build test resource name
	pid_t mypid = getpid();
	std::time_t t = std::time(0);
	std::ostringstream name;
	name << "testfile_" << mypid << "_" << t << ".txt";
	std::string filename = name.str();

	std::string postUrl = BASE_URL + POST_PATH + "/" + filename;
	std::string resourceUrl = BASE_URL + RESOURCE_PATH + "/" + filename;
	std::string payload = "hello-from-doctest-" + filename;

	// 1) POST
	SUBCASE("POST create resource")
	{
		auto res = curlRequest("POST", postUrl, payload, false);
		CHECK(res.first == CREATED);
	}

	// 2) GET ensure resource returned and includes HTTP headers when requested
	SUBCASE("GET retrieve resource after POST")
	{
		// GET with headers included so we can check HTTP status line presence
		auto res = curlRequest("GET", resourceUrl, "", true);
		CHECK(res.first == OK);
		// headers should contain HTTP/1.1 or HTTP/1.0
		CHECK(res.second.find("HTTP/1.") != std::string::npos);
		// body should contain payload
		CHECK(res.second.find(payload) != std::string::npos);
	}

	// 3) DELETE
	SUBCASE("DELETE resource")
	{
		auto res = curlRequest("DELETE", resourceUrl, "", false);
		CHECK(res.first == NO_CONTENT);
	}

	// 4) GET after DELETE
	SUBCASE("GET after DELETE should return 404")
	{
		auto res = curlRequest("GET", resourceUrl, "", false);
		CHECK(res.first == NOT_FOUND);
	}

	// Shut down the server
	kill(pid, SIGTERM);
	int status = 0;
	waitpid(pid, &status, 0);
}
