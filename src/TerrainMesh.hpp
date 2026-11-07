#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "PerlinNoise.hpp"

#include <vector>

struct TerrainVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    float height;
};

class TerrainMesh {
  public:
    TerrainMesh() = default;

    ~TerrainMesh() {
        cleanup();
    }

    void init() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // Position: location 0 (vec3)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, position));

        // Normal: location 1 (vec3)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, normal));

        // TexCoord: location 2 (vec2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, texCoord));

        // Height: location 3 (float)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, height));

        glBindVertexArray(0);
    }

    void generate(const PerlinNoise& noise, int resolution, float worldSize, float amplitude, float frequency, int octaves) {
        m_indexCount = (resolution - 1) * (resolution - 1) * 6;

        std::vector<TerrainVertex> vertices(resolution * resolution);
        std::vector<unsigned int> indices(m_indexCount);

        float step = worldSize / static_cast<float>(resolution - 1);
        float halfSize = worldSize * 0.5f;
        const float eps = 0.15f;

        // Generate vertices
        for (int z = 0; z < resolution; ++z) {
            for (int x = 0; x < resolution; ++x) {
                float wx = static_cast<float>(x) * step - halfSize;
                float wz = static_cast<float>(z) * step - halfSize;

                // Height using fBm noise
                float rawNoise = noise.fbm2D(wx * frequency, wz * frequency, octaves);
                float wy = rawNoise * amplitude;

                // Central difference finite difference for smooth analytical normal
                float hL = noise.fbm2D((wx - eps) * frequency, wz * frequency, octaves) * amplitude;
                float hR = noise.fbm2D((wx + eps) * frequency, wz * frequency, octaves) * amplitude;
                float hD = noise.fbm2D(wx * frequency, (wz - eps) * frequency, octaves) * amplitude;
                float hU = noise.fbm2D(wx * frequency, (wz + eps) * frequency, octaves) * amplitude;

                glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f * eps, hD - hU));

                int vIdx = z * resolution + x;
                vertices[vIdx].position = glm::vec3(wx, wy, wz);
                vertices[vIdx].normal = normal;
                vertices[vIdx].texCoord = glm::vec2(static_cast<float>(x) / (resolution - 1),
                                                    static_cast<float>(z) / (resolution - 1));
                vertices[vIdx].height = wy;
            }
        }

        // Generate indices
        int idx = 0;
        for (int z = 0; z < resolution - 1; ++z) {
            for (int x = 0; x < resolution - 1; ++x) {
                unsigned int topLeft     = z * resolution + x;
                unsigned int topRight    = topLeft + 1;
                unsigned int bottomLeft  = (z + 1) * resolution + x;
                unsigned int bottomRight = bottomLeft + 1;

                // First triangle
                indices[idx++] = topLeft;
                indices[idx++] = bottomLeft;
                indices[idx++] = topRight;

                // Second triangle
                indices[idx++] = topRight;
                indices[idx++] = bottomLeft;
                indices[idx++] = bottomRight;
            }
        }

        // Upload to GPU buffers
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

  private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    int m_indexCount = 0;

    void cleanup() {
        if (EBO) glDeleteBuffers(1, &EBO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        VAO = VBO = EBO = 0;
    }
};

