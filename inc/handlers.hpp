#pragma once
#include "server.hpp"

int doesRequestNeedCGI(Request &req);

enum e_CGI
{
	NO_CGI,
	PY,
	PHP
};