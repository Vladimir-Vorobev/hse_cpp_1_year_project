#include <torch/torch.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include "src/tokenizer.h"
#include "src/gpt.h"
#include "../include/json/json.hpp"

std::queue<std::pair<std::vector<int>, std::vector<int>>> train_data_from_file(const std::string& file_path) {
    std::ifstream ifs(file_path);
    nlohmann::json json_data;
    ifs >> json_data;

    std::queue<std::pair<std::vector<int>, std::vector<int>>> batch_queue;

    for (const auto& entry : json_data["data"]) {
        std::vector<int> x = entry["x"].get<std::vector<int>>();
        std::vector<int> y = entry["y"].get<std::vector<int>>();
        batch_queue.push({x, y});
    }

    return batch_queue;
}

std::vector<torch::Tensor> pad_x(const std::vector<torch::Tensor>& x, int pad_token) {
    size_t max_length = 0;
    for (const auto& item : x) {
        if (item.size(0) > max_length) {
            max_length = item.size(0);
        }
    }

    std::vector<torch::Tensor> padded_x;
    for (const auto& item : x) {
        auto padded_item = torch::full({int(max_length)}, pad_token);
        padded_item.index({torch::indexing::Slice(0, item.size(0))}) = item;
        padded_x.push_back(padded_item);
    }

    return padded_x;
}

struct DataLoader {
    std::vector<std::string> json_files;
    size_t batch_size;
    size_t current_file_index;
    size_t current_batch_start_index;
    int pad_token;
    std::queue<std::pair<std::vector<int>, std::vector<int>>> current_batch_queue;

    DataLoader(const std::vector<std::string>& json_files, size_t batch_size, int pad_token) : json_files(json_files), batch_size(batch_size), current_file_index(0), current_batch_start_index(0), pad_token(pad_token) {
        load_next_file();
    }

    bool has_next() {
        return !current_batch_queue.empty() || current_file_index < json_files.size();
    }

    std::pair<torch::Tensor, torch::Tensor> next() {
        if (current_batch_queue.empty() && current_file_index < json_files.size()) {
            load_next_file();
        }

        std::vector<torch::Tensor> x_tensors, y_tensors;
        for (size_t i = 0; i < batch_size && !current_batch_queue.empty(); ++i) {
            auto [x, y] = current_batch_queue.front();
            current_batch_queue.pop();

            x_tensors.push_back(torch::tensor(x, torch::kLong));
            y_tensors.push_back(torch::tensor(y, torch::kLong));
        }

        x_tensors = pad_x(x_tensors, pad_token);

        return {torch::stack(x_tensors), torch::stack(y_tensors)};
    }

    void load_next_file() {
        if (current_file_index < json_files.size()) {
            current_batch_queue = train_data_from_file(json_files[current_file_index]);
            current_file_index++;
        }
    }
};

void train(std::shared_ptr<MultiTokenGPT> model, torch::optim::Optimizer& optimizer, DataLoader& dataloader, size_t epoch, int pad_token, size_t batch_size) {
    model->train();
    torch::Device device(torch::kCUDA);
    torch::autograd::GradMode::set_enabled(true);

    std::vector<double> loss_history;

    int batch_idx = 0;
    double total_loss = 0.0;

    while (dataloader.has_next()) {
        auto [inputs, targets] = dataloader.next();

        inputs = inputs.to(device);
        targets = targets.to(device);

        optimizer.zero_grad();
        auto embeddings = model->embedding->forward(inputs);
        auto shared_output = model->shared_trunk->forward(embeddings);

        auto loss = torch::tensor(0.0, torch::dtype(torch::kFloat32).device(device));

        for (int i = 0; i < model->num_tokens_to_predict; ++i) {
            auto head_output = model->token_prediction_heads[i]->as<DecoderLayer>()->forward(shared_output);
            auto last_token_output = head_output.index({torch::indexing::Slice(), -1, torch::indexing::Slice()});
            auto logits = model->unembedding->forward(last_token_output);
            auto target = targets.index({torch::indexing::Slice(), i});

            auto batch_loss = torch::nn::functional::cross_entropy(logits, target, torch::nn::functional::CrossEntropyFuncOptions().ignore_index(pad_token));

            loss = loss + batch_loss;
        }

        loss.backward();
        optimizer.step();

        total_loss += loss.item<double>();

        if (batch_idx % 20000 == 0) {
            torch::save(model, "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/train/multi_token_gpt_epoch_" + std::to_string(epoch) + "_batch_" + std::to_string(batch_idx) + ".pt");
        }

        if (batch_idx % 1000 == 0) {
            loss_history.push_back(loss.item<double>());
            std::cout << "Epoch [" << epoch << "], " << float(batch_idx) / (float(28989101) / float(batch_size)) * 100 << "%, Loss: " << loss.item<float>() << std::endl;
        }

        // cudaDeviceSynchronize();
        // cudaFree(0);

        batch_idx++;
    }

    json epoch_stat;
    epoch_stat["loss"] = loss_history;
    std::string filename = "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/train/loss/epoch_" + std::to_string(epoch) + ".json";

    std::ofstream json_file(filename);
    if (json_file.is_open()) {
        json_file << epoch_stat.dump(4);
        json_file.close();
    }
    else {
        std::cerr << "Error opening JSON output file: " << filename << std::endl;
    }

    double average_loss = total_loss / batch_idx;
    std::cout << "Epoch [" << epoch << "], Average Loss: " << average_loss << std::endl;
}

