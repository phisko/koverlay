#pragma once

#include "mediator/Module.hpp"
#include "Packets.hpp"

class ProtocolModule final : public putils::Module<ProtocolModule,
        kia::packets::IncomingMessage, kia::packets::HttpResponse> {
public:
    void handle(const kia::packets::HttpResponse & packet) const noexcept;
    void handle(const kia::packets::IncomingMessage & packet) const noexcept;

private:
    void postParams(kia::packets::HttpRequest & request, std::string_view line) const noexcept;
    void getParams(kia::packets::HttpRequest & request) const noexcept;
    void getRequestLine(kia::packets::HttpRequest & request, std::string_view line) const noexcept;
    void addHeader(kia::packets::HttpRequest & request, std::string_view line) const noexcept;
};
