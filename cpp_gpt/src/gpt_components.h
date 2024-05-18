#include <torch/torch.h>

struct RoPEImpl : torch::nn::Module {
    torch::Tensor pe{nullptr};

    RoPEImpl(int max_len, int model_dim) {
        torch::Device device(torch::kCUDA);

        pe = register_buffer("pe", torch::zeros({max_len, model_dim}).to(device));
        torch::Tensor position = torch::arange(0, max_len, torch::kFloat32).unsqueeze(1).to(device);
        torch::Tensor divTerm = torch::exp(torch::arange(0, model_dim, 2, torch::kFloat32).to(device) * (-std::log(10000.0) / model_dim));
        torch::Tensor angle = position * divTerm;

        pe.index_put_({"...", torch::indexing::Slice(0, torch::indexing::None, 2)}, torch::sin(angle));
        pe.index_put_({"...", torch::indexing::Slice(1, torch::indexing::None, 2)}, torch::cos(angle));
    }

    torch::Tensor forward(torch::Tensor x) {
        int seq_len = x.size(1);
        auto pe_slice = pe.index({torch::indexing::Slice(0, seq_len)}).unsqueeze(0);
        auto x_even = x.index({"...", torch::indexing::Slice(0, torch::indexing::None, 2)});
        auto x_odd = x.index({"...", torch::indexing::Slice(1, torch::indexing::None, 2)});

        auto x_new_even = x_even * pe_slice.index({"...", torch::indexing::Slice(0, torch::indexing::None, 2)}) - x_odd * pe_slice.index({"...", torch::indexing::Slice(1, torch::indexing::None, 2)});
        auto x_new_odd = x_even * pe_slice.index({"...", torch::indexing::Slice(1, torch::indexing::None, 2)}) + x_odd * pe_slice.index({"...", torch::indexing::Slice(0, torch::indexing::None, 2)});

        return torch::cat({x_new_even.unsqueeze(-1), x_new_odd.unsqueeze(-1)}, -1).flatten(-2, -1);
    }
};

TORCH_MODULE(RoPE);

struct SingleHeadAttentionImpl : torch::nn::Module {
    torch::nn::Linear query{nullptr};
    torch::nn::Linear key{nullptr};
    torch::nn::Linear value{nullptr};
    RoPE rope{nullptr};
    int head_size;

    SingleHeadAttentionImpl(int model_dim, int max_input_len, int head_size) : rope(max_input_len, head_size), head_size(head_size) {
        query = register_module("query", torch::nn::Linear(torch::nn::LinearOptions(model_dim, head_size).bias(false)));
        key = register_module("key", torch::nn::Linear(torch::nn::LinearOptions(model_dim, head_size).bias(false)));
        value = register_module("value", torch::nn::Linear(torch::nn::LinearOptions(model_dim, head_size).bias(false)));
    }

    torch::Tensor forward(torch::Tensor x) {
        auto q = rope(query(x));
        auto k = rope(key(x));
        auto v = value(x);

        auto scores = torch::matmul(q, k.transpose(1, 2)) / std::sqrt(head_size);
        int context_length = k.size(1);

        auto mask = torch::tril(torch::ones({context_length, context_length}, x.options())) == 0;
        scores = scores.masked_fill(mask, -std::numeric_limits<float>::infinity());
        scores = torch::nn::functional::softmax(scores, 2);

        return torch::matmul(scores, v);
    }
};

TORCH_MODULE(SingleHeadAttention);

struct MultiHeadAttentionImpl : torch::nn::Module {
    torch::nn::ModuleList attn_heads{nullptr};
    torch::nn::Linear out_linear{nullptr};

    MultiHeadAttentionImpl(int model_dim, int num_heads, int max_input_len) {
        attn_heads = register_module("attn_heads", torch::nn::ModuleList());

        for (int i = 0; i < num_heads; ++i) {
            int head_size = model_dim / num_heads;
            attn_heads->push_back(SingleHeadAttention(model_dim, max_input_len, head_size));
        }

        out_linear = register_module("out_linear", torch::nn::Linear(model_dim, model_dim));
    }

    torch::Tensor forward(torch::Tensor x) {
        std::vector<torch::Tensor> heads;
        for (const auto& head : *attn_heads) {
            heads.push_back(head->as<SingleHeadAttention>()->forward(x));
        }

        auto attnention = out_linear(torch::cat(heads, -1));
        return attnention;
    }
};

TORCH_MODULE(MultiHeadAttention);

struct DecoderMLPImpl : torch::nn::Module {
    torch::nn::Linear linear_1{nullptr};
    torch::nn::Linear linear_2{nullptr};
    torch::nn::Dropout dropout{nullptr};
    torch::nn::ReLU relu{nullptr};

    DecoderMLPImpl(int model_dim, float dropout) : dropout(dropout) {
        linear_1 = register_module("linear_1", torch::nn::Linear(model_dim, model_dim * 4));
        linear_2 = register_module("linear_2", torch::nn::Linear(model_dim * 4, model_dim));
        relu = torch::nn::ReLU();
    }

    torch::Tensor forward(torch::Tensor x) {
        x = relu(linear_1(x));
        x = dropout(linear_2(x));
        return x;
    }
};

TORCH_MODULE(DecoderMLP);

struct DecoderLayerImpl : torch::nn::Module {
    torch::nn::LayerNorm layer_norm_1{nullptr};
    torch::nn::LayerNorm layer_norm_2{nullptr};
    MultiHeadAttention attention{nullptr};
    DecoderMLP mlp{nullptr};

    DecoderLayerImpl(int model_dim, int num_attn_heads, int max_input_len, float dropout) {
        layer_norm_1 = register_module("layer_norm_1", torch::nn::LayerNorm(torch::nn::LayerNormOptions({model_dim})));
        layer_norm_2 = register_module("layer_norm_2", torch::nn::LayerNorm(torch::nn::LayerNormOptions({model_dim})));
        attention = register_module("attention", MultiHeadAttention(model_dim, num_attn_heads, max_input_len));
        mlp = register_module("mlp", DecoderMLP(model_dim, dropout));
    }

    torch::Tensor forward(torch::Tensor x) {
        x = x + attention(layer_norm_1(x));
        x = x + mlp(layer_norm_2(x));
        return x;
    }
};

TORCH_MODULE(DecoderLayer);