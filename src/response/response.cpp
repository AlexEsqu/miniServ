#include "server.hpp"

std::string createResponse(std::string filePath)
{
	std::string finalResponse;
	std::ifstream input(filePath.c_str()); // opening the file as the content for the response
	std::stringstream content;
	content << input.rdbuf(); // putting the content of the input file into the content variable

	Response response(200,"text/html", content.str()); // Setting all the necessary infos for the response
	finalResponse = response.createResponse(); // creating the final response with all the values precedently added
	return (finalResponse);
}
