#include "handlers.hpp"

/*If the extension requires cgi, the function will return the extension.
Otherwise, it will return NULL*/
std::string doesRequestNeedCGI(Request &req)
{
	std::vector<std::string> acceptedCGIs;

	acceptedCGIs.push_back(".py");
	acceptedCGIs.push_back(".php");
	std::vector<std::string>::iterator it;

	for (it = acceptedCGIs.begin(); it != acceptedCGIs.end(); it++)
	{
		if (req.getRequestedURL().find(*it) != it->npos)
		{
			std::cout << *it << std::endl;
			return (*it);
		}
	}
	return (NULL);
}