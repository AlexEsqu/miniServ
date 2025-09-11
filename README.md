Should we handle ? :
429 Too Many Requests

The HTTP 429 Too Many Requests client error response status code indicates the client has sent too many requests in a given amount of time. This mechanism of asking the client to slow down the rate of requests is commonly called "rate limiting".

//client errors

if request takes too long
	408 REQUEST TIMEOUT
else if request synthax is bad or is unreadable
	400 BAD REQUEST
else if request lacks content length
	411 LENGTH REQUIRED
else if request content is too large (for POST methods for example)
	413 Content Too large
else if request method is not handled by our server
	405 METHOD NOT ALLOWED (or 418 IM A TEAPOT if we're quirky)
else if the uri is too long 
	414 URI Too Long
else if cannot find file
	404 NOT FOUND
else if filename found but we dont allow the request (access to a protected file for example)
	403 FORBIDDEN
else if the content type of the request is not handled by us or we do not want to handle it
	415 UNSUPPORTED MEDIA TYPE
else if GET
	200 OK
else (POST)
	201 CREATED

// SERVER ERRORS
	500 Internal Server Error
	503 Service Unavailable
	505 HTTP Version Not Supported
	507 Insufficient Storage





