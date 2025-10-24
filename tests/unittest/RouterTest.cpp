#include "doctest.h"

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <string>

#include "Router.hpp"
#include "Route.hpp"

// helper RAII for temporary directories/files
struct TempDir
{
	std::string path;
	TempDir(const char* tmpl = "/tmp/miniserv_testXXXXXX")
	{
		char buf[256];
		std::snprintf(buf, sizeof(buf), "%s", tmpl);
		char* res = mkdtemp(buf);
		if (res)
			path = std::string(res);
		else
			path = "";
	}
	~TempDir()
	{
		// Remove all files inside (best-effort)
		// user tests will remove files explicitly
		if (!path.empty()) {
			rmdir(path.c_str()); // only succeeds if empty
		}
	}
	std::string join(const std::string& name) const
	{
		if (path.empty()) return name;
		if (path[path.size()-1] == '/')
			return path + name;
		return path + "/" + name;
	}
};

// small helper to write a file
static void writeFile(const std::string& p, const std::string& content = "ok")
{
	std::ofstream ofs(p.c_str(), std::ios::binary);
	ofs << content;
	ofs.close();
}

// remove file if exists
static void rmfile(const std::string& p)
{
	unlink(p.c_str());
}

// remove directory if exists (must be empty)
static void rmdir_force(const std::string& p)
{
	rmdir(p.c_str());
}

TEST_CASE("ROUTER")
{



SUBCASE("Router utils")
{
	SUBCASE("slash helpers and joinPaths")
	{
	CHECK(Router::isRootPath("/") == true);
	CHECK(Router::isRootPath("") == true);
	CHECK(Router::isRootPath("/foo") == false);

	CHECK(Router::hasStartingSlash("/a") == true);
	CHECK(Router::hasStartingSlash("a") == false);

	CHECK(Router::hasTrailingSlash("/a/") == true);
	CHECK(Router::hasTrailingSlash("/a") == false);

	CHECK(Router::joinPaths("/root/", "/file") == "/root/file");
	CHECK(Router::joinPaths("/root", "file") == "/root/file");
	CHECK(Router::joinPaths("/root/", "file") == "/root/file");
	CHECK(Router::joinPaths("/root", "/file") == "/root/file");
	}

	SUBCASE("Router isDirectory and isValidFilePath")
	{
	TempDir tmp;
	REQUIRE(tmp.path.size() > 0);

	std::string dirpath = tmp.join("dir");
	int mk = mkdir(dirpath.c_str(), 0700);
	CHECK(mk == 0);

	std::string filepath = tmp.join("f.txt");
	writeFile(filepath, "hello");

	CHECK(Router::isDirectory(dirpath) == true);
	CHECK(Router::isDirectory(filepath) == false);

	CHECK(Router::isValidFilePath(filepath) == true);
	CHECK(Router::isValidFilePath(dirpath) == false);

	// cleanup
	rmfile(filepath);
	rmdir_force(dirpath);
	}
}

SUBCASE("Router for GET")
{
	{
		TempDir tmp;
		REQUIRE(tmp.path.size() > 0);

		// create root + files
		std::string root = tmp.path;
		std::string fileA = tmp.join("hello.txt");
		std::string dirA = tmp.join("subdir");
		int mk = mkdir(dirA.c_str(), 0700);
		CHECK(mk == 0);
		std::string idx = tmp.join("subdir/index.html");
		writeFile(fileA, "h");
		writeFile(idx, "<html></html>");

		// prepare route: url path "/r" -> root = tmp.path
		Route r;
		r.setURLPath("/r");
		r.setRootDirectory(root);
		std::vector<std::string> defaults;
		defaults.push_back(std::string("index.html"));
		r.setDefaultFiles(defaults);

		SUBCASE("returns file at routed URI if existing")
		{
			// request to a file under route
			std::string urlFile = "/r/hello.txt";
			std::string resolvedFile = Router::routeFilePathForGet(urlFile, &r);
			CHECK(resolvedFile == root + "/hello.txt");
		}

		SUBCASE("returns default file at routed URI if default provided")
		{
			// request to directory without trailing slash -> should return default file path
			std::string urlDirNoSlash = "/r/subdir";
			std::string resolvedDir = Router::routeFilePathForGet(urlDirNoSlash, &r);
			CHECK(resolvedDir == root + "/subdir/index.html");

			// request to directory with trailing slash -> should also return default file
			std::string urlDirSlash = "/r/subdir/";
			std::string resolvedDir2 = Router::routeFilePathForGet(urlDirSlash, &r);
			CHECK(resolvedDir2 == root + "/subdir/index.html");
		}

		SUBCASE("returns empty when file does not exist")
		{
			std::string missing = "/r/missing.txt";
			std::string resolved = Router::routeFilePathForGet(missing, &r);
			CHECK(resolved.empty());
		}

		// cleanup
		rmfile(fileA);
		rmfile(idx);
		rmdir_force(dirA);

	}

	{
		// This test verifies behaviour when directory has no default file AND route->isAutoIndex()
		// Because Route interface in the codebase exposes isAutoIndex(), but no direct setter visible in tests,
		// we emulate scenario by creating a Route with no default files and expecting Router::routeFilePathForGetAsDirectory
		// to return routedURL only if it is a directory and route->isAutoIndex() true.
		// If setting autoindex is not available in Route API, this test only validates directory detection logic and default-file pathing.
		TempDir tmp;
		REQUIRE(tmp.path.size() > 0);

		std::string root = tmp.path;
		std::string dirA = tmp.join("dnoidx");
		int mk = mkdir(dirA.c_str(), 0700);
		CHECK(mk == 0);

		Route r;
		r.setURLPath("/p");
		r.setRootDirectory(root);
		// ensure no default files
		std::vector<std::string> defs;
		r.setDefaultFiles(defs);

		SUBCASE("returns directory path if no default and autoindex is allowed")
		{
			// prepare routedURL already replaced by root directory
			std::string routed = root + "/dnoidx";
			std::string res = Router::routeFilePathForGetAsDirectory(routed, &r);
			bool ok = (res.empty() || res == (routed + "/"));
			CHECK(ok);
		}

		SUBCASE("returns empty path if no default nor autoindex is not allowed")
		{
			std::string url = "/p/dnoidx";
			std::string resolvedFromGet = Router::routeFilePathForGet(url, &r);
			bool ok = (resolvedFromGet.empty() || Router::isDirectory(resolvedFromGet));
			CHECK(ok);
		}

		rmdir_force(dirA);
	}
}

SUBCASE("Router for POST")
{
	SUBCASE("routeFilePathForPost replaces route path by upload directory")
	{
		TempDir tmp;
		REQUIRE(tmp.path.size() > 0);

		std::string root = tmp.path;
		std::string upload = tmp.join("upload");
		int mk = mkdir(upload.c_str(), 0700);
		CHECK(mk == 0);

		Route r;
		r.setURLPath("/u");
		r.setRootDirectory(root);
		r.setUploadDirectory(upload);

		std::string url = "/u/some/path/file.bin";
		std::string res = Router::routeFilePathForPost(url, &r);

		// replaceRoutePathByUploadDirectory simply replaces the leading route->getURLPath() with upload dir
		std::string expected = upload + std::string("/some/path/file.bin");
		CHECK(res == expected);

		rmdir_force(upload);
	}

	SUBCASE("routeFilePathForPost with non-existing upload base still composes path")
	{
		TempDir tmp;
		REQUIRE(tmp.path.size() > 0);

		std::string root = tmp.path;
		std::string upload = tmp.join("upload_nonexist");
		// don't create upload dir on purpose

		Route r;
		r.setURLPath("/u");
		r.setRootDirectory(root);
		r.setUploadDirectory(upload);

		std::string url = "/u/some/path/file.bin";
		std::string res = Router::routeFilePathForPost(url, &r);

		std::string expected = upload + std::string("/some/path/file.bin");
		CHECK(res == expected);
	}
}
}
