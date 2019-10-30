#ifndef VCraft_chunk_impl
#define VCraft_chunk_impl

#include "Chunks.hpp"

void Chunk::removeEmpty(const boost::array<block_index, 4> &idx) {
  boost::array<block_index, 4> idxTemp = {{0, 0, 0, 1}};
  for (int i = 0; i < 3; i++)
    if (idx[i] == 0 || idx[i] == BLOCK_SIZE - 1) {
      idxTemp[0] = idx[0];
      idxTemp[1] = idx[1];
      idxTemp[2] = idx[2];
      block(idxTemp) = 1;
      return;
    }

  idxTemp = {{0, 0, 0, 0}};
  for (int i = 0; i < 3; i++) {
    idxTemp = boost::array<block_index, 4>(idx);
    for (int j = -1; j <= -1; j++) {
      idxTemp[i] = idx[i] + j;
      if (block(idxTemp) == 2) {
        idxTemp = boost::array<block_index, 4>(idx);
        idxTemp[3] = 1;
        block(idxTemp) = 1;
        return;
      }
    }
  }
}

void Chunk::loadCubes() {
  for (int i = 0; i < MODEL_PATH.size(); i++) {
    Chunk::cubes.push_back(Cube(MODEL_PATH[i], static_cast<cubeEnum>(i + 1)));
  }
}

void Chunk::plotShape(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib,
                      std::vector<Vertex> &vertices, int i, int j, int k) {
  Vertex vertex = {};
  for (const auto &index : shape.mesh.indices) {
    vertex.pos = {attrib.vertices[3 * index.vertex_index + 0] + 2 * i,
                  attrib.vertices[3 * index.vertex_index + 1] + 2 * j,
                  attrib.vertices[3 * index.vertex_index + 2] - 1 + (2 * k)};

    vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                       1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
    vertex.color = {0.0f, 0.0f, 0.0f};

    vertices.push_back(vertex);
  }
}

void Chunk::FillChunk(int blockIdX, int blockIdY, int blockIdZ,
                      std::vector<Vertex> &vertices) {
  for (int i = 0; i < BLOCK_SIZE; i++) {
    for (int j = 0; j < BLOCK_SIZE; j++) {
      for (int k = 0; k > -BLOCK_SIZE; k--) {
        int pos = (std::abs(i) % 2) * 2 + (std::abs(j) % 2);
        boost::array<block_index, 4> idxCube = {{i, j, k + BLOCK_SIZE - 1, 0}};
        boost::array<block_index, 4> idxEmpty = {{i, j, k + BLOCK_SIZE - 1, 1}};
        this->removeEmpty(idxCube);
        if (block(idxCube) != 2 && block(idxEmpty) == 1)
          plotShape(Chunk::cubes[block(idxCube)].shapes[pos],
                    Chunk::cubes[block(idxCube)].attrib, vertices,
                    i + blockIdX * BLOCK_SIZE, j + blockIdY * BLOCK_SIZE,
                    k + blockIdZ * BLOCK_SIZE);
      }
    }
  }
}
#endif
