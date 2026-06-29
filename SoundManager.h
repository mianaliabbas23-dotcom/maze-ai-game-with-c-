#pragma once

// ─────────────────────────────────────────────────────────────
//  SoundManager — Retro chiptune style, zero external files
//  MSVC compatible — no local structs inside functions
//  Uses WAV-in-memory + loadFromMemory (SFML 3 compatible)
// ─────────────────────────────────────────────────────────────

#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Defined at file scope so MSVC accepts it everywhere
struct ChipNote
{
    float freq;
    float beat;
    float dur;
};

class SoundManager
{
public:
    enum ID
    {
        MOVE = 0,
        WALL_HIT,
        AI_STEP,
        CHASE_ALERT,
        WIN,
        LOSE,
        VIZ_TICK,
        START_MUSIC,
        WIN_MUSIC,
        LOSE_MUSIC,
        COUNT
    };

    SoundManager()
    {
        generateAll();
        for (int i = 0; i < COUNT; i++)
        {
            sounds[i] = std::make_unique<sf::Sound>(buffers[i]);
            if (i == START_MUSIC || i == WIN_MUSIC || i == LOSE_MUSIC)
            {
                sounds[i]->setLooping(true);
                sounds[i]->setVolume(50.f);
            }
            else
            {
                sounds[i]->setVolume(80.f);
            }
        }
    }

    void play(ID id)
    {
        if (id < 0 || id >= COUNT || !sounds[id]) return;
        sounds[id]->stop();
        sounds[id]->play();
    }

    void stop(ID id)
    {
        if (id >= 0 && id < COUNT && sounds[id])
            sounds[id]->stop();
    }

    void stopAllMusic()
    {
        stop(START_MUSIC);
        stop(WIN_MUSIC);
        stop(LOSE_MUSIC);
    }

    bool isPlaying(ID id)
    {
        if (id < 0 || id >= COUNT || !sounds[id]) return false;
        return sounds[id]->getStatus() == sf::SoundSource::Status::Playing;
    }

private:
    sf::SoundBuffer            buffers[COUNT];
    std::unique_ptr<sf::Sound> sounds[COUNT];

    static const unsigned int SR = 44100;

    // ── Oscillators ───────────────────────────────────────────

    float squareOsc(float freq, int sample, float duty = 0.5f)
    {
        float phase = std::fmod(freq * (float)sample / (float)SR, 1.f);
        return (phase < duty) ? 1.f : -1.f;
    }

    float triOsc(float freq, int sample)
    {
        float phase = std::fmod(freq * (float)sample / (float)SR, 1.f);
        return (phase < 0.5f) ? (4.f * phase - 1.f) : (3.f - 4.f * phase);
    }

    float noiseOsc()
    {
        return ((float)(rand() % 32767) / 16383.5f) - 1.f;
    }

    // ── Envelope ──────────────────────────────────────────────
    float calcEnv(int i, int total, float atk, float rel)
    {
        float t = (float)i / (float)total;
        if (t < atk)         return t / atk;
        if (t > 1.f - rel)   return (1.f - t) / rel;
        return 1.f;
    }

    // ── Buffer helpers ────────────────────────────────────────
    std::vector<int16_t> makeBuffer(float dur)
    {
        return std::vector<int16_t>((int)((float)SR * dur), 0);
    }

    void mixIn(std::vector<int16_t>& buf, int i, float val)
    {
        if (i < 0 || i >= (int)buf.size()) return;
        int v = (int)buf[i] + (int)(val * 32767.f);
        if (v >  32767) v =  32767;
        if (v < -32767) v = -32767;
        buf[i] = (int16_t)v;
    }

    void squareNote(std::vector<int16_t>& buf,
                    float freq, float vol,
                    float startSec, float durSec,
                    float duty = 0.5f,
                    float atkR = 0.01f, float relR = 0.15f)
    {
        int start = (int)(startSec * (float)SR);
        int len   = (int)(durSec   * (float)SR);
        for (int i = 0; i < len; i++)
        {
            float e = calcEnv(i, len, atkR, relR);
            float s = squareOsc(freq, i, duty);
            mixIn(buf, start + i, s * e * vol);
        }
    }

    void triNote(std::vector<int16_t>& buf,
                 float freq, float vol,
                 float startSec, float durSec,
                 float atkR = 0.01f, float relR = 0.2f)
    {
        int start = (int)(startSec * (float)SR);
        int len   = (int)(durSec   * (float)SR);
        for (int i = 0; i < len; i++)
        {
            float e = calcEnv(i, len, atkR, relR);
            float s = triOsc(freq, i);
            mixIn(buf, start + i, s * e * vol);
        }
    }

    void squareSweep(std::vector<int16_t>& buf,
                     float f0, float f1, float vol,
                     float startSec, float durSec)
    {
        int start = (int)(startSec * (float)SR);
        int len   = (int)(durSec   * (float)SR);
        for (int i = 0; i < len; i++)
        {
            float t    = (float)i / (float)len;
            float freq = f0 + (f1 - f0) * t;
            float e    = calcEnv(i, len, 0.005f, 0.2f);
            float s    = squareOsc(freq, i);
            mixIn(buf, start + i, s * e * vol);
        }
    }

    void noiseBurst(std::vector<int16_t>& buf,
                    float vol, float startSec, float durSec)
    {
        int start = (int)(startSec * (float)SR);
        int len   = (int)(durSec   * (float)SR);
        for (int i = 0; i < len; i++)
        {
            float e = calcEnv(i, len, 0.001f, 0.7f);
            mixIn(buf, start + i, noiseOsc() * e * vol);
        }
    }

    // ── WAV builder ───────────────────────────────────────────
    void load(ID id, const std::vector<int16_t>& samples)
    {
        std::vector<uint8_t> wav = buildWav(samples);
        buffers[id].loadFromMemory(wav.data(), wav.size());
    }

    std::vector<uint8_t> buildWav(const std::vector<int16_t>& samples)
    {
        uint32_t dataSize = (uint32_t)(samples.size() * 2);
        uint32_t fileSize = 36 + dataSize;
        uint32_t byteRate = SR * 2;
        std::vector<uint8_t> w;
        w.reserve(44 + dataSize);
        auto w32 = [&](uint32_t v){
            w.push_back((uint8_t)(v));
            w.push_back((uint8_t)(v >> 8));
            w.push_back((uint8_t)(v >> 16));
            w.push_back((uint8_t)(v >> 24));
        };
        auto w16 = [&](uint16_t v){
            w.push_back((uint8_t)(v));
            w.push_back((uint8_t)(v >> 8));
        };
        auto str = [&](const char* s){
            w.push_back((uint8_t)s[0]);
            w.push_back((uint8_t)s[1]);
            w.push_back((uint8_t)s[2]);
            w.push_back((uint8_t)s[3]);
        };
        str("RIFF"); w32(fileSize); str("WAVE");
        str("fmt "); w32(16); w16(1); w16(1);
        w32(SR); w32(byteRate); w16(2); w16(16);
        str("data"); w32(dataSize);
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(samples.data());
        w.insert(w.end(), raw, raw + dataSize);
        return w;
    }

    // Helper: write a sequence of ChipNotes as square waves
    void writeNotes(std::vector<int16_t>& buf,
                    const ChipNote* notes, int count,
                    float beat, float vol,
                    float duty = 0.5f,
                    float atkR = 0.005f, float relR = 0.15f)
    {
        for (int i = 0; i < count; i++)
            squareNote(buf, notes[i].freq, vol,
                       notes[i].beat * beat,
                       notes[i].dur  * beat * 0.85f,
                       duty, atkR, relR);
    }

    // Helper: write a sequence of ChipNotes as triangle waves
    void writeTriNotes(std::vector<int16_t>& buf,
                       const ChipNote* notes, int count,
                       float beat, float vol,
                       float atkR = 0.005f, float relR = 0.2f)
    {
        for (int i = 0; i < count; i++)
            triNote(buf, notes[i].freq, vol,
                    notes[i].beat * beat,
                    notes[i].dur  * beat * 0.8f,
                    atkR, relR);
    }

