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

#ifndef VCraft_defination
#define VCraft_defination

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 3;
const int MAX_SINGLE_COMMAND_BUFFERS = 5;
const int MAX_STAGING_BUFFERS = 10;
const int TIMEOUT = 10;
const int BLOCK_SIZE = 16;
const int RENDER_CHUNKS = 3;
const int ALLOC_SIZE =
    BLOCK_SIZE * BLOCK_SIZE * BLOCK_SIZE * RENDER_CHUNKS * RENDER_CHUNKS * 4;
const int CUBE_ALLOC_VERTEX = 92;
const int CUBE_ALLOC_INDEX = 36;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::string NAME = "Vulkan";
const std::string MODEL_PATH = "/D/git/vulkan/VCraft/models/grassBox.obj";
const std::string TEXTURE_PATH =
    "/D/git/vulkan/VCraft/textures/voxelTerrain.jpg";

#ifndef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "Camera/Camera.hpp"
#include "Renderer/VCraft_Renderer_def.hpp"
#include "GameWorld/GameWorld.hpp"

void mainLoop() {

  SDL_Event event;
  bool quit = false;
  Camera camera;
  GameWorld world;
  VCraftRenderer renderer;
  renderer.Setup();
  world.loadModel();

  while (!quit) {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      quit = true;
    else if (event.window.type == SDL_WINDOWEVENT_SIZE_CHANGED)
      renderer.SetFrameBuffer() = true;
    else if (event.type == SDL_KEYDOWN)
      camera.UpdateUbo(event);
    renderer.drawFrame(camera.ubo, world.getVertices(), world.getIndices());
    // quit = true;
    //world.setUpBlocks();
  }

  vkDeviceWaitIdle(renderer.GetDevice());
}

#endif
