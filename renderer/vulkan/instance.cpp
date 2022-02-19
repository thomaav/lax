#include <algorithm>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace Vulkan
{

void Instance::addExtension(const char *extension)
{
	extensions.push_back(extension);
}

void Instance::addLayer(const char *layer)
{
	layers.push_back(layer);
}

void Instance::checkExtensionsAvailable()
{
	u32 instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data());

	for (auto &extension : extensions)
	{
		auto v = std::find_if(availableInstanceExtensions.begin(), availableInstanceExtensions.end(),
		                      [&extension](const auto &availableExtension) {
			                      return std::string_view{ extension } ==
			                             std::string_view{ availableExtension.extensionName };
		                      });

		if (v == availableInstanceExtensions.end())
		{
			terminate("Extension %s not found", extension);
		}
	}
}

void Instance::checkLayersAvailable()
{
	u32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

	std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data());

	for (auto &layer : layers)
	{
		auto v =
		    std::find_if(availableInstanceLayers.begin(), availableInstanceLayers.end(),
		                 [&layer](const auto &availableInstanceLayer) {
			                 return std::string_view{ layer } == std::string_view{ availableInstanceLayer.layerName };
		                 });

		if (v == availableInstanceLayers.end())
		{
			terminate("Layer %s not found", layer);
		}
	}
}

void Instance::build()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = nullptr;
	appInfo.applicationVersion = VK_NULL_HANDLE;
	appInfo.pEngineName = nullptr;
	appInfo.engineVersion = VK_NULL_HANDLE;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	checkExtensionsAvailable();
	instanceCreateInfo.enabledExtensionCount = extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	checkLayersAvailable();
	instanceCreateInfo.enabledLayerCount = layers.size();
	instanceCreateInfo.ppEnabledLayerNames = layers.data();

	VULKAN_ASSERT_SUCCESS(vkCreateInstance(&instanceCreateInfo, nullptr, &handle));
}

void Instance::destroy()
{
	vkDestroyInstance(handle, nullptr);
}

} /* namespace Vulkan */
