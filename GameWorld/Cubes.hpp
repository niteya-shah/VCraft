#ifndef VCraft_cube_impl
#define VCraft_cube_impl

enum cubeEnum { air, grass, mud };

struct Cube {

  Cube(std::string path, cubeEnum cub){
    model_path = path;
    c = cub;
    loadModel();
  }

  void loadModel() {
    std::string warn, err;

    // TODO : Material unused
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          model_path.c_str())) {
      throw std::runtime_error(warn + err);
    }
  }

  tinyobj::attrib_t attrib;

  std::vector<tinyobj::shape_t> shapes;

  std::vector<tinyobj::material_t> materials;

  std::string model_path;

  cubeEnum c;
};

#endif
