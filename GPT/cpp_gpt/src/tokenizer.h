#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <torch/torch.h>
#include "../../include/json/json.hpp"

using json = nlohmann::json;

#include <algorithm>
#include <filesystem>

struct Tokenizer {
    std::unordered_map<std::string, int> vocab;
    std::unordered_map<int, std::string> id_to_token;
    int max_token_length = 0;

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

            if (key.size() > max_token_length) {
                max_token_length = key.size();
            }
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

            for (size_t j = std::min(i + max_token_length, processed_text.size()); j > i; --j) {
                std::string substr = processed_text.substr(i, j - i);
                if (vocab.find(substr) != vocab.end()) {
                    token = substr;
                    token_id = vocab[token];
                    break;
                }
            }

            if (token_id != -1) {
                token_ids.push_back(token_id);
                i += token.size();
            } else {
                token = "<pad>";
                token_id = vocab[token];
                token_ids.push_back(token_id);
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

    int get_vocab_size() {
        return vocab.size();
    }

    void wikidump_to_dataset(const std::string& input_dir, const std::string& output_file, int max_input_len, int pairs_per_wikipage) {
        std::cout << "Starting dataset creation..." << std::endl;

        std::string line;
        std::string current_doc_text;
        int pair_counter = 0;
        int token_counter = 0;
        bool in_doc = false;

        nlohmann::json json_dataset;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(input_dir)) {
            std::cout << entry << std::endl;
            if (entry.is_regular_file()) {
                std::ifstream infile(entry.path());
                if (!infile) {
                    std::cerr << "Error opening file: " << entry.path() << std::endl;
                    continue;
                }

                while (std::getline(infile, line)) {
                    if (line.find("<doc") != std::string::npos) {
                        in_doc = true;
                        current_doc_text.clear();
                    }
                    else if (line.find("</doc>") != std::string::npos) {
                        in_doc = false;
                        auto tokens = encode(current_doc_text);
                        size_t total_tokens = tokens.size(0);

                        if (total_tokens < 300) {
                            continue;
                        }

                        torch::Tensor random_start_pos = torch::randperm(total_tokens - 4).narrow(0, 0, pairs_per_wikipage);

                        for (int i = 0; i < pairs_per_wikipage; ++i) {
                            int start_pos = random_start_pos[i].item<int>();
                            int length = torch::randint(1, max_input_len, {1})[0].item<int>();
                            length = std::min(length, int(total_tokens - start_pos - 4));

                            auto x_tensor = tokens.narrow(0, start_pos, length);
                            auto y_tensor = tokens.narrow(0, start_pos + length, 4);

                            std::vector<int64_t> x(x_tensor.data_ptr<int64_t>(), x_tensor.data_ptr<int64_t>() + x_tensor.numel());
                            std::vector<int64_t> y(y_tensor.data_ptr<int64_t>(), y_tensor.data_ptr<int64_t>() + y_tensor.numel());
                        

                            if (!torch::any(x_tensor == vocab["<pad>"]).item<bool>()) {
                                nlohmann::json training_pair;
                                training_pair["x"] = x;
                                training_pair["y"] = y;
                                json_dataset["data"].push_back(training_pair);

                                token_counter += length;
                                pair_counter++;
                            }
                        }
                    }
                    else if (in_doc && line != "") {
                        current_doc_text += line + " ";
                    }
                }
            }
            save_statistics(pair_counter, token_counter);
        }
        save_json_file(json_dataset, output_file);
    }

    void save_statistics(int pair_counter, int token_counter) {
        json statistics;
        statistics["pair_counter"] = pair_counter;
        statistics["token_counter"] = token_counter;
        save_json_file(statistics, "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_stat_F.json");
    }

    void save_json_file(json json_data, std::string filename) {
        std::ofstream json_file(filename);
        if (json_file.is_open()) {
            json_file << json_data.dump(4);
            json_file.close();
        }
        else {
            std::cerr << "Error opening JSON output file: " << filename << std::endl;
        }
    }
};