#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

#include "Chunks.hpp"

class GameWorld {
public:
  GameWorld() { setUpBlocks(); }

  void UpdateUbo(SDL_Event event) {
    camera.UpdateUbo(event);
  }

  UniformBufferObject &getUbo() { return camera.ubo; }

  void setUpBlocks() {
    indices.clear();
    vertices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    grass.loadCubes();
    grass.plotSkyBox();
    int i = 0, j = 0, k = 0;
    for (int i = -4; i < 5; i++)
      for (int j = -4; j < 5; j++)
        for (int k = 0; k < 2; k++) {
          grass.Fill();
          grass.FillChunk(i, j, k, uniqueVertices, vertices, indices);
        }
    pprint("indices");
    pprint(indices.size());
    pprint("vertices");
    pprint(vertices.size());
  }

  const std::vector<uint32_t> &getIndices() { return indices; }

  const std::vector<Vertex> &getVertices() { return vertices; }

  const std::vector<Vertex> &getskyboxVertices() {
    for(int i = 0; i < grass.skyBoxVertices.size(); i++){
      grass.skyBoxVertices[i].pos += camera.camera_location - camera.delta_pos;
    }
    camera.delta_pos = glm::vec3(camera.camera_location);
    return grass.skyBoxVertices;
  }

private:
  std::vector<Vertex> vertices;

  std::vector<uint32_t> indices;

  Grassland grass;

  Camera camera;

  int time = 0;
};

#endif
