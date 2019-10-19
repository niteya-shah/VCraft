#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

#define n 16

#include "Chunks.hpp"
#include "Cubes.hpp"

class GameWorld {
public:
  GameWorld() {
    for (int i = 0; i < MODEL_PATH.size(); i++) {
      cubes.push_back(Cube(MODEL_PATH[i], static_cast<cubeEnum>(i + 1)));
    }
    grass.Fill();
    setUpBlocks();
  }

  void plotShape(tinyobj::shape_t &shape, tinyobj::attrib_t &attrib,
                 std::unordered_map<Vertex, uint32_t> &uniqueVertices, int i,
                 int j, int k) {
    Vertex vertex = {};
    for (const auto &index : shape.mesh.indices) {
      vertex.pos = {attrib.vertices[3 * index.vertex_index + 0] + 2 * i,
                    attrib.vertices[3 * index.vertex_index + 1] + 2 * j,
                    attrib.vertices[3 * index.vertex_index + 2] - 1 + (2 * k)};

      vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
      vertex.color = {0.0f, 0.0f, 0.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }

  void setUpBlocks() {
    indices.clear();
    vertices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k > -n; k--) {
          int pos = (std::abs(i) % 2) * 2 + (std::abs(j) % 2);
          boost::array<block_index, 4> idxCube = {{i, j, k + n - 1, 0}};
          boost::array<block_index, 4> idxEmpty = {{i, j, k + n - 1, 1}};
          grass.removeEmpty(idxCube);
          if (grass(idxCube) != 2 && grass(idxEmpty) == 1)
            plotShape(cubes[grass(idxCube)].shapes[pos],
                      cubes[grass(idxCube)].attrib, uniqueVertices, i, j, k);
        }
      }
    }
    pprint("indices");
    pprint(indices.size());
    pprint("vertices");
    pprint(vertices.size());
  }

  const std::vector<uint32_t> &getIndices() { return indices; }

  const std::vector<Vertex> &getVertices() { return vertices; }

private:
  std::vector<Vertex> vertices;

  std::vector<uint32_t> indices;

  std::vector<Cube> cubes;

  int time = 0;

  Grassland grass;
};

#endif
