#ifndef RECORDING_TRANSPORT_FACTORY_H
#define RECORDING_TRANSPORT_FACTORY_H

#include <thrift/transport/TTransport.h>
#include "RecordingTransport.h"

namespace apache { namespace thrift { namespace transport {

/*
 * 这里是一个工厂，负责RecordingTransport 的配置与创建, 产生相关实例
 */
class RecordingTransportFactory : public TTransportFactory {
public:
  RecordingTransportFactory(const std::string& record_file,
                           bool record_reads = true,
                           bool record_writes = true)
    : record_file_(record_file),
      record_reads_(record_reads),
      record_writes_(record_writes) {}
  
  virtual ~RecordingTransportFactory() {}
  
  boost::shared_ptr<TTransport> getTransport(boost::shared_ptr<TTransport> trans) {
    return boost::shared_ptr<TTransport>(new RecordingTransport(trans, record_file_, record_reads_, record_writes_));
  }
  
private:
  std::string record_file_;
  bool record_reads_;
  bool record_writes_;
};

}}} // apache::thrift::transport

#endif // RECORDING_TRANSPORT_FACTORY_H
