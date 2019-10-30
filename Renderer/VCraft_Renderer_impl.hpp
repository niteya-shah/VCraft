#ifndef VCraft_Renderer_impl
#define VCraft_Renderer_impl

class VCraftRenderer {
public:
  void Setup(const std::vector<uint32_t> &indices,
             const std::vector<Vertex> &vertices,
             const std::vector<Vertex> &skyboxVertices) {
    initWindow();
    initVulkan(indices, vertices, skyboxVertices);
  }

  ~VCraftRenderer() { cleanup(); }

  void drawFrame(UniformBufferObject &ubo, const std::vector<Vertex> &vertices,
                 const std::vector<uint32_t> &indices,
                 const std::vector<Vertex> &skyboxVertices) {

    ERR_GUARD_VULKAN(vkWaitForFences(device, 1, &inFlightFences[currentFrame],
                                     VK_TRUE,
                                     std::numeric_limits<uint64_t>::max()));

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        device, swapChain, std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain(indices);
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image");
    }


    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ERR_GUARD_VULKAN(vkResetFences(device, 1, &inFlightFences[currentFrame]));

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
      framebufferResized = false;
      recreateSwapChain(indices);
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    if (modelChanged) {

      for(int i = 0;i < 3;i++){
      updateVertexBuffer(i, vertices);

      updateIndexBuffer(i, indices);

      modelChanged = false;
    }
    }
    updateUniformBuffer(imageIndex, ubo);

    updateSkyBoxVertexBuffer(imageIndex, skyboxVertices);
  }

  bool &SetFrameBuffer() { return framebufferResized; }

  bool &SetModelChanged(){return modelChanged;}

  const VkDevice &GetDevice() { return device; }

