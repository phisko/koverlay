# Http

An extensible C++ Http server inspired by C#'s Nancy.

## HttpServer

The only class users have to interact with in order to get a working server is `HttpServer`.

### Members

##### Constructor
```cpp
HttpServer(short normalPort = 4242, short securePort = 4243);
```
Starts a server on the given ports. `securePort` will be used for an HTTPS connection using OpenSSL.

Upon construction, one thread is started for each port.

##### get
```cpp
void get(const std::string &uri, const Route &route)
```
Adds a new GET endpoint at the given `uri`, which will be served by `route`.

##### post
```cpp
void post(const std::string &uri, const Route &route)
```
Adds a new POST endpoint at the given `uri`, which will be served by `route`.

##### Helper types

```cpp
using HttpParams = std::unordered_map<std::string, std::string>;
using Route = std::function<std::string(const HttpParams &)>;
```
A `Route` is a function that will be called upon request for a given URI. It should return the `std::string` that will be sent to the client, and takes as parameter the GET or POST parameters for the query.

### Example

```cpp
int main()
{
    putils::HttpServer server(4242, -1);

    // Home page
    server.get("/", [](const auto &)
    {
        return "test";
    });

    // Get with parameters
    server.get("/getParams", [](const auto &params)
    {
        return params.at("first") + " " + params.at("second");
    });

    // Run an executable
    server.get("/run/{first}", [](const auto &params)
    {
        putils::Process::Options options;
        options.stdout.redirected = true;

        putils::Process p(params.at("first"), options);

        return putils::to_string(p.getStdout());
    });

    // Post
    server.post("/{first}", [](const auto &params)
    {
        return params.at("first") + " " + params.at("str");
    });

    std::string buff;
    while (std::getline(std::cin, buff))
        if (buff == "exit")
            return;
}
```
