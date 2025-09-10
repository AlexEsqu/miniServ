#include "handlers.hpp"


/* Running on an issue, when the client asks for "/" i need to serve whatever file that
is asked in the config, for example : index.php index.html 
*/

/*If the extension requires cgi, the function will return the extension.
Otherwise, it will return NULL*/

int doesRequestNeedCGI(Request &req)
{
	std::vector<std::string> acceptedCGIs;

	acceptedCGIs.push_back(".py");
	acceptedCGIs.push_back(".php");
	std::vector<std::string>::iterator it;

	for (it = acceptedCGIs.begin(); it != acceptedCGIs.end(); it++)
	{
		std::size_t pos = req.getRequestedURL().find(*it);
		if (pos != std::string::npos)
		{
			if (req.getRequestedURL().substr(pos) == ".py");
				return(PY);
			if (req.getRequestedURL().substr(pos) == ".php");
				return(PHP);
		}
	}
	return (NO_CGI);
}

