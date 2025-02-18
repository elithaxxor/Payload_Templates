const http = require('http');
const httpProxy = require('http-proxy');

// TODO: Userinput to  Define the target server
// TODO: Userinput to  Define the port
// TODO:

console.log("// Create an HTTP server that listens for requests and forwards them to the target server\n")

const proxy = httpProxy.createProxyServer({"target": "http://example.com"});

const server = http.createServer((req, res) => {
    const target = 'http://example.com'; // Replace with the target server URL

    // Forward the request to the target server
    proxy.web(req, res, { target: target }, (err) => {
        if (err) {
            console.error('Proxy error:', err);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Something went wrong.');
        }
    });
});

// Start the server on port 3000
server.listen(3000, () => {
    console.log('Proxy server is running on http://localhost:3000');
});
// To test the proxy server, you can use a tool like curl or Postman to send requests to http://localhost:3000
// and see if they are forwarded to the target server.

// TODO: Add error handling and logging, and implement any additional features as needed.