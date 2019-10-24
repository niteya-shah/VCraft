#ifndef VCraft_SkyBox_def
#define VCraft_SkyBox_def

#include "Cubes.hpp"

struct SkyBox {
  void plotSkyBox() {

    Cube c(SKYBOX_PATH, air);
    Vertex vertex = {};
    int i = 100;
    for (const auto &index : c.shapes[0].mesh.indices) {
      vertex.pos = {c.attrib.vertices[3 * index.vertex_index + 0] * i,
                    c.attrib.vertices[3 * index.vertex_index + 1] * i,
                    c.attrib.vertices[3 * index.vertex_index + 2] * i};

      vertex.texCoord = {c.attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0f -
                             c.attrib.texcoords[2 * index.texcoord_index + 1]};
      vertex.color = {0.0f, 0.0f, 0.0f};

      skyBoxVertices.push_back(vertex);
    }
    std::reverse(skyBoxVertices.begin(), skyBoxVertices.end());
  }

  std::vector<Vertex> skyBoxVertices;
};

#endif
