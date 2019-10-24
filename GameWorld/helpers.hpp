#ifndef VCraft_gameWorld_helpers
#define VCraft_gameWorld_helpers

glm::vec3 con1Dto3D(int index, glm::vec3 shape){
  glm::vec3 vector;
  vector[0] = index%static_cast<int>(shape[0]);
  vector[1] = static_cast<int>((index/shape[0]))%static_cast<int>(shape[1]);
  vector[2] = index/(shape[0] * shape[1]);
  return vector;
}

int con3Dto1D(glm::vec3 vector, glm::vec3 shape)
{
  return vector[0] + (vector[1] * shape[0]) + (vector[2] * shape[0] * shape[1]);
}

#endif