size_t count_model_parameters(const std::shared_ptr<MultiTokenGPT>& model) {
    size_t total_params = 0;
    for (const auto& p : model->parameters()) {
        total_params += p.numel();
    }
    return total_params;
}

int main() {
    try {
        int command;
        std::cout << "To create dataset write 1\nTo train model write 2\nTo inference model write 3" << std::endl;
        std::cin >> command;

        Tokenizer tokenizer("C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/cpp_gpt/src/vocab.json");
        int vocab_size = tokenizer.get_vocab_size();
        int max_input_len = 512;
        int model_dim = 768;
        int num_decoder_layers = 12;
        int num_attn_heads = 12;
        int num_tokens_to_predict = 1;
        float dropout = 0.1;
        size_t batch_size = 32;
        size_t num_epochs = 1;
        float learning_rate = 5e-4;

        if (command == 1) {
            std::string approval;
            std::cout << "Are you sure that you want to create a new dataset? (n/Y)" << std::endl;
            std::cin >> approval;

            if (approval == "Y") {
                // int pairs_per_wikipage = 25;

                // tokenizer.wikidump_to_dataset(
                //     "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/enwikipages_F",
                //     "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_F.json",
                //     max_input_len,
                //     pairs_per_wikipage
                // );
            }
        }
        else if (command == 2) {
            std::string approval;
            std::cout << "Are you sure that you want to train a new model? (n/Y)" << std::endl;
            std::cin >> approval;

            if (approval == "Y") {
                torch::Device device(torch::kCUDA);
                if (!torch::cuda::is_available()) {
                    std::cout << "CUDA not available, using CPU." << std::endl;
                    device = torch::kCPU;
                }

                auto model = std::make_shared<MultiTokenGPT>(vocab_size, max_input_len, model_dim, num_decoder_layers, num_attn_heads, num_tokens_to_predict, dropout);
                model->to(device);
                // cudaDeviceSynchronize();
                // cudaFree(0);

                size_t params_num = count_model_parameters(model);
                std::cout << "Total number of model parameters: " << params_num << std::endl;

                std::vector<std::string> json_files = {
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_A.json",
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_B.json",
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_C.json",
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_D.json",
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_E.json",
                    "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/dataset/dataset_F.json"
                };
                DataLoader dataloader(json_files, batch_size, tokenizer.vocab["<pad>"]);

                torch::optim::Adam optimizer(model->parameters(), torch::optim::AdamOptions(learning_rate));

                // std::vector<double> epoch_losses;

                for (size_t epoch = 1; epoch <= num_epochs; ++epoch) {
                    train(model, optimizer, dataloader, epoch, tokenizer.vocab["<pad>"], batch_size);
                    torch::save(model, "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/train/multi_token_gpt_epoch_" + std::to_string(epoch) + ".pt");
                    // epoch_losses.push_back(total_loss / total_batches);
                }

                // Plot the loss and save to PNG
                // plt::plot(epoch_losses);
                // plt::xlabel("Epoch");
                // plt::ylabel("Average Loss");
                // plt::title("Training Loss Over Epochs");
                // plt::save("training_loss.png");

                // Save the model
                torch::save(model, "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/train/multi_token_gpt_final.pt");
            }
        }
        else if (command == 3) {
            auto model = std::make_shared<MultiTokenGPT>(vocab_size, max_input_len, model_dim, num_decoder_layers, num_attn_heads, num_tokens_to_predict, dropout);
            torch::load(model, "C:/Users/Vladimir/PycharmProjects/hse_cpp_1_year_project/GPT/train/multi_token_gpt_epoch_1_batch_680000.pt");
            model->to(torch::kCUDA);

            while (true) {
                std::vector<std::string> input_texts;
                std::string input_text;

                do {
                    std::cout << "Enter new input.";
                    std::getline(std::cin, input_text);
                } while (input_text.empty());

                if (input_text == "exit") {
                    break;
                }
                // std::cout << std::endl;

                input_texts.push_back(input_text);

                auto input_tokens = tokenizer.batch_encode(input_texts, 512);
                input_tokens = input_tokens.to(torch::kCUDA);

                std::vector<int64_t> predicted_token_ids;
                std::unordered_map<int64_t, int> token_frequencies;
                std::unordered_map<int64_t, int> last_predicted_position;
                float distance_decay_factor = 0.9f;
                float frequence_decay_factor = 1.1f;

                while (true) {
                    torch::Tensor output = model->forward(input_tokens);

                    for (int i = 0; i < num_tokens_to_predict; ++i) {
                        auto logits = output.index({0, i, torch::indexing::Slice()});

                        for (const auto& [token, freq] : token_frequencies) {
                            int current_position = predicted_token_ids.size();
                            int last_position = last_predicted_position[token];
                            int distance = current_position - last_position;

                            float penalty = static_cast<float>(freq) * 1.1f * std::pow(distance_decay_factor, distance) * std::pow(frequence_decay_factor, freq);
                            logits.index_put_({token}, logits.index({token}) - penalty);
                        }

                        auto predicted_id = torch::argmax(logits, -1).item<int64_t>();
                        predicted_token_ids.push_back(predicted_id);
                        token_frequencies[predicted_id]++;
                        last_predicted_position[predicted_id] = predicted_token_ids.size();
                
                        input_tokens = torch::cat({input_tokens, torch::tensor({{predicted_id}}, torch::kCUDA)}, 1);
                        input_tokens = input_tokens.index({torch::indexing::Slice(), torch::indexing::Slice(-512, torch::indexing::None)});

                        std::cout << tokenizer.decode(torch::tensor({{predicted_id}}));
                    }
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
    }
    return 0;
}