private:
  void initVulkan(const std::vector<uint32_t> &indices,
                  const std::vector<Vertex> &vertices,
                  const std::vector<Vertex> &skyboxVertices) {

    createInstance();

    setupDebugMessenger();

    createSurface();

    pickPhysicalDevice();

    createLogicalDevice();

    createVMAAllocator();

    createSwapChain();

    createImageViews();

    createRenderPass();

    createDescriptorSetLayout();

    createGraphicsPipeline();

    createCommandPool();

    createSyncObjects();

    createColorResources();

    createDepthResources();

    fetchModel();

    createFramebuffers();

    createTextureImage();

    createTextureImageView();

    createTextureSampler();

    createVertexBuffer(vertices);

    createIndexBuffers(indices);

    createSkyboxVertexBuffer(skyboxVertices);

    createUniformBuffer();

    createDescriptorPool();

    createDescriptorSets();

    createCommandBuffers(indices);
  }

  void cleanup() {

    cleanupSwapChain();

    vkDestroySampler(device, textureSampler, nullptr);

    vkDestroyImageView(device, textureImageView, nullptr);

    vmaDestroyImage(allocator, textureImage, textureImageMemory);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    for (size_t imc = 0; imc < swapChainImages.size(); imc++) {

      vmaDestroyBuffer(allocator, vertexBuffer[imc], vertexBufferMemory[imc]);

      vmaDestroyBuffer(allocator, indexBuffer[imc], indexBufferMemory[imc]);

      vmaDestroyBuffer(allocator, skyboxVertexBuffer[imc],
                       skyBoxVertexBufferMemory[imc]);
    }

    for (size_t syncObject = 0; syncObject < MAX_FRAMES_IN_FLIGHT;
         ++syncObject) {
      vkDestroySemaphore(device, renderFinishedSemaphores[syncObject], nullptr);

      vkDestroySemaphore(device, imageAvailableSemaphores[syncObject], nullptr);

      vkDestroyFence(device, inFlightFences[syncObject], nullptr);
    }
    for (size_t syncObject = 0; syncObject < MAX_SINGLE_COMMAND_BUFFERS;
         ++syncObject)
      vkDestroyFence(device, singleCommandBufferFences[syncObject], nullptr);

    for (size_t imageFence = 0; imageFence < swapChainImages.size();
         imageFence++) {
      for (size_t syncObject = 0; syncObject < MAX_STAGING_BUFFERS;
           syncObject++) {
        vkDestroyFence(device, stagingFences[imageFence][syncObject], nullptr);
      }
    }
    vmaDestroyAllocator(allocator);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    SDL_DestroyWindow(window);

    SDL_Quit();
  }

  void createVMAAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    ERR_GUARD_VULKAN(vmaCreateAllocator(&allocatorInfo, &allocator));
  }

  void fetchModel() {}

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    std::cerr << "validation Layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices queueIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (queueFamily.queueCount > 0 &&
          (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
        queueIndices.graphicsFamily = i;
      }
      if (queueFamily.queueCount > 0 && presentSupport) {
        queueIndices.presentFamily = i;
      }
      if (queueIndices.isComplete()) {
        break;
      }
      i++;
    }
    return queueIndices;
  }

  bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices queueIndices = findQueueFamilies(device);

    bool extensionSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;

    if (extensionSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() &&
                          !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return queueIndices.isComplete() && extensionSupported &&
           swapChainAdequate && supportedFeatures.samplerAnisotropy;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
  }

  void initWindow() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
    window = SDL_CreateWindow(NAME.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                              SDL_WINDOW_VULKAN);

    SDL_SetWindowResizable(window, SDL_TRUE);
  }

  void createDepthResources() {

    VkFormat depthFormat = findDepthFormat();
    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
                depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY, depthImage, depthImageMemory);
    depthImageView =
        createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
  }

  bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
  }

  VkFormat findDepthFormat() {
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT,
                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                                VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
      if (tiling == VK_IMAGE_TILING_LINEAR &&
          (props.linearTilingFeatures & features) == features) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }
    throw std::runtime_error("unable to find supported format");
  }

  void createTextureSampler() {

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler");
    }
  }

  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags,
                              uint32_t mipLevels) {

    VkImageView imageView;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view");
    }

    return imageView;
  }

  void createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
                                       VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
  }

  void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);

    // mipLevels = static_cast<uint32_t>(
    //                 std::floor(std::log2(std::max(texWidth, texHeight)))) +
    //             1;

    if (!pixels) {
      throw std::runtime_error("failed to load texture image");
    }
    mipLevels = 1;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY, textureImage, textureImageMemory);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));

    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight,
                    mipLevels);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void createColorResources() {

    VkFormat colorFormat = swapChainImageFormat;
    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
                colorFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY, colorImage, colorImageMemory);
    colorImageView =
        createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
  }

  void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels) {

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat,
                                        &formatProperties);
    if (!(formatProperties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
      throw std::runtime_error(
          "texture image format does not support linear blitting");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &barrier);

      VkImageBlit blit = {};
      blit.srcOffsets[0] = {0, 0, 0};
      blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
      blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = 1;
      blit.dstOffsets[0] = {0, 0, 0};
      blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                            mipHeight > 1 ? mipHeight / 2 : 1, 1};
      blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = 1;

      vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                     VK_FILTER_LINEAR);

      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);

      if (mipWidth > 1)
        mipWidth /= 2;
      if (mipHeight > 1)
        mipHeight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
  }

  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkSampleCountFlagBits numSamples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usageBuffer,
                   VmaMemoryUsage usageAlloc, VkImage &image,
                   VmaAllocation &imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usageBuffer;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usageAlloc;

    ERR_GUARD_VULKAN(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image,
                                    &imageMemory, nullptr));
  }

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      if (hasStencilComponent(format)) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
    } else {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
  }

  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height) {

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
  }

  void createDescriptorPool() {

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount =
        static_cast<uint32_t>(swapChainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount =
        static_cast<uint32_t>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor pool");
    }
  }

  void createUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(swapChainImages.size());
    uniformBufferMemory.resize(swapChainImages.size());

    for (size_t ub = 0; ub < swapChainImages.size(); ub++) {
      createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers[ub],
                   uniformBufferMemory[ub]);
    }
  }

  void createDescriptorSetLayout() {

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> binding = {
        uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(binding.size());
    layoutInfo.pBindings = binding.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &descriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor set layout");
    }
  }

  void createIndexBuffers(const std::vector<uint32_t> &indices) {
    VkDeviceSize bufferSizeGPU = sizeof(uint32_t) * ALLOC_SIZE *
                                 CUBE_ALLOC_INDEX; // indices.size() * 100;
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, indices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    indexBuffer.resize(swapChainImages.size());
    indexBufferMemory.resize(swapChainImages.size());
    for (size_t imc = 0; imc < swapChainImages.size(); imc++) {
      createBuffer(
          bufferSizeGPU,
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
          VMA_MEMORY_USAGE_GPU_ONLY, indexBuffer[imc], indexBufferMemory[imc]);
      copyBuffer(stagingBuffer, indexBuffer[imc], bufferSize, imc);
    }

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usageBuffer,
                    VmaMemoryUsage usageAlloc, VkBuffer &buffer,
                    VmaAllocation &allocationMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageBuffer;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usageAlloc;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
                                     &buffer, &allocationMemory, nullptr));
  }

  void createVertexBuffer(const std::vector<Vertex> &vertices) {

    VkDeviceSize bufferSizeGPU = sizeof(Vertex) * ALLOC_SIZE *
                                 CUBE_ALLOC_VERTEX; // vertices.size() * 100;
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    vertexBuffer.resize(swapChainImages.size());
    vertexBufferMemory.resize(swapChainImages.size());
    for (size_t imc = 0; imc < swapChainImages.size(); imc++) {
      createBuffer(bufferSizeGPU,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                   VMA_MEMORY_USAGE_GPU_ONLY, vertexBuffer[imc],
                   vertexBufferMemory[imc]);

      copyBuffer(stagingBuffer, vertexBuffer[imc], bufferSize, imc);
    }

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void createSkyboxVertexBuffer(const std::vector<Vertex> &vertices) {

    VkDeviceSize bufferSizeGPU = sizeof(Vertex) * vertices.size();
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    skyboxVertexBuffer.resize(swapChainImages.size());
    skyBoxVertexBufferMemory.resize(swapChainImages.size());
    for (size_t imc = 0; imc < swapChainImages.size(); imc++) {
      createBuffer(bufferSizeGPU,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                   VMA_MEMORY_USAGE_GPU_ONLY, skyboxVertexBuffer[imc],
                   skyBoxVertexBufferMemory[imc]);

      copyBuffer(stagingBuffer, skyboxVertexBuffer[imc], bufferSize, imc);
    }

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    ERR_GUARD_VULKAN(
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }

  void endSingleTimeCommands(VkCommandBuffer &commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    ERR_GUARD_VULKAN(vkResetFences(
        device, 1, &singleCommandBufferFences[singleCommandBufferCurrent]));

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                      singleCommandBufferFences[singleCommandBufferCurrent]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to submit end command");
    }
    ERR_GUARD_VULKAN(vkWaitForFences(
        device, 1, &singleCommandBufferFences[singleCommandBufferCurrent],
        VK_TRUE, std::numeric_limits<uint64_t>::max()));

    singleCommandBufferCurrent =
        (singleCommandBufferCurrent + 1) % MAX_SINGLE_COMMAND_BUFFERS;

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }

  void endSingleTimeCommands(VkCommandBuffer &commandBuffer,
                             size_t imageIndex) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    ERR_GUARD_VULKAN(vkResetFences(
        device, 1,
        &stagingFences[imageIndex][stagingBufferCurrent[imageIndex]]));

    if (vkQueueSubmit(
            graphicsQueue, 1, &submitInfo,
            stagingFences[imageIndex][stagingBufferCurrent[imageIndex]]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to end single Time command Indexed");
    }

    ERR_GUARD_VULKAN(vkWaitForFences(
        device, 1, &stagingFences[imageIndex][stagingBufferCurrent[imageIndex]],
        VK_TRUE, std::numeric_limits<uint64_t>::max()));

    stagingBufferCurrent[imageIndex] =
        (stagingBufferCurrent[imageIndex] + 1) % MAX_STAGING_BUFFERS;

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
                  size_t imageIndex) {

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    vkCmdFillBuffer(commandBuffer, dstBuffer, 0, VK_WHOLE_SIZE, 0);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, imageIndex);
  }

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    singleCommandBufferFences.resize(MAX_SINGLE_COMMAND_BUFFERS);

    stagingFences.resize(swapChainImages.size());
    stagingBufferCurrent.resize(swapChainImages.size());

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo inFlightFenceInfo = {};
    inFlightFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    inFlightFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFenceCreateInfo singleCommandBufferFencesInfo = {};
    singleCommandBufferFencesInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    for (size_t syncObject = 0; syncObject < MAX_FRAMES_IN_FLIGHT;
         syncObject++) {
      if ((vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                             &imageAvailableSemaphores[syncObject]) !=
           VK_SUCCESS) ||
          (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                             &renderFinishedSemaphores[syncObject]) !=
           VK_SUCCESS) ||
          (vkCreateFence(device, &inFlightFenceInfo, nullptr,
                         &inFlightFences[syncObject]) != VK_SUCCESS)) {
        throw std::runtime_error(
            "failed to create synchronisation objects for a frame");
      }
    }

    for (size_t syncObject = 0; syncObject < MAX_SINGLE_COMMAND_BUFFERS;
         syncObject++) {
      if (vkCreateFence(device, &singleCommandBufferFencesInfo, nullptr,
                        &singleCommandBufferFences[syncObject]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronisation objects for "
                                 "a Single Command Buffer");
      }
    }

    for (size_t imageFence = 0; imageFence < swapChainImages.size();
         imageFence++) {
      stagingBufferCurrent[imageFence] = 0;
      stagingFences[imageFence].resize(MAX_STAGING_BUFFERS);
      for (size_t syncObject = 0; syncObject < MAX_STAGING_BUFFERS;
           syncObject++) {
        VkFenceCreateInfo stagingFencesInfo = {};
        stagingFencesInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if (vkCreateFence(device, &stagingFencesInfo, nullptr,
                          &stagingFences[imageFence][syncObject]) !=
            VK_SUCCESS) {
          throw std::runtime_error("cannot create Staging Fence");
        }
      }
    }
  }

  void cleanupSwapChain() {
    vkDestroyImageView(device, colorImageView, nullptr);

    vmaDestroyImage(allocator, colorImage, colorImageMemory);

    for (size_t imageFence = 0; imageFence < swapChainImages.size();
         imageFence++)
      vkResetFences(device, MAX_STAGING_BUFFERS,
                    stagingFences[imageFence].data());

    vkDestroyImageView(device, depthImageView, nullptr);

    vmaDestroyImage(allocator, depthImage, depthImageMemory);

    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (size_t ub = 0; ub < swapChainImages.size(); ub++) {
      vmaDestroyBuffer(allocator, uniformBuffers[ub], uniformBufferMemory[ub]);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkFreeCommandBuffers(device, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
  }

  void recreateSwapChain(const std::vector<uint32_t> &indices) {

    int width = 0, height = 0;
    SDL_Event event;

    while (width == 0 || height == 0) {
      SDL_GetWindowSize(window, &width, &height);
      SDL_WaitEventTimeout(&event, TIMEOUT);
      if (event.type == SDL_WINDOWEVENT_HIDDEN)
        continue;
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers(indices);
  }

  void createDescriptorSets() {

    std::vector<VkDescriptorSetLayout> layout(swapChainImages.size(),
                                              descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(swapChainImages.size());
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = layout.data();

    descriptorSets.resize(swapChainImages.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets");
    }

    for (size_t db = 0; db < swapChainImages.size(); db++) {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffers[db];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSets[db];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;
      descriptorWrites[0].pImageInfo = nullptr;
      descriptorWrites[0].pTexelBufferView = nullptr;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSets[db];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = nullptr;
      descriptorWrites[1].pImageInfo = &imageInfo;
      descriptorWrites[1].pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(device,
                             static_cast<uint32_t>(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
    }
  }

  void createCommandBuffers(const std::vector<uint32_t> &indices) {

    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers");
    }

    for (size_t i = 0; i < commandBuffers.size(); ++i) {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      beginInfo.pInheritanceInfo = nullptr;

      if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer");
      }

      VkRenderPassBeginInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = renderPass;
      renderPassInfo.framebuffer = swapChainFramebuffers[i];

      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = swapChainExtent;

      std::array<VkClearValue, 2> clearValues = {};
      clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
      clearValues[1].depthStencil = {1.0f, 0};
      renderPassInfo.clearValueCount =
          static_cast<uint32_t>(clearValues.size());
      renderPassInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                           VK_SUBPASS_CONTENTS_INLINE);

      vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        graphicsPipeline);

      VkBuffer vertexBuffers[] = {vertexBuffer[i]};
      VkDeviceSize offsets[] = {0};

      vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[i], 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(commandBuffers[i],
                              VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                              0, 1, &descriptorSets[i], 0, nullptr);

      vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);

      VkBuffer skyboxvertexBuffers[] = {skyboxVertexBuffer[i]};
      vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, skyboxvertexBuffers,
                             offsets);
      vkCmdBindDescriptorSets(commandBuffers[i],
                              VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                              0, 1, &descriptorSets[i], 0, nullptr);
      vkCmdDraw(commandBuffers[i], 36, 1, 0, 0);
      vkCmdEndRenderPass(commandBuffers[i]);

      if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
      }
    }
  }

  void createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create command Pool");
    }
  }

  void createFramebuffers() {

    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {

      std::array<VkImageView, 3> attachments = {colorImageView, depthImageView,
                                                swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount =
          static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                              &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }

  void createRenderPass() {

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {
        colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass");
    }
  }

  void createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkPipelineColorBlendAttachmentState colorBlendAttachement = {};
    colorBlendAttachement.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachement.blendEnable = VK_FALSE;
    colorBlendAttachement.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachement.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachement.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachement.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachement;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::array<VkDynamicState, 3> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                   VK_DYNAMIC_STATE_SCISSOR,
                                                   VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                               &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to crate pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; //&dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &graphicsPipeline) != VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
  }

  VkShaderModule createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create Shader Module");
    }
    return shaderModule;
  }

  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      swapChainImageViews[i] =
          createImageView(swapChainImages[i], swapChainImageFormat,
                          VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
  }

  void createSwapChain();

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  void createSurface() {
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
      throw std::runtime_error("failed to create window surface");
  }

  void createLogicalDevice() {
    QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        queueIndices.graphicsFamily.value(),
        queueIndices.presentFamily.value()};

    float queuePrioirity = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePrioirity;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create Logical Device");
    }

    vkGetDeviceQueue(device, queueIndices.graphicsFamily.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(device, queueIndices.presentFamily.value(), 0,
                     &presentQueue);
  }

  void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
      throw std::runtime_error("Failed to find GPUs with vulkan Support");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto &device : devices) {
      int score = rateDeviceSuitability(device);
      candidates.insert(std::make_pair(score, device));
    }
    if (candidates.rbegin()->first > 0 &&
        isDeviceSuitable(candidates.rbegin()->second)) {
      physicalDevice = candidates.rbegin()->second;
      msaaSamples = getMaxUsableSampleCount();
    } else {
      throw std::runtime_error("failed to find a suitable GPU");
    }
  }

  int rateDeviceSuitability(VkPhysicalDevice device) {
    int score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    if (!deviceFeatures.geometryShader) {
      return 0;
    }

    return score;
  }

  void updateUniformBuffer(uint32_t currentImage, UniformBufferObject &ubo) {

    void *data;
    vmaMapMemory(allocator, uniformBufferMemory[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(allocator, uniformBufferMemory[currentImage]);
  }

  void updateVertexBuffer(uint32_t currentImage,
                          const std::vector<Vertex> &vertices) {

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VmaAllocation stagingBufferMemory;
    VkBuffer stagingBuffer;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer[currentImage], bufferSize,
               currentImage);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void updateSkyBoxVertexBuffer(uint32_t currentImage,
                                const std::vector<Vertex> &vertices) {

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VmaAllocation stagingBufferMemory;
    VkBuffer stagingBuffer;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    copyBuffer(stagingBuffer, skyboxVertexBuffer[currentImage], bufferSize,
               currentImage);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void updateIndexBuffer(uint32_t currentImage,
                         const std::vector<uint32_t> &indices) {

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    VmaAllocation stagingBufferMemory;
    VkBuffer stagingBuffer;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferMemory);

    void *data;
    ERR_GUARD_VULKAN(vmaMapMemory(allocator, stagingBufferMemory, &data));
    memcpy(data, indices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer[currentImage], bufferSize,
               currentImage);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers)
      return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                     &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to setup debug messenger");
    }
  }

  std::vector<const char *> getRequiredExtensions() {
    uint32_t extensionCount = 0;
    uint32_t requiredExtensionCount = 0;

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount, nullptr);
    std::vector<const char *> requiredExtensions(requiredExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount,
                                     requiredExtensions.data());

    if (enableValidationLayers) {
      requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (const auto &reqExtension : requiredExtensions) {
      if (!(std::find(extensions.begin(), extensions.end(), reqExtension) !=
            extensions.end()))
        throw std::runtime_error("Cannot Find All the required extensions");
    }
    return requiredExtensions;
  }

  void createInstance() {

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available");
    }
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create Instance");
    }
  }

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
      if ((typeFilter & (1 << i)) &&
          (memProperties.memoryTypes[i].propertyFlags & properties) ==
              properties) {
        return i;
      }
    }
    throw std::runtime_error("failed to find suitable memory type!");
  }

  VkSampleCountFlagBits getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts =
        std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts,
                 physicalDeviceProperties.limits.framebufferDepthSampleCounts);

    if (counts & VK_SAMPLE_COUNT_64_BIT) {
      return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
      return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
      return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
      return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
      return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
      return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
  }

  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkImageView> swapChainImageViews;

  std::vector<VkImage> swapChainImages;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;

  std::vector<VkSemaphore> renderFinishedSemaphores;

  std::vector<VkFence> inFlightFences;

  std::vector<std::vector<VkFence>> stagingFences;

  std::vector<VkFence> singleCommandBufferFences;

  std::vector<VkBuffer> uniformBuffers;

  std::vector<VmaAllocation> uniformBufferMemory;

  std::vector<VkDescriptorSet> descriptorSets;

  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

  VkImage colorImage;

  VmaAllocation colorImageMemory;

  VkImageView colorImageView;

  uint32_t mipLevels;

  std::vector<VkBuffer> indexBuffer;

  std::vector<VmaAllocation> indexBufferMemory;

  std::vector<VkBuffer> vertexBuffer;

  std::vector<VmaAllocation> vertexBufferMemory;

  std::vector<VkBuffer> skyboxVertexBuffer;

  std::vector<VmaAllocation> skyBoxVertexBufferMemory;

  bool framebufferResized = false;

  bool modelChanged = false;

  std::vector<size_t> stagingBufferCurrent;

  size_t singleCommandBufferCurrent = 0;

  size_t currentFrame = 0;

  VkPipeline graphicsPipeline;

  VkCommandPool commandPool;

  VkSwapchainKHR swapChain;

  VkFormat swapChainImageFormat;

  VkExtent2D swapChainExtent;

  SDL_Window *window;

  VkInstance instance;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkDevice device;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkQueue graphicsQueue;

  VkSurfaceKHR surface;

  VkQueue presentQueue;

  VkRenderPass renderPass;

  VkPipelineLayout pipelineLayout;

  VkDescriptorSetLayout descriptorSetLayout;

  VkDescriptorPool descriptorPool;

  VkImage textureImage;

  VmaAllocation textureImageMemory;

  VkImageView textureImageView;

  VkSampler textureSampler;

  VkImage depthImage;

  VmaAllocation depthImageMemory;

  VkImageView depthImageView;

  VmaAllocator allocator;

  int time = 0;
};

#endif
