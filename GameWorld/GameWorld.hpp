#ifndef VCraft_gameWorld_impl
#define VCraft_gameWorld_impl

#define n 48
#define w 5

class GameWorld {

public:
  void loadModel() {
    std::string warn, err;
    // TODO : Material unused
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          MODEL_PATH.c_str())) {
      throw std::runtime_error(warn + err);
    }
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
      // Remove later
      // int pos = (std::abs(i) % 2) * 2 + (std::abs(j) % 2);
      // if(pos < 3 && time == 0)
      //   vertex.color[pos] = 1;
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
    int pos = 0;
    for (int i = -n; i < n; i++) {
      for (int j = -n; j < n; j++) {
        int pos = (std::abs(i) % 2) * 2 + (std::abs(j) % 2);
        auto shape = shapes[pos];
        plotShape(shape, attrib, uniqueVertices, i, j, -1);
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
  tinyobj::attrib_t attrib;

  std::vector<tinyobj::shape_t> shapes;

  std::vector<tinyobj::material_t> materials;

  std::vector<Vertex> vertices;

  std::vector<uint32_t> indices;

  int time = 0;
};

#endif
