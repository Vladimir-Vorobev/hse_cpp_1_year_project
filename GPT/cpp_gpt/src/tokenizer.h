#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <torch/torch.h>
#include "../../include/json/json.hpp"

using json = nlohmann::json;

std::string replace_substrings(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

std::string to_lower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

struct Tokenizer {
    std::unordered_map<std::string, int> vocab;
    std::unordered_map<int, std::string> id_to_token;

    Tokenizer(const std::string& vocab_file) {
        std::ifstream file(vocab_file);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open vocab file: " << vocab_file << std::endl;
            throw std::runtime_error("Unable to open vocab file");
        }
        json j;
        file >> j;

        for (auto& item : j.items()) {
            auto key = replace_substrings(item.key(), "</w>", " ");
            vocab[key] = item.value();
            id_to_token[item.value()] = key;
        }

        if (vocab.find("<pad>") == vocab.end()) {
            vocab["<pad>"] = vocab.size();
        }

        id_to_token[vocab["<pad>"]] = "<pad>";
    }

    torch::Tensor encode(const std::string& text) {
        std::vector<int> token_ids;
        size_t i = 0;

        std::string processed_text = to_lower(text);

        while (i < processed_text.size()) {
            std::string token;
            int token_id = -1;

            for (size_t j = processed_text.size(); j > i; --j) {
                std::string substr = processed_text.substr(i, j - i);
                if (vocab.find(substr) != vocab.end()) {
                    token = substr;
                    token_id = vocab[substr];
                    break;
                }
            }

            if (token_id != -1) {
                token_ids.push_back(token_id);
                i += token.size();
            } else {
                ++i;
            }
        }

        return torch::tensor(token_ids);
    }

    std::string decode(const torch::Tensor& token_tensor) {
        std::string text;
        for (int i = 0; i < token_tensor.size(0); ++i) {
            int id = token_tensor[i].item<int>();
            std::string token = id_to_token[id];
            text += token;
        }
        return text;
    }

    torch::Tensor batch_encode(std::vector<std::string>& texts, int64_t max_len) {
        std::vector<torch::Tensor> all_token_tensors;
        int64_t max_seq_len = 0;

        for (auto& text : texts) {
            auto token_tensor = encode(text);
            if (token_tensor.size(0) > max_len) {
                token_tensor = token_tensor.slice(0, 0, max_len);
            }
            max_seq_len = std::max(max_seq_len, token_tensor.size(0));
            all_token_tensors.push_back(token_tensor);
        }

        max_seq_len = std::min(max_seq_len, max_len);

        for (auto& token_tensor : all_token_tensors) {
            if (token_tensor.size(0) < max_seq_len) {
                token_tensor = torch::cat({token_tensor, torch::full({max_seq_len - token_tensor.size(0)}, vocab["<pad>"])}, 0);
            }
        }

        return torch::stack(all_token_tensors);
    }

    std::vector<std::string> batch_decode(const torch::Tensor& tokens_tensor) {
        std::vector<std::string> texts;
        for (int i = 0; i < tokens_tensor.size(0); ++i) {
            texts.push_back(decode(tokens_tensor[i]));
        }
        return texts;
    }

    int get_vocab_size() {
        return vocab.size();
    }
};