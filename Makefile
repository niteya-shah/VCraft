VULKAN_SDK_PATH = /D/vulkan/1.1.108.0/x86_64
STB_INCLUDE_PATH = /D/git/vulkan/VkX/libraries
TINYOBJ_INCLUDE_PATH = /D/git/vulkan/VkX/libraries
SDL_INCLUDE_PATH = /usr/include/SDL2

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(TINYOBJ_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH) -O3 -DNDEBUG #-pedantic -Wall -Wextra -Weffc++

LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `sdl2-config --cflags --libs` -lvulkan

remake = $(touch VulkanTest.out)

VulkanTest.out: main.cpp
	g++ $(CFLAGS) main.cpp $(LDFLAGS) -o VulkanTest.out

.PHONY: test clean

test: VulkanTest.out
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./VulkanTest.out

clean:
	rm	-f	VulkanTest.out
