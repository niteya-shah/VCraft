#ifndef VCraft_ubo
#define VCraft_ubo

struct UniformBufferObject{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};
#endif
