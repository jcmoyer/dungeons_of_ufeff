#include "audio.hpp"

#include <cassert>
#include <filesystem>

#include "mathutil.hpp"
#include "stb_vorbis.h"

static audio_buffer load_audio(const char* filename)
{
    audio_buffer buffer;
    buffer.sample_count = stb_vorbis_decode_filename(filename, &buffer.channels, &buffer.sample_rate, &buffer.samples);
    return std::move(buffer);
}

static void load_audio(const char* filename, audio_buffer& buffer)
{
    buffer.sample_count = stb_vorbis_decode_filename(filename, &buffer.channels, &buffer.sample_rate, &buffer.samples);
}

void audio_system::init()
{
    // probably will never need this many but this is not something that we want to keep reallocating
    // also !!!!!IMPORTANT!!!!! this modifies tracks without a mutex so it MUST be done before starting
    // the audio callback or decode thread
    tracks.reserve(1024);

    SDL_AudioSpec want, spec;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 128;
    want.callback = audio_callback;
    want.userdata = this;
    device = SDL_OpenAudioDevice(NULL, 0, &want, &spec, 0);
    SDL_PauseAudioDevice(device, 0);

    decode_thread = std::thread([&]() {
        std::vector<decode_request> local_queue;

        while (true)
        {
            // wait for queue to fill up
            std::unique_lock<std::mutex> lk(decode_queue_m);
            decode_cv.wait(lk, [&]() { return decode_queue.size(); });

            // quickly take the decode queue
            std::swap(local_queue, decode_queue);

            // release as soon as we're done with the queue
            lk.unlock();

            // load requests sequentially and put them into the tracklist
            for (const decode_request& req : local_queue)
            {
                // empty filename kills the thread
                if (req.filename.size() == 0)
                    return;

                audio_track t;
                t.buffer = get_or_load(req.filename.c_str());
                t.loop = req.loop;
                t.parameters = std::move(req.parameters);
                std::scoped_lock lk(tracks_m);
                tracks.push_back(std::move(t));
            }

            local_queue.clear();
        }
    });
}

audio_buffer* audio_system::get_or_load(const char* filename)
{
    assert(std::filesystem::exists(filename));

    std::string filename_str{filename};
    cache_m.lock();
    if (auto it = cache.find(filename_str); it != cache.end())
    {
        cache_m.unlock();
        return &it->second;
    }
    else
    {
        // Atomic insert so we can observe that a load for this filename
        // already exists if this function is called again, but before the
        // load has completed. This prevents the same resource from being
        // loaded multiple times.
        auto [new_it, success] = cache.emplace(std::make_pair(filename_str, audio_buffer{}));
        audio_buffer* ptr = &new_it->second;
        cache_m.unlock();

        // now actually load the data
        load_audio(filename, *ptr);

        return ptr;
    }
}

std::shared_ptr<audio_parameters> audio_system::play_sound(const char* filename)
{
    auto parameters = std::make_shared<audio_parameters>();
    {
        std::scoped_lock lk(decode_queue_m);
        decode_queue.push_back({filename, false, parameters});
    }
    decode_cv.notify_one();
    return parameters;
}

std::shared_ptr<audio_parameters> audio_system::play_music(const char* filename)
{
    auto parameters = std::make_shared<audio_parameters>();
    {
        std::scoped_lock lk(decode_queue_m);
        decode_queue.push_back({filename, true, parameters});
    }
    decode_cv.notify_one();
    return parameters;
}

void audio_system::audio_callback(void* userdata, Uint8* stream, int len)
{
    audio_system* self = (audio_system*)userdata;
    SDL_memset(stream, 0, len);
    short* data_base = (short*)stream;

    std::scoped_lock lk(self->tracks_m);

    for (audio_track& track : self->tracks)
    {
        audio_buffer* buffer = track.buffer;
        float volume = track.parameters->volume;

        if (track.parameters->paused)
            continue;

        if (!buffer->sample_count)
            continue;

        short* data = data_base;

        int written = 0;
        for (int samp = 0; samp < len / sizeof(short); ++samp)
        {
            if (track.done)
                break;

            *data = (short)clamp<int>(*data + static_cast<short>(*(buffer->samples + track.cursor) * volume), SHRT_MIN, SHRT_MAX);
            ++data;
            ++track.cursor;

            if (track.cursor == 2 * buffer->sample_count)
            {
                if (track.loop)
                {
                    track.cursor = 0;
                }
                else
                {
                    track.done = true;
                }
            }
        }
    }

    // TODO: this was raising an STL iterator-out-of-range error after reserving memory ahead of time
    // it seems like remove_if returns an invalid iterator if the collection is empty; need to investigate
    if (self->tracks.size())
    {
        self->tracks.erase(
            std::remove_if(self->tracks.begin(), self->tracks.end(), [](const audio_track& t) {
                return t.done || t.parameters->done;
            }),
            self->tracks.end());
    }
}

audio_system::~audio_system()
{
    // signal decode thread to exit
    play_sound("");
    decode_thread.join();
}