    // ── SOUND DESIGNS ─────────────────────────────────────────
    void generateAll()
    {
        // ── MOVE — tiny rising blip ───────────────────────────
        {
            auto b = makeBuffer(0.06f);
            squareSweep(b, 300.f, 500.f, 0.18f, 0.f, 0.06f);
            load(MOVE, b);
        }

        // ── WALL_HIT — noise + sweep down ────────────────────
        {
            auto b = makeBuffer(0.08f);
            noiseBurst(b, 0.5f, 0.f, 0.04f);
            squareSweep(b, 200.f, 80.f, 0.3f, 0.f, 0.08f);
            load(WALL_HIT, b);
        }

        // ── AI_STEP — quiet low blip ──────────────────────────
        {
            auto b = makeBuffer(0.04f);
            squareSweep(b, 120.f, 90.f, 0.08f, 0.f, 0.04f);
            load(AI_STEP, b);
        }

        // ── CHASE_ALERT — 3-note descending danger sting ─────
        {
            auto b = makeBuffer(0.35f);
            squareNote(b, 880.f, 0.45f, 0.00f, 0.10f, 0.5f, 0.01f, 0.1f);
            squareNote(b, 660.f, 0.45f, 0.12f, 0.10f, 0.5f, 0.01f, 0.1f);
            squareNote(b, 440.f, 0.50f, 0.24f, 0.11f, 0.5f, 0.01f, 0.3f);
            load(CHASE_ALERT, b);
        }

        // ── WIN — ascending scale + final chord ──────────────
        {
            auto b = makeBuffer(1.6f);

            // Ascending C major scale (square)
            float runFreqs[8] = {261.6f,293.7f,329.6f,349.2f,
                                  392.f, 440.f, 493.9f, 523.f};
            for (int i = 0; i < 8; i++)
                squareNote(b, runFreqs[i], 0.40f,
                           i * 0.07f, 0.060f, 0.5f, 0.005f, 0.2f);

            // Final chord — C major (triangle)
            triNote(b, 261.6f, 0.30f, 0.70f, 0.80f, 0.02f, 0.25f);
            triNote(b, 329.6f, 0.25f, 0.70f, 0.80f, 0.02f, 0.25f);
            triNote(b, 392.0f, 0.22f, 0.70f, 0.80f, 0.02f, 0.25f);
            triNote(b, 523.0f, 0.20f, 0.70f, 0.80f, 0.02f, 0.25f);
            squareNote(b, 130.8f, 0.28f, 0.70f, 0.80f, 0.5f, 0.02f, 0.3f);

            load(WIN, b);
        }

        // ── LOSE — descending scale + thud ───────────────────
        {
            auto b = makeBuffer(1.4f);

            float runFreqs[8] = {523.f,493.9f,440.f,392.f,
                                  349.2f,329.6f,293.7f,261.6f};
            for (int i = 0; i < 8; i++)
                squareNote(b, runFreqs[i], 0.38f,
                           i * 0.09f, 0.080f, 0.5f, 0.005f, 0.15f);

            squareNote(b, 130.8f, 0.45f, 0.85f, 0.50f, 0.5f, 0.01f, 0.6f);
            noiseBurst(b, 0.25f, 0.85f, 0.15f);

            load(LOSE, b);
        }

        // ── VIZ_TICK — tiny pip ───────────────────────────────
        {
            auto b = makeBuffer(0.03f);
            squareNote(b, 1000.f, 0.15f, 0.f, 0.03f, 0.5f, 0.001f, 0.5f);
            load(VIZ_TICK, b);
        }

        // ── START_MUSIC — NES-style loop, 4 bars at 140 BPM ──
        {
            float bpm   = 140.f;
            float beat  = 60.f / bpm;
            float bar   = beat * 4.f;
            float total = bar  * 4.f;
            auto b = makeBuffer(total);

            // Melody
            ChipNote melody[15] = {
                {523.f,0.f,.9f},{659.f,1.f,.9f},{784.f,2.f,.9f},{659.f,3.f,.9f},
                {523.f,4.f,.9f},{659.f,5.f,.9f},{784.f,6.f,.9f},{880.f,7.f,.9f},
                {784.f,8.f,.9f},{659.f,9.f,.9f},{523.f,10.f,.9f},{440.f,11.f,.9f},
                {392.f,12.f,.9f},{440.f,13.f,.9f},{523.f,14.f,1.9f}
            };
            writeNotes(b, melody, 15, beat, 0.32f);

            // Counter-melody (triangle)
            ChipNote counter[16] = {
                {329.6f,0.5f,.9f},{392.f,1.5f,.9f},{440.f,2.5f,.9f},{392.f,3.5f,.9f},
                {329.6f,4.5f,.9f},{392.f,5.5f,.9f},{440.f,6.5f,.9f},{523.f,7.5f,.9f},
                {440.f,8.5f,.9f},{392.f,9.5f,.9f},{329.6f,10.5f,.9f},{293.7f,11.5f,.9f},
                {261.6f,12.5f,.9f},{293.7f,13.5f,.9f},{329.6f,14.5f,.9f},{261.6f,15.5f,.9f}
            };
            writeTriNotes(b, counter, 16, beat, 0.18f);

            // Bass line (triangle, 16 beats)
            float bassFreqs[16] = {
                130.8f,130.8f,130.8f,130.8f,
                130.8f,130.8f,130.8f,130.8f,
                130.8f,130.8f,130.8f,130.8f,
                98.0f, 98.0f, 130.8f,130.8f
            };
            for (int i = 0; i < 16; i++)
                triNote(b, bassFreqs[i], 0.25f,
                        i * beat, beat * 0.7f, 0.01f, 0.3f);

            // Drums
            for (int bar2 = 0; bar2 < 4; bar2++)
            {
                float bo = (float)bar2 * bar;
                noiseBurst(b, 0.18f, bo + 0.0f * beat, 0.04f);
                noiseBurst(b, 0.18f, bo + 2.0f * beat, 0.04f);
                noiseBurst(b, 0.08f, bo + 1.0f * beat, 0.02f);
                noiseBurst(b, 0.08f, bo + 3.0f * beat, 0.02f);
                for (int h = 0; h < 8; h++)
                    noiseBurst(b, 0.04f, bo + (float)h * beat * 0.5f, 0.015f);
            }

            load(START_MUSIC, b);
        }

        // ── WIN_MUSIC — bright fast loop, 2 bars 160 BPM ─────
        {
            float bpm   = 160.f;
            float beat  = 60.f / bpm;
            float total = beat * 8.f;
            auto b = makeBuffer(total);

            ChipNote winMel[7] = {
                {1047.f,0.f,.9f},{1319.f,1.f,.9f},{1568.f,2.f,.9f},{1047.f,3.f,.9f},
                {1319.f,4.f,.9f},{1568.f,5.f,.9f},{2093.f,6.f,1.9f}
            };
            writeNotes(b, winMel, 7, beat, 0.28f);

            ChipNote winHarm[7] = {
                {784.f,0.f,.9f},{880.f,1.f,.9f},{1047.f,2.f,.9f},{784.f,3.f,.9f},
                {880.f,4.f,.9f},{1047.f,5.f,.9f},{1319.f,6.f,1.9f}
            };
            writeTriNotes(b, winHarm, 7, beat, 0.20f);

            float wb[8] = {261.6f,261.6f,261.6f,261.6f,261.6f,261.6f,261.6f,261.6f};
            for (int i = 0; i < 8; i++)
                triNote(b, wb[i], 0.22f, (float)i * beat, beat * 0.6f, 0.01f, 0.3f);

            load(WIN_MUSIC, b);
        }

        // ── LOSE_MUSIC — slow minor loop, 2 bars 80 BPM ──────
        {
            float bpm   = 80.f;
            float beat  = 60.f / bpm;
            float total = beat * 8.f;
            auto b = makeBuffer(total);

            ChipNote loseMel[4] = {
                {440.f,0.f,1.9f},{392.f,2.f,1.9f},
                {349.2f,4.f,1.9f},{329.6f,6.f,1.9f}
            };
            writeNotes(b, loseMel, 4, beat, 0.22f);

            triNote(b, 110.f,  0.20f, 0.f,          total,       0.05f, 0.3f);
            triNote(b, 87.3f,  0.15f, total * 0.5f, total * 0.5f,0.05f, 0.3f);

            ChipNote loseHigh[4] = {
                {880.f,0.5f,.9f},{784.f,2.5f,.9f},
                {698.5f,4.5f,.9f},{659.f,6.5f,.9f}
            };
            writeTriNotes(b, loseHigh, 4, beat, 0.10f);

            for (int i = 0; i < 4; i++)
                noiseBurst(b, 0.12f, (float)i * beat * 2.f, 0.06f);

            load(LOSE_MUSIC, b);
        }
    }
};
