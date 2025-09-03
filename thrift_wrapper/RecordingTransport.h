#ifndef RECORDING_TRANSPORT_H
#define RECORDING_TRANSPORT_H

#include <thrift/transport/TTransport.h>
#include <thrift/transport/TVirtualTransport.h>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>

namespace apache { namespace thrift { namespace transport {

class RecordingTransport : public TVirtualTransport<RecordingTransport> {
public:
  RecordingTransport(boost::shared_ptr<TTransport> transport, 
                    const std::string& record_file,
                    bool record_reads = true,
                    bool record_writes = true);
  
  ~RecordingTransport() override;
  
  bool isOpen() override {
    return transport_->isOpen();
  }
  
  bool peek() override {
    return transport_->peek();
  }
  
  void open() override {
    transport_->open();
  }
  
  void close() override {
    transport_->close();
  }
  
  uint32_t read(uint8_t* buf, uint32_t len);
  
  void write(const uint8_t* buf, uint32_t len);
  
  void flush();
  
  const uint8_t* borrow(uint8_t* buf, uint32_t* len);
  
  void consume(uint32_t len);
  
 ///recording part 
  // 开始录制
  void startRecording();
  // 停止录制
  void stopRecording();

///replay part
  // 回放功能 - 从录制文件中读取数据模拟服务器响应
  bool enablePlayback(const std::string& playback_file);
  bool hasMoreData();
/*
    // 新增方法：检查是否有特定方法的回放数据
    bool hasMethodData(const std::string& method_name) {
        // 实现逻辑：检查录制文件中是否有指定方法的数据
        // 可以根据方法名、请求类型等进行匹配
    }
    
    // 新增方法：获取回放统计信息
    //PlaybackStats getPlaybackStats() const {
        // 返回回放统计信息，如成功次数、失败次数等
    //}
    
    // 新增方法：设置回放过滤器
    void setPlaybackFilter(const PlaybackFilter& filter) {
        // 设置回放过滤条件，如只回放特定方法或特定参数
    }
*/
    
  
private:
  boost::shared_ptr<TTransport> transport_;
  std::ofstream record_stream_;
  std::ifstream playback_stream_;
  std::string record_file_;
  bool record_reads_;
  bool record_writes_;
  bool is_recording_;

  bool is_playback_;
  std::mutex record_mutex_;
  std::mutex playback_mutex_;
  
  void recordData(const uint8_t* buf, uint32_t len, bool is_write);
};

}}} // apache::thrift::transport

#endif // RECORDING_TRANSPORT_H
