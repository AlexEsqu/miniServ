#include "Response.hpp"

e_dataProgress	Response::parseCGIHeaderLine(std::string &chunk)
{
	// store chunk and return to epoll if the chunk does not contain
	// the end of the header line
	size_t lineEnd = chunk.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		_unparsedBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string headerLine = _unparsedBuffer + chunk.substr(0, lineEnd);
	_unparsedBuffer.clear();

	// either add the header line as variable to the header map of variable
	if (!headerLine.empty())
		addHttpHeader(headerLine);
	// or if it's empty, I reached the end of the headers and can set if
	// parsing the body is necessary, which will update the parsing state
	else
		assembleCGIBody();

	// erase data used from the chunk, store the rest
	chunk.erase(0, lineEnd + 2);

	return RECEIVED_ALL;
}

e_dataProgress	Response::assembleCGIBody(std::string &chunk)
{
	// copy all leftover from the parsing into the buffer
	if (!_unparsedBuffer.empty())
	{
		_requestBodyBuffer.writeToBuffer(_unparsedBuffer.substr(0, getRequest().getContentLength()));
		_unparsedBuffer.clear();
	}

	size_t	remainderToRead = getRequest().getContentLength() - _requestBodyBuffer.getBufferSize();

	_requestBodyBuffer.writeToBuffer(chunk.substr(0, remainderToRead));
	chunk.clear();

	// check received content is correct length or wait for more
	// std::cout << "buffer size is [" << _requestBodyBuffer.getBufferSize() << "]\n";
	if (_requestBodyBuffer.getBufferSize() < _contentLength && _requestBodyBuffer.getBufferSize() < _conf.getMaxSizeClientRequestBody())
		return WAITING_FOR_MORE;
	else
	{
		_requestState = PARSING_DONE;
		return RECEIVED_ALL;
	}
}

// For every chunk of data added to the request, parsing continues from last state
// and returns if the current parsed item (header, body...) is not finished
void	Response::addCGIChunk(std::string chunk)
{
	while (_cgiParsingState != PARSING_CGI_DONE)
	{
		switch (_cgiParsingState)
		{
			case PARSING_CGI_HEADERS:
			{
				if (parseCGIHeaderLine(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_CGI_BODY:
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




