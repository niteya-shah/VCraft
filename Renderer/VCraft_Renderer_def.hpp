#ifndef VCraft_Renderer_def
#define VCraft_Renderer_def

#ifdef NDEBUG
std::ostream &operator<<(std::ostream &out, const glm::vec3 vector) {
  out << vector[0] << "  " << vector[1] << "  " << vector[2];
  return out;
}
std::ostream &operator<<(std::ostream &out, const glm::vec2 vector) {
  out << vector[0] << "  " << vector[1];
  return out;
}
std::ostream &operator<<(std::ostream &out, std::vector<size_t> vector) {
  out << vector[0] << "  " << vector[1] << "  " << vector[2] << std::endl;
  return out;
}
#endif

#include "vulkan_utils.hpp"
#include "fileIO.hpp"
#include "VCraft_Renderer_impl.hpp"
#include "swapChain.hpp"
#include <../Camera/ubo.hpp>

#endif
