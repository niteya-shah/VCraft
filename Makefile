VULKAN_SDK_PATH = /D/vulkan/1.1.114.0/x86_64
STB_INCLUDE_PATH = /D/git/vulkan/VCraft/libraries
TINYOBJ_INCLUDE_PATH = /D/git/vulkan/VCraft/libraries
SDL_INCLUDE_PATH = /usr/include/SDL2
VMA_INCLUDE_PATH = /D/git/vulkan/VulkanMemoryAllocator/src/
USR_INCLUDE_PATH = /usr/lib/

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(VULKAN_SDK_PATH)/bin -I$(VULKAN_SDK_PATH)/lib -I$(STB_INCLUDE_PATH) -I$(TINYOBJ_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH) -I$(VMA_INCLUDE_PATH) -I$(USR_INCLUDE_PATH) -DNDEBUG #-pedantic -Wall -Wextra -Weffc++

LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `sdl2-config --cflags --libs` -lvulkan -L$(VK_LAYER_PATH)

VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d

remake = $(touch VulkanTest.out)

VulkanTest.out: main.cpp
	g++ $(CFLAGS) main.cpp $(LDFLAGS) -O3 -o VulkanTest.out

.PHONY: test clean

test: VulkanTest.out


clean:
	rm	-f	VulkanTest.out
