#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

#include "Chunks.hpp"
#include "SkyBox.hpp"
#include "helpers.hpp"

class GameWorld {
public:
  GameWorld() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1023);
    seed = dist(rng);
    pprint(seed);
    setUpBlocks();
  }

  void UpdateUbo(SDL_Event event) { camera.UpdateUbo(event); }

  UniformBufferObject &getUbo() { return camera.ubo; }

  void setUpBlocks() {

    indices.clear();
    vertices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    skybox.plotSkyBox();

    for (int i = -RENDER_SIZE + current_block[0]; i <= RENDER_SIZE +
    current_block[0]; i++) {
      std::vector<std::vector<Grassland>> tempLayer;
      for (int j = -RENDER_SIZE + current_block[1]; j <= RENDER_SIZE +
      current_block[1]; j++) {
        std::vector<Grassland> tempvec;
        for (int k = 0; k < 2; k++) {
          Grassland grass(seed, i, j, k);
          grass.loadCubes();
          grass.Fill();
          grass.FillChunk(i, j, k, uniqueVertices, vertices, indices);
          tempvec.push_back(grass);
        }
        tempLayer.push_back(tempvec);
      }
      grassland.push_back(tempLayer);
    }

    // pprint("indices");
    // pprint(indices.size());
    // pprint("vertices");
    // pprint(vertices.size());
  }

  const std::vector<uint32_t> &getIndices() { return indices; }

  const std::vector<Vertex> &getVertices() { return vertices; }

  const std::vector<Vertex> &getskyboxVertices() {

    for (int i = 0; i < skybox.skyBoxVertices.size(); i++) {
      skybox.skyBoxVertices[i].pos += camera.camera_location - camera.delta_pos;
    }
    camera.delta_pos = glm::vec3(camera.camera_location);
    return skybox.skyBoxVertices;
  }

  bool modelChanged() {

    if (current_block[0] != camera.block_info[0] ||
        current_block[1] != camera.block_info[1]) {
      current_block = glm::vec3(camera.block_info);
      // pprint("block info");
      // std::cout<<camera.block_info[0]<<" "<<camera.block_info[1]<<"
      // "<<camera.block_info[2]<<" "<<std::endl; pprint("current Block");
      // std::cout<<current_block[0]<<" "<<current_block[1]<<"
      // "<<current_block[2]<<" "<<std::endl;
      return true;
    } else
      return false;
  }

private:
  std::vector<Vertex> vertices;

  std::vector<uint32_t> indices;

  std::vector<std::vector<std::vector<Grassland>>> grassland;

  glm::vec3 current_block = glm::vec3(0, 0, 0);

  SkyBox skybox;

  Camera camera;

  int time = 0;

  int seed;
};

#endif
