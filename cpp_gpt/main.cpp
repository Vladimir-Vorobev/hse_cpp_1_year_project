#include <torch/torch.h>
#include <iostream>
#include "src/gpt.h"

int main() {
    torch::Device device(torch::kCUDA);
    if (!torch::cuda::is_available()) {
        std::cout << "CUDA not available, using CPU." << std::endl;
        device = torch::kCPU;
    }

    int vocab_size = 30000;
    int max_input_len = 1024;
    int model_dim = 512;
    int num_decoder_layers = 8;
    int num_attn_heads = 8;
    int num_tokens_to_predict = 4;
    float dropout = 0.2;

    try {
        MultiTokenGPT model(vocab_size, max_input_len, model_dim, num_decoder_layers, num_attn_heads, num_tokens_to_predict, dropout);
        model.to(device);
        torch::Tensor x = torch::randint(0, vocab_size, {1, max_input_len}).to(device);
        auto predictions = model.forward(x);
        std::cout << "Prediction shape: " << predictions.sizes() << std::endl;
        auto predicted_tokens = predictions.argmax(-1).to(torch::kCPU);
        std::cout << "Predicted tokens: ";
        for (int i = 0; i < predicted_tokens.size(1); ++i) {
            std::cout << predicted_tokens[0][i].item<int>() << " ";
        }
        std::cout << std::endl;
    } catch (const c10::Error& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        std::cerr << e.backtrace() << std::endl;
    }
    return 0;
}
