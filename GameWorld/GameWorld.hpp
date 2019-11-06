#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

#include "Chunks.hpp"
#include "SkyBox.hpp"
#include "helpers.hpp"

#define num_blocks (2 * RENDER_SIZE + 1) * (2 * RENDER_SIZE + 1) * 2

class GameWorld {
public:
  GameWorld() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1023);
    seed = dist(rng);
    Chunk::loadCubes();
    skybox.plotSkyBox();
    setUpBlocks();
    omp_set_num_threads(6);
  }

  void UpdateUbo(SDL_Event event) { camera.UpdateUbo(event); }

  UniformBufferObject &getUbo() { return camera.ubo; }

  void setUpBlocks() {

    indices.clear();
    vertices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    std::cout<<camera.block_info<<"  "<<current_block<<std::endl;
#pragma omp parallel
   {
      std::vector<Vertex> vertices_temp;

#pragma omp for schedule(auto)
      for (int index = 0; index < num_blocks; index++) {
        glm::vec3 loc = con1Dto3D(index, shape);
        loc -= glm::vec3(RENDER_SIZE - current_block[0] + 1, RENDER_SIZE - current_block[1] + 1, 0);
        chunks[index] = std::make_unique<Grassland>(
            Grassland(seed, loc[0], loc[1], loc[2]));
        chunks[index]->Fill();
        chunks[index]->FillChunk(loc[0], loc[1], loc[2], vertices_temp);
      }
#pragma omp critical
      for (int count = 0; count < vertices_temp.size(); count++) {
        if (uniqueVertices.count(vertices_temp[count]) == 0) {
          uniqueVertices[vertices_temp[count]] =
              static_cast<uint32_t>(vertices.size());
          vertices.push_back(vertices_temp[count]);
        }
        indices.push_back(uniqueVertices[vertices_temp[count]]);
      }
    }
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

  std::unique_ptr<Chunk> chunks[num_blocks];

  glm::vec3 current_block = glm::vec3(0, 0, 0);

  glm::vec3 shape = glm::vec3((2 * RENDER_SIZE + 1), (2 * RENDER_SIZE + 1), 2);

  SkyBox skybox;

  Camera camera;

  int time = 0;

  int seed;
};

#endif
