#include <torch/torch.h>

// it is a templete to verify that build can be built and run, the real GPT coming soon...
struct GPT : torch::nn::Module {
    torch::nn::Linear linear{nullptr};

    GPT()
        : linear(register_module("linear", torch::nn::Linear(10, 1)))
    {}

    torch::Tensor forward(torch::Tensor input) {
        return linear(input);
    }
};