// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.h>
//#include <memory> use shared_ptr
// USE TRANFER QUEUE
// Read Image barrier

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <optional>
#include <set>
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <tiny_obj_loader.h>
#include <typeinfo>
#include <vector>

#ifndef Vkx_defination
#define Vkx_defination

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_STAGING_BUFFERS = 10;
const int TIMEOUT = 10;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::string NAME = "Vulkan";
const std::string MODEL_PATH = "/D/git/vulkan/VkX/models/grassBox.obj";
const std::string TEXTURE_PATH = "/D/git/vulkan/VkX/textures/voxelTerrain.jpg";

#ifndef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#ifdef NDEBUG
std::ostream& operator<<(std::ostream &out,const glm::vec3 vector){
  out<<vector[0] << "  " << vector[1] << "  " << vector[2];
  return out;
}
#endif

#include "Ubo.hpp"
#include "vulkan_utils.hpp"
#include "fileIO.hpp"
#include "Vkx_impl.hpp"
#include "swapChain.hpp"

#endif
