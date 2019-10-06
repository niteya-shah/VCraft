#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

class GameWorld {

public:
  void loadModel() {
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          MODEL_PATH.c_str())) {
      throw std::runtime_error(warn + err);
    }
    setUpBlocks();
  }

  void setUpBlocks() {

    indices.clear();
    vertices.clear();
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    auto shape = shapes[0];
    for (int i = -3 - time; i < 3 + time; i++) {
      for (int j = -3 - time; j < 3 + time; j++) {
        for (const auto &index : shape.mesh.indices) {
          Vertex vertex = {};
          vertex.pos = {attrib.vertices[3 * index.vertex_index + 0] + 2 * i,
                        attrib.vertices[3 * index.vertex_index + 1] + 2 * j,
                        attrib.vertices[3 * index.vertex_index + 2] - 3};

          vertex.texCoord = {
              attrib.texcoords[2 * index.texcoord_index + 0],
              1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
          vertex.color = {0.0f, 0.0f, 0.0f};
          if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
          }
          indices.push_back(uniqueVertices[vertex]);
        }
      }
    }
    // std::cout<<indices.size()<<std::endl;
    shape = shapes[1];
    for (int i = -3 - time; i < 3 + time; i++) {
      for (int j = -3 - time; j < 3 + time; j++) {
        for (int k = 0; k < 1; k++) {
          for (const auto &index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {attrib.vertices[3 * index.vertex_index + 0] + 2 * i,
                          attrib.vertices[3 * index.vertex_index + 1] + 2 * j,
                          attrib.vertices[3 * index.vertex_index + 2] - 5 -
                              2 * k};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.color = {0.0f, 0.0f, 0.0f};
            if (uniqueVertices.count(vertex) == 0) {
              uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
              vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
          }
        }
      }
    }
  }

  const std::vector<uint32_t>& getIndices(){
    return indices;
  }

  const std::vector<Vertex>& getVertices(){
    return vertices;
  }


private:
  tinyobj::attrib_t attrib;

  std::vector<tinyobj::shape_t> shapes;

  std::vector<tinyobj::material_t> materials;

  std::vector<Vertex> vertices;

  std::vector<uint32_t> indices;

  int time = 0;
};

#endif
