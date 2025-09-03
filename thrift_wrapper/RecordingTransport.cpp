#include "RecordingTransport.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

using namespace apache::thrift::transport;

// 数据包头部结构
struct PacketHeader {
    char direction;      // 'W' for write, 'R' for read
    int64_t timestamp;    // 时间戳（毫秒）
    uint32_t length;     // 数据长度
};

RecordingTransport::RecordingTransport(boost::shared_ptr<TTransport> transport,
                                       const std::string& record_file,
                                       bool record_reads,
                                       bool record_writes)
  : transport_(transport),
    record_file_(record_file),
    record_reads_(record_reads),
    record_writes_(record_writes),
    is_recording_(false),
    is_playback_(false) {
}

RecordingTransport::~RecordingTransport() {
  stopRecording();
  std::lock_guard<std::mutex> lock(record_mutex_);
  if (record_stream_.is_open()) {
    record_stream_.close();
  }
  if (playback_stream_.is_open()) {
    playback_stream_.close();
  }
}

//流量的录制在read/write函数
uint32_t RecordingTransport::read(uint8_t* buf, uint32_t len) {
  if (is_playback_ && playback_stream_.is_open()) {
//TODO 这里的递归会产生死锁问题
   // std::lock_guard<std::mutex> lock(playback_mutex_);
    
    // 检查是否还有数据可读
    if (playback_stream_.eof()) {
      throw TTransportException(TTransportException::END_OF_FILE, 
                               "No more data to read - playback file ended");
    }
    
    // 读取包头
    PacketHeader header;
    playback_stream_.read(reinterpret_cast<char*>(&header), sizeof(PacketHeader));
    
    if (playback_stream_.gcount() != sizeof(PacketHeader)) {
      throw TTransportException(TTransportException::UNKNOWN, 
                               "Incomplete packet header in playback file");
    }
    
    // 只处理读取方向的数据包
    if (header.direction != 'R') {
      // 跳过非读取数据包
      std::cout << "DEBUG skip not 'R' marked data. direction=" << header.direction << "||len=" << len << std::endl;
      playback_stream_.seekg(header.length, std::ios::cur);
      return read(buf, len); // 递归调用读取下一个包
    }
    
    if (header.length > len) {
      throw TTransportException(TTransportException::UNKNOWN, 
                               "Playback data larger than buffer");
    }
    
    // 读取数据
    std::cout << "read data from file. size=" << header.length << std::endl;
    playback_stream_.read(reinterpret_cast<char*>(buf), header.length);
    
    if (playback_stream_.gcount() != header.length) {
      throw TTransportException(TTransportException::UNKNOWN, 
                               "Incomplete data in playback file");
    }
    
    // 如果同时也在录制，记录这个读取操作
    if (record_reads_ && is_recording_) {
      std::cout << "ERR: should not record in playback mode" << std::endl;
      recordData(buf, header.length, false);
    }
    
    return header.length;
  } else {
    // 正常模式 - 从实际传输层读取, 并且对数据进行记录
    std::cout << "read data from network. size=" << len << std::endl;
    uint32_t bytes_read = transport_->read(buf, len);
    
    if (record_reads_ && is_recording_) {
      recordData(buf, bytes_read, false);
    }
    
    return bytes_read;
  }
}

void RecordingTransport::write(const uint8_t* buf, uint32_t len) {
  if (is_playback_) {
    std::cout << "no need to write data to sever in replay mode. len=" << len << std::endl;
    return;
  }
  transport_->write(buf, len);
  
  if (record_writes_ && is_recording_) {
    recordData(buf, len, true);
  }
}

void RecordingTransport::flush() {
  transport_->flush();
}

const uint8_t* RecordingTransport::borrow(uint8_t* buf, uint32_t* len) {
  return transport_->borrow(buf, len);
}

void RecordingTransport::consume(uint32_t len) {
  transport_->consume(len);
}

void RecordingTransport::recordData(const uint8_t* buf, uint32_t len, bool is_write) {
  std::lock_guard<std::mutex> lock(record_mutex_);
  
  if (!record_stream_.is_open()) {
    record_stream_.open(record_file_, std::ios::binary | std::ios::app);
    if (!record_stream_.is_open()) {
      throw TTransportException(TTransportException::UNKNOWN, 
                               "Failed to open record file");
    }
  }
  
  // 创建数据包头
  PacketHeader header;
  header.direction = is_write ? 'W' : 'R';
  header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  header.length = len;
  
  // 写入包头和数据
  record_stream_.write(reinterpret_cast<const char*>(&header), sizeof(header));
  record_stream_.write(reinterpret_cast<const char*>(buf), len);
  record_stream_.flush();
  std::cout << "DEBUG: recording||direction=" << header.direction << "||ts=" << header.timestamp << "||len=" << header.length << "||data=XXX..." << std::endl;
}

void RecordingTransport::startRecording() {
  if (!is_recording_) {
    std::lock_guard<std::mutex> lock(record_mutex_);
    record_stream_.open(record_file_, std::ios::binary | std::ios::trunc);
    if (!record_stream_.is_open()) {
      throw TTransportException(TTransportException::UNKNOWN, 
                               "Failed to open record file");
    }
    is_recording_ = true;
  }
}

void RecordingTransport::stopRecording() {
  if (is_recording_) {
    std::lock_guard<std::mutex> lock(record_mutex_);
    if (record_stream_.is_open()) {
        record_stream_.close();
    }
    is_recording_ = false;
  }
}

bool RecordingTransport::enablePlayback(const std::string& playback_file) {
  if (is_playback_) {
    std::lock_guard<std::mutex> lock(playback_mutex_);
    if (playback_stream_.is_open()) {
      playback_stream_.close();
    }
  }
  
  std::lock_guard<std::mutex> lock(playback_mutex_);
  playback_stream_.open(playback_file, std::ios::binary);
  if (!playback_stream_.is_open()) {
    return false;
  }
  
  is_playback_ = true;
  return true;
}

// 检查是否还有数据可读
bool RecordingTransport::hasMoreData() {
  if (!is_playback_ || !playback_stream_.is_open()) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(playback_mutex_);
  return !playback_stream_.eof();
}
