#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

class PerlinNoise {
  public:
    explicit PerlinNoise(unsigned int seed = 1337) {
        reseed(seed);
    }

    void reseed(unsigned int seed) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);

        p.insert(p.end(), p.begin(), p.end());
    }

    float noise2D(float x, float y) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);

        float u = fade(x);
        float v = fade(y);

        int A  = p[X] + Y;
        int AA = p[A];
        int AB = p[A + 1];
        int B  = p[X + 1] + Y;
        int BA = p[B];
        int BB = p[B + 1];

        float res = lerp(v,
            lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1.0f, y)),
            lerp(u, grad(p[AB], x, y - 1.0f), grad(p[BB], x - 1.0f, y - 1.0f))
        );

        return res; // Value in ~[-1, 1]
    }

    float fbm2D(float x, float y, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            total += noise2D(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return (maxValue > 0.0f) ? (total / maxValue) : 0.0f;
    }

  private:
    std::vector<int> p;

    static inline float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    static inline float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    static inline float grad(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
};

