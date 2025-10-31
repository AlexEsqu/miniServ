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
std::string&	trim(std::string& s)
{
	return ltrim(rtrim(s));
}

std::string		strToUpper(std::string& s)
{
	for (size_t i = 0; i < s.length(); i++)
		s[i] = toupper(s[i]);
	return s;
}

std::string		strToLower(std::string& s)
{
	for (size_t i = 0; i < s.length(); i++)
		s[i] = tolower(s[i]);
	return (s);
}

void			verboseLog(const std::string& message)
{
	(void)message;
	#ifdef DEBUG
	std::cout << message << std::endl;
	#endif
}

std::string	parseUrlEncoding(const std::string& encoded)
{
	std::string	decoded;

	for (size_t i = 0; i < encoded.length(); ++i)
	{
		// %XX actually uses hex enconding it's maddness but works ???
		if (encoded[i] == '%' && i + 2 < encoded.length())
		{
			std::string hex = encoded.substr(i + 1, 2);
			char decodedChar = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			decoded += decodedChar;
			i += 2;
		}

		// + is a space
		else if (encoded[i] == '+')
			decoded += ' ';

		// any other character is its own
		else
			decoded += encoded[i];
	}
	return decoded;
}
