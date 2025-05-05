#pragma once

#include <string>
#include <vector>

namespace hydra { namespace compression {

class tvc {
public:
    explicit tvc(int gop_size = 8);

    void encode_video(const std::string& input_path, const std::string& output_path);
    void decode_video(const std::string& input_path, const std::string& output_path);

private:
    int gop_size;
};

}} // namespace hydra::compression
