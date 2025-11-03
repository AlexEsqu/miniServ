#include "Response.hpp"

e_dataProgress	Response::parseCGIHeaderLine(std::string &chunk)
{
	// store chunk and return to epoll if the chunk does not contain
	// the end of the header line
	size_t lineEnd = chunk.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		_unparsedCgiBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string headerLine = _unparsedCgiBuffer + chunk.substr(0, lineEnd);
	_unparsedCgiBuffer.clear();

	// either add the header line as variable to the header map of variable
	if (!headerLine.empty())
		addHttpHeader(headerLine);
	// or if it's empty, I reached the end of the headers and can go to body
	else
		_cgiParsingState = CGI_PARSING_BODY;

	// erase data used from the chunk, store the rest
	chunk.erase(0, lineEnd + 2);

	return RECEIVED_ALL;
}

e_dataProgress	Response::assembleCGIBody(std::string &chunk)
{
	// copy all leftover from the parsing into the buffer
	if (!_unparsedCgiBuffer.empty())
	{
		_responsePage.writeToBuffer(_unparsedCgiBuffer.substr(0, getRequest().getContentLength()));
		_unparsedCgiBuffer.clear();
	}

	_responsePage.writeToBuffer(chunk);
	chunk.clear();

	// check received content is correct length or wait for more
	if (_responsePage.getBufferSize() > getRequest().getConf().getMaxSizeClientRequestBody())
	{
		setError(PAYLOAD_TOO_LARGE);
		return RECEIVED_ALL;
	}

	return WAITING_FOR_MORE;
}

// For every chunk of data added to the request, parsing continues from last state
// and returns if the current parsed item (header, body...) is not finished
void	Response::addCGIChunk(std::string chunk)
{
	while (_cgiParsingState != CGI_PARSING_DONE && !hasError())
	{
		if (chunk.empty())
			_cgiParsingState = CGI_PARSING_DONE;
		switch (_cgiParsingState)
		{
			case CGI_PARSING_HEADERS:
			{
				if (parseCGIHeaderLine(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			case CGI_PARSING_BODY:
			{
				if (assembleCGIBody(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			default:
				return;
		}
	}
}




