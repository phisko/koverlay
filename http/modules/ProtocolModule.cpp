#include <sstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include "ProtocolModule.hpp"

#include "chop.hpp"
#include "url.hpp"

static bool readLine(std::istream & s, std::string & ret) {
    if (!s || s.peek() == -1)
        return false;

    ret.clear();

    char c;
    while (s && s.peek() != -1) {
        c = (char) s.get();
        if (c == '\r' && s.peek() == '\n') {
            s.get();
            return true;
        }
        ret.append(1, c);
    }

    return true;
}

void ProtocolModule::handle(const kia::packets::IncomingMessage & packet) const noexcept {
    /*
     Request       = Request-Line              ; Section 5.1
                        *(( general-header        ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3
     */

    kia::packets::HttpRequest request;

    request.clientFd = packet.clientFd;

    std::stringstream stream(packet.msg);
    std::string line;

    // First line indicates method, request-URI and HTTP version
    if (!readLine(stream, line)) {
        send(std::move(request));
        return;
    }

    getRequestLine(request, line);

    // Get header fields into the HttpRequest
    while (readLine(stream, line) && putils::chop(line).length())
        addHeader(request, line);

    // Rest of message is the body
    while (readLine(stream, line)) {
        postParams(request, line);
        request.body += line + "\n";
    }

    request.body = putils::chop(request.body);

    send(std::move(request));
}

void ProtocolModule::postParams(kia::packets::HttpRequest & request, std::string_view line) const noexcept {
    static const std::regex reg("^(.*)=(.*)$");

    std::cmatch m;
    if (std::regex_match(line.data(), m, reg))
        request.params[m[1]] = m[2];
}

static void readGetParams(kia::packets::HttpRequest & request, std::string_view line) {
    std::stringstream s(line.data());

    while (s && s.peek() > 0) {
        std::string key;
        while (s && s.peek() > 0 && s.peek() != '=')
            key.append(1, (char) s.get());
        if (!s)
            throw std::runtime_error("Bad key-value pair in GET parameters");

        s.get(); // Skip '='

        std::string value;
        while (s && s.peek() > 0 && s.peek() != '&')
            value.append(1, (char) s.get());

        if (s.peek() == '&')
            s.get();

        request.params[putils::url::decode(key)] = putils::url::decode(value);
    }
}

void ProtocolModule::getParams(kia::packets::HttpRequest & request) const noexcept {
    const auto pos = request.uri.find('?');
    if (pos != std::string::npos) {
        try {
            readGetParams(request, request.uri.substr(pos + 1));
            request.uri = request.uri.substr(0, pos);
        }
        catch (const std::runtime_error & e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void ProtocolModule::getRequestLine(kia::packets::HttpRequest & request, std::string_view line) const noexcept {
    // From RFC 2616: Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    static const std::regex reg("^([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)$");

    std::cmatch m;
    if (std::regex_match(line.data(), m, reg)) {
        request.method = m[1];
        request.uri = m[2];
        getParams(request);
        request.httpVersion = m[3];
    }
}

void ProtocolModule::addHeader(kia::packets::HttpRequest & request, std::string_view line) const noexcept {
    // message-header = field-name ":" [ field-value ]
    static const std::regex reg("^([^\\:]*)\\: (.*)$");

    std::cmatch m;
    if (std::regex_match(line.data(), m, reg)) {
        auto key = m[1];
        auto value = m[2];

        request.headers.emplace(key, value);
    }
}

void ProtocolModule::handle(const kia::packets::HttpResponse & response) const noexcept {
    /*
      Response      = Status-Line               ; Section 6.1
                       *(( general-header        ; Section 4.5
                        | response-header        ; Section 6.2
                        | entity-header ) CRLF)  ; Section 7.1
                       CRLF
                       [ message-body ]          ; Section 7.2
     */

    std::stringstream output;

    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    output << response.httpVersion << " "
           << response.statusCode << " "
           << response.reasonPhrase << "\r\n";

    // message-header = field-name ":" [ field-value ]
    for (auto & pair : response.headers)
        output << pair.first << ":" << pair.second << "\r\n";
    output << "\r\n";

    output << response.body;
    output << "\r\n";

    send(kia::packets::OutgoingMessage{ response.clientFd, output.str() });
}
