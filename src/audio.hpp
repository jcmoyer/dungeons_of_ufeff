#pragma once

#include <SDL.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

struct audio_buffer
{
    int channels = 0;
    int sample_rate = 0;
    short* samples = nullptr;
    std::atomic<int> sample_count = 0;

    audio_buffer() = default;
    audio_buffer(audio_buffer&& rhs)
        : channels{rhs.channels}, sample_rate{rhs.sample_rate}, samples{rhs.samples}, sample_count{rhs.sample_count.load()}
    {
    }
};

struct audio_parameters
{
    std::atomic<float> volume = 1.f;
    std::atomic<bool> done = false;
    std::atomic<bool> paused = false;
};

struct audio_track
{
    audio_buffer* buffer;
    int cursor = 0;
    bool loop = false;
    bool done = false;
    std::shared_ptr<audio_parameters> parameters;
};

struct decode_request
{
    std::string filename;
    bool loop;
    std::shared_ptr<audio_parameters> parameters;
};

class audio_system
{
public:
    audio_system() = default;
    audio_system(const audio_system&) = delete;
    audio_system& operator=(const audio_system&) = delete;

    ~audio_system();

    void init();

    std::shared_ptr<audio_parameters> play_sound(const char* filename);
    std::shared_ptr<audio_parameters> play_music(const char* filename);

    audio_buffer* get_or_load(const char* filename);

private:
    static void audio_callback(void* userdata, Uint8* stream, int len);

    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;

    std::mutex tracks_m;
    std::vector<audio_track> tracks;

    std::mutex cache_m;
    std::unordered_map<std::string, audio_buffer> cache;

    //== load / decode thread ======================================
    std::thread decode_thread;
    std::mutex decode_queue_m;
    std::vector<decode_request> decode_queue;
    std::condition_variable decode_cv;
};