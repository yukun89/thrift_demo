g++ -std=c++11 server.cpp gen-cpp/SVR_types.cpp gen-cpp/SVR.cpp -o server -lthrift
g++ -std=c++11 client.cpp gen-cpp/SVR_types.cpp gen-cpp/SVR.cpp -o client -lthrift
g++ -std=c++11 clientRecording.cpp gen-cpp/SVR_types.cpp gen-cpp/SVR.cpp RecordingTransport.cpp -o clientRecording -lthrift
g++ -std=c++11 clientReplay.cpp gen-cpp/SVR_types.cpp gen-cpp/SVR.cpp RecordingTransport.cpp -o clientReplay -lthrift
