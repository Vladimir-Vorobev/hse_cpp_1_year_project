#include <torch/torch.h>
#include "gpt_components.h"

struct MultiTokenGPT : torch::nn::Module {
    torch::nn::Embedding embedding{nullptr};
    torch::nn::Sequential shared_trunk{nullptr};
    torch::nn::ModuleList token_prediction_heads{nullptr};
    torch::nn::Linear unembedding{nullptr};
    int num_tokens_to_predict;

    MultiTokenGPT(int vocab_size, int max_input_len, int model_dim, int num_decoder_layers, int num_attn_heads, int num_tokens_to_predict, float dropout)
        : num_tokens_to_predict(num_tokens_to_predict)
    {
        embedding = register_module("embedding", torch::nn::Embedding(vocab_size, model_dim));
        shared_trunk = register_module("shared_trunk", torch::nn::Sequential());

        for (int i = 0; i < num_decoder_layers; ++i) {
            shared_trunk->push_back(DecoderLayer(model_dim, num_attn_heads, max_input_len, dropout));
        }

        token_prediction_heads = register_module("token_prediction_heads", torch::nn::ModuleList());
        for (int i = 0; i < num_tokens_to_predict; ++i) {
            token_prediction_heads->push_back(DecoderLayer(model_dim, num_attn_heads, max_input_len, dropout));
        }

        unembedding = register_module("unembedding", torch::nn::Linear(model_dim, vocab_size));
    }

    torch::Tensor forward(torch::Tensor x) {
        auto embeddings = embedding(x);
        auto shared_output = shared_trunk->forward(embeddings);

        std::vector<torch::Tensor> predictions;
        for (const auto& head : *token_prediction_heads) {
            auto head_output = head->as<DecoderLayer>()->forward(shared_output);
            auto last_token_output = head_output.index({torch::indexing::Slice(), -1, torch::indexing::Slice()});
            auto logits = unembedding(last_token_output);
            predictions.push_back(logits.unsqueeze(1));
        }

        auto prediction_tensor = torch::cat(predictions, 1);

        return prediction_tensor;
    }
};