#include <torch/torch.h>
#include <iostream>
#include "gpt.h"

int main() {
    torch::Device device(torch::kCUDA);
    if (!torch::cuda::is_available()) {
        std::cout << "CUDA not available, using CPU." << std::endl;
        device = torch::kCPU;
    }

    GPT model;
    model.to(device);
    torch::Tensor x = torch::randn({1, 10}).to(device);
    auto result = model.forward(x);
    std::cout << "Output: \n" << result << std::endl;
    return 0;
}
