#include "parsing.hpp"

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string item;

	while (getline (ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}

// trim from end of string (right)
std::string& rtrim(std::string& s)
{
	const char* ws = " \t\n\r\f\v";
	s.erase(s.find_last_not_of(ws) + 1);
	return s;
}

// trim from beginning of string (left)
std::string& ltrim(std::string& s)
{
	const char* ws = " \t\n\r\f\v";
	s.erase(0, s.find_first_not_of(ws));
	return s;
}

// trim from both ends of string (right then left)
std::string& trim(std::string& s)
{
	return ltrim(rtrim(s));
}

std::string& strToUpper(std::string& s)
{
	for (std::string::iterator i = s.begin(); i != s.end(); i++) {
		*i = toupper(*i);
	}
	return s;
}
