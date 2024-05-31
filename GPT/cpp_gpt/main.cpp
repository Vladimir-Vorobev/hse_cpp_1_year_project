#include <torch/torch.h>
#include <iostream>
#include "src/tokenizer.h"
#include "src/gpt.h"

int main() {
    torch::Device device(torch::kCUDA);
    if (!torch::cuda::is_available()) {
        std::cout << "CUDA not available, using CPU." << std::endl;
        device = torch::kCPU;
    }

    try {
        Tokenizer tokenizer("vocab.json");

        int vocab_size = tokenizer.get_vocab_size();
        int max_input_len = 1024;
        int model_dim = 512;
        int num_decoder_layers = 8;
        int num_attn_heads = 8;
        int num_tokens_to_predict = 4;
        float dropout = 0.2;

        MultiTokenGPT model(vocab_size, max_input_len, model_dim, num_decoder_layers, num_attn_heads, num_tokens_to_predict, dropout);
        model.to(device);
        std::vector<std::string> texts;
        texts.push_back("How are you?");
        texts.push_back("Standard C++ Foundation - a non-profit organization that promotes the use and understanding of standard C++.");
        texts.push_back("Hi!      How are\n you?");
        torch::Tensor x = tokenizer.batch_encode(texts, max_input_len).to(device);
        std::cout << "Input: " << tokenizer.batch_decode(x) << std::endl;
        std::cout << "Encoded input: " << x << std::endl;
        auto predictions = model.forward(x);
        std::cout << "Prediction shape: " << predictions.sizes() << std::endl;
        auto predicted_tokens = predictions.argmax(-1).to(torch::kCPU);
        std::cout << "Predicted tokens: " << predicted_tokens << std::endl;
        std::cout << "Predicted text: " << tokenizer.batch_decode(predicted_tokens) << std::endl;
    } catch (const c10::Error& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl; 
        std::cerr << e.backtrace() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
    }
    return 0;
}
