#include <iostream>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "gen-cpp/SVR.h"
#include "gen-cpp/SVR_types.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace ::SVR_NS;

class SVRHandler : virtual public SVRIf {
public:
    SVRHandler() {}

    void add(Data& _return, const Data& v1, const Data& v2) override {
        _return.x = v1.x + v2.x;
        _return.y = v1.y + v2.y;
        std::cout << "Adding data: (" << v1.x << ", " << v1.y << ") + (" 
                  << v2.x << ", " << v2.y << ") = (" 
                  << _return.x << ", " << _return.y << ")" << std::endl;
    }

    void subtract(Data& _return, const Data& v1, const Data& v2) override {
        _return.x = v1.x - v2.x;
        _return.y = v1.y - v2.y;
        std::cout << "Subtracting data: (" << v1.x << ", " << v1.y << ") - (" 
                  << v2.x << ", " << v2.y << ") = (" 
                  << _return.x << ", " << _return.y << ")" << std::endl;
    }
};

int main(int argc, char **argv) {
    int port = 9089;
    //step1: 创建processor层
    ::boost::shared_ptr<SVRHandler> handler(new SVRHandler());
    ::boost::shared_ptr<TProcessor> processor(new SVRProcessor(handler));
    //step2: 规定传输层协议
    ::boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    ::boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    //step3: 协议层
    ::boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    std::cout << "Starting SVR server on port " << port << "..." << std::endl;
    server.serve();
    return 0;
}
