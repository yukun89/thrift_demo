#include <iostream>
#include <memory>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "thrift_wrapper/RecordingTransportFactory.h"
#include "gen-cpp/SVR.h"
#include "gen-cpp/SVR_types.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace SVR_NS;

void record() {
//传输层
    boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9089));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
/// 创建录制传输工厂
	//这里，我们将read/write都进行了录制，req与resp均进行了录制
	boost::shared_ptr<RecordingTransportFactory> recordingFactory(new RecordingTransportFactory("record_file.bin", true, true));
	boost::shared_ptr<TTransport> recordingTransport = recordingFactory->getTransport(transport);
//protocol层
    	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(recordingTransport));
    SVRClient client(protocol);

    try {
/// 开始录制
	    dynamic_cast<RecordingTransport*>(recordingTransport.get())->startRecording();
	    recordingTransport->open();
	
        // 创建两个示例数据
        Data d1, d2;
        d1.x = 3; d1.y = 2;
        d2.x = 1; d2.y = 4;

        std::cout << "Client Data 1: (" << d1.x << ", " << d1.y << ")" << std::endl;
        std::cout << "Client Data 2: (" << d2.x << ", " << d2.y << ")" << std::endl;
        std::cout << std::endl;

        // 测试加法
        Data result_add;
        client.add(result_add, d1, d2);
        std::cout << "Client Addition result: (" << result_add.x << ", " << result_add.y << ")" << std::endl;
        std::cout << std::endl;

        // 测试减法
        /*
        Data result_sub;
        client.subtract(result_sub, d1, d2);
        std::cout << "Client Subtraction result: (" << result_sub.x << ", " << result_sub.y << ")" << std::endl;
	*/
        /*transport->close();*/
///停止录制
	    dynamic_cast<RecordingTransport*>(recordingTransport.get())->stopRecording();
	    recordingTransport->close();
    } catch (TException& tx) {
        std::cout << "ERROR: " << tx.what() << std::endl;
    }

}

/*
 * 相关record的逻辑是在Transport层实现的，所以必须要在Transport层开启与关闭录制
 * */
void replay() {
//传输层
    boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9089));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
/// 创建录制传输工厂
	//这里，我们将read/write都进行了录制，req与resp均进行了录制
	boost::shared_ptr<RecordingTransportFactory> recordingFactory(new RecordingTransportFactory("record_file.bin", false, false));
	boost::shared_ptr<TTransport> recordingTransport = recordingFactory->getTransport(transport);
//protocol层
    	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(recordingTransport));
    SVRClient client(protocol);

    try {
/// 不打开录制
	    dynamic_cast<RecordingTransport*>(recordingTransport.get())->enablePlayback("record_file.bin");
	    recordingTransport->open();
	
        // 创建两个示例数据
        Data d1, d2;
        d1.x = 0; d1.y = 1;
        d2.x = 1; d2.y = 0;

        std::cout << "Client Data 1: (" << d1.x << ", " << d1.y << ")" << std::endl;
        std::cout << "Client Data 2: (" << d2.x << ", " << d2.y << ")" << std::endl;
        std::cout << std::endl;

        // 测试加法
        Data result_add;
        client.add(result_add, d1, d2);
        std::cout << "Client Addition result: (" << result_add.x << ", " << result_add.y << ")" << std::endl;
        std::cout << std::endl;

        // 测试减法
        /*
        Data result_sub;
        client.subtract(result_sub, d1, d2);
        std::cout << "Client Subtraction result: (" << result_sub.x << ", " << result_sub.y << ")" << std::endl;
	*/
///停止录制
	    recordingTransport->close();
    } catch (TException& tx) {
        std::cout << "ERROR: " << tx.what() << std::endl;
    }

}

int main() {
    //record();
    replay();
    return 0;
}
