VULKAN_SDK_PATH = /D/vulkan/1.1.108.0/x86_64
STB_INCLUDE_PATH = /D/work/vulkan/triangle/libraries
TINYOBJ_INCLUDE_PATH = /D/work/vulkan/triangle/libraries

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(TINYOBJ_INCLUDE_PATH) -O3 -DNDEBUG #-pedantic -Wall -Wextra -Weffc++

LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./VulkanTest

clean:
	rm	-f	VulkanTest
