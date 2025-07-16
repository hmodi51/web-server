
# Web Server

This project is a just a toy server designed to mimick some of the functionalities of HTTP/1.1 specifications. Its still a ongoing project and hasn't even touched the features which were added in HTTP/1.1 specification. 

### Some things which the server currently supports - 

- Client connection to server
- ***GET** and **HEAD** request method for content-type : text/html
- Handling with **200** and **404** response code but still have to work on it to support different headers and make it more robust.


### Things which we have to improve :
- Loop the request until done
- Path traversal defense
- More robust error handling
- Start with writing Test Cases
- and lastly adding concurrency but that is probably will be for the end.