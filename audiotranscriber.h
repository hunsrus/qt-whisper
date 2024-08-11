#ifndef AUDIOTRANSCRIBER_H
#define AUDIOTRANSCRIBER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <regex>
#include <atomic>
#include <cstdio>
#include <fstream>
#include <cstdint>

class AudioTranscriber {
public:
    AudioTranscriber(){}

    void start() {
        this->PROC_STARTED = true;
        transcribeThread_ = std::thread(&AudioTranscriber::runCommand, this);
    }

    void join() {
        if(this->PROC_STARTED)
        {
            while(!transcribeThread_.joinable()) {}
            transcribeThread_.join();
        }
    }

    double getWavDuration(const std::string &filename) {
        std::string command = "ffmpeg -i " + filename + " 2>&1 | grep Duration | cut -d ' ' -f 4 | sed s/,//";
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run command" << std::endl;
            return -1;
        }

        char buffer[128];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        pclose(pipe);

        int hours, minutes;
        float seconds;
        sscanf(result.c_str(), "%d:%d:%f", &hours, &minutes, &seconds);

        return hours * 3600 + minutes * 60 + seconds;
    }

    int getProgress() {
        return this->progress_;
    }

    void setWhisperPath(std::string whisperPath)
    {
        this->whisperPath_ = whisperPath;
    }

    void setModel(std::string model)
    {
        this->model_ = model;
    }

    void setAudioFile(std::string audioFile)
    {
        this->audioFile_ = audioFile;
    }

    std::string getAudioFile(void)
    {
        return this->audioFile_;
    }

    void killProcess(void) {
        this->PROC_SHOULD_RUN = false;
    }

private:
    void runCommand() {
        std::string fullCommand = this->whisperPath_+" -l es -m "+this->model_+" -f "+this->audioFile_;
        FILE* pipe = popen(fullCommand.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run command." << std::endl;
            return;
        }

        char buffer[128];
        std::regex timeRegex(R"(\[(\d+):(\d+):(\d+)\.(\d+)\s-->\s(\d+):(\d+):(\d+)\.(\d+)\])");
        std::smatch match;

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr && this->PROC_SHOULD_RUN) {
            std::string line(buffer);
            //std::cout << line; // Opcional: para ver la salida en la consola

            if (std::regex_search(line, match, timeRegex)) {
                /*
                int startHours = std::stoi(match[1].str());
                int startMinutes = std::stoi(match[2].str());
                int startSeconds = std::stoi(match[3].str());
                int startMilliseconds = std::stoi(match[4].str());
                */

                // Puedes optar por usar el tiempo de inicio o el tiempo de fin
                // Aquí se usa el tiempo de fin:
                int endHours = std::stoi(match[5].str());
                int endMinutes = std::stoi(match[6].str());
                int endSeconds = std::stoi(match[7].str());
                int endMilliseconds = std::stoi(match[8].str());

                int totalSeconds = endHours * 3600 + endMinutes * 60 + endSeconds;
                progress_ = totalSeconds;
            }
        }

        std::cerr << "PIPE CERRADO" << std::endl;

        pclose(pipe);
        done_ = true;
    }

    std::string whisperPath_;
    std::string model_;
    std::string audioFile_;
    std::atomic<int> progress_;
    std::atomic<bool> done_;
    std::thread transcribeThread_;

    std::atomic<bool> PROC_SHOULD_RUN = true;
    std::atomic<bool> PROC_STARTED = false;
};

#endif // AUDIOTRANSCRIBER_H
