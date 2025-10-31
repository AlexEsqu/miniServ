#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <iostream>
#include <cstdlib>

std::vector<std::string>	split(const std::string &s, char delim);
std::string&				rtrim(std::string& s);
std::string&				ltrim(std::string& s);
std::string&				trim(std::string& s);
std::string					strToUpper(std::string& s);
std::string					strToLower(std::string& s);
std::string					parseUrlEncoding(const std::string& encoded);
