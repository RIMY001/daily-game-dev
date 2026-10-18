# Vulkan 学习手册：从零到渲染三角形

> **作者的话**：这份手册不是 API 文档，而是一个图形工程师带新人入门 Vulkan 的笔记。我会用类比解释概念，用代码展示实践，用踩坑经验分享教训。学完这份手册，你就能渲染出第一个三角形——这是所有图形程序的起点。

---

## 目录

- [第一章：为什么需要 Vulkan？](#第一章为什么需要-vulkan)
  - [1.1 OpenGL 的问题](#11-opengl-的问题)
  - [1.2 Vulkan 的设计哲学](#12-vulkan-的设计哲学)
  - [1.3 显式控制的代价与收益](#13-显式控制的代价与收益)
- [第二章：环境搭建](#第二章环境搭建)
  - [2.1 安装 Vulkan SDK](#21-安装-vulkan-sdk)
  - [2.2 验证安装](#22-验证安装)
  - [2.3 第一个 Vulkan 程序](#23-第一个-vulkan-程序)
- [第三章：实例与设备](#第三章实例与设备)
  - [3.1 核心概念：Vulkan 的对象模型](#31-核心概念vulkan-的对象模型)
  - [3.2 创建实例](#32-创建实例)
  - [3.3 选择物理设备](#33-选择物理设备)
  - [3.4 创建逻辑设备](#34-创建逻辑设备)
- [第四章：交换链](#第四章交换链)
  - [4.1 核心概念：双缓冲与三缓冲](#41-核心概念双缓冲与三缓冲)
  - [4.2 创建交换链](#42-创建交换链)
  - [4.3 图像视图](#43-图像视图)
- [第五章：命令缓冲](#第五章命令缓冲)
  - [5.1 核心概念：命令录制与执行](#51-核心概念命令录制与执行)
  - [5.2 命令池与命令缓冲](#52-命令池与命令缓冲)
  - [5.3 录制第一个命令](#53-录制第一个命令)
- [第六章：渲染管线](#第六章渲染管线)
  - [6.1 核心概念：状态对象](#61-核心概念状态对象)
  - [6.2 创建图形管线](#62-创建图形管线)
  - [6.3 渲染 Pass 与帧缓冲](#63-渲染-pass-与帧缓冲)
- [第七章：顶点缓冲](#第七章顶点缓冲)
  - [7.1 核心概念：GPU 内存管理](#71-核心概念gpu-内存管理)
  - [7.2 创建顶点缓冲](#72-创建顶点缓冲)
  - [7.3 绑定与绘制](#73-绑定与绘制)
- [第八章：着色器模块](#第八章着色器模块)
  - [8.1 核心概念：SPIR-V 与 Shader 编译](#81-核心概念spir-v-与-shader-编译)
  - [8.2 编写第一个 Shader](#82-编写第一个-shader)
  - [8.3 加载 Shader 模块](#83-加载-shader-模块)
- [第九章：纹理](#第九章纹理)
  - [9.1 核心概念：图像与采样器](#91-核心概念图像与采样器)
  - [9.2 创建纹理图像](#92-创建纹理图像)
  - [9.3 采样器与描述符](#93-采样器与描述符)
- [第十章：深度测试](#第十章深度测试)
  - [10.1 核心概念：深度缓冲](#101-核心概念深度缓冲)
  - [10.2 创建深度图像](#102-创建深度图像)
  - [10.3 配置深度测试](#103-配置深度测试)
- [第十一章：帧同步](#第十一章帧同步)
  - [11.1 核心概念：Fence 与 Semaphore](#111-核心概念fence-与-semaphore)
  - [11.2 同步原语](#112-同步原语)
  - [11.3 完整的渲染循环](#113-完整的渲染循环)
- [第十二章：从零到三角形的最短路径](#第十二章从零到三角形的最短路径)
  - [12.1 最小化清单](#121-最小化清单)
  - [12.2 完整代码](#122-完整代码)
  - [12.3 运行与调试](#123-运行与调试)

---

## 第一章：为什么需要 Vulkan？

### 1.1 OpenGL 的问题

**类比**：OpenGL 像一个全包式餐厅。你告诉服务员"我要一份牛排"，他帮你选肉、调味、烹饪、摆盘。你不需要知道细节，但你也无法控制——如果厨师今天心情不好，你的牛排可能就煎老了。

**OpenGL 的设计问题**：

1. **隐式状态机**
   ```cpp
   // OpenGL：全局状态，改一个影响所有
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   // 现在所有绘制都用这个混合模式
   // 想改回来？必须手动恢复
   ```

2. **驱动层黑盒**
   - 驱动帮你管理内存、同步、状态验证
   - 你看不到驱动做了什么，也优化不了
   - 不同厂商驱动行为不一致（NVIDIA vs AMD vs Intel）

3. **单线程瓶颈**
   ```cpp
   // OpenGL：命令必须在一个线程顺序提交
   // 多核 CPU 利用率低
   void render() {
       glDrawArrays(...);  // CPU 等待 GPU
       glDrawArrays(...);  // GPU 等待 CPU
       // 无法并行录制命令
   }
   ```

4. **运行时错误检查**
   ```cpp
   // OpenGL：错误在运行时才发现
   glBindTexture(GL_TEXTURE_2D, 0);  // 绑定失败
   // 程序继续运行，但渲染结果错误
   // 调试困难
   ```

### 1.2 Vulkan 的设计哲学

**类比**：Vulkan 像自己做饭。你去超市选肉、买调料、自己烹饪。虽然麻烦，但你知道每一步在做什么，可以精确控制火候、调味。

**Vulkan 的核心设计**：

1. **显式控制**
   ```cpp
   // Vulkan：所有状态都在 Pipeline 对象中
   VkPipeline pipeline;
   vkCreateGraphicsPipelines(device, ..., &pipeline);
   // 状态不可变，切换 Pipeline 即可
   // 驱动不需要猜测你的意图
   ```

2. **多线程友好**
   ```cpp
   // Vulkan：命令缓冲可以并行录制
   void render() {
       // 主线程：逻辑更新
       // 工作线程 1：录制命令缓冲 A
       // 工作线程 2：录制命令缓冲 B
       // 渲染线程：提交所有命令缓冲
   }
   ```

3. **开发期验证**
   ```cpp
   // Vulkan：验证层在开发期检查错误
   // Release 模式禁用，零开销
   #ifndef NDEBUG
   enableValidationLayers();
   #endif
   ```

4. **跨平台**
   - Windows、Linux、Android、macOS（通过 MoltenVK）
   - 一套代码，多平台运行

### 1.3 显式控制的代价与收益

| 方面 | OpenGL | Vulkan |
|------|--------|--------|
| **代码量** | 少（驱动帮你做） | 多（所有都要手动） |
| **学习曲线** | 平缓 | 陡峭 |
| **性能** | 依赖驱动优化 | 可预测，可优化 |
| **调试** | 运行时错误 | 开发期验证 |
| **多线程** | 受限 | 原生支持 |
| **控制力** | 低（黑盒） | 高（白盒） |

**收益**：
- 性能可预测（没有驱动黑盒）
- 多线程利用率提高 2-3 倍
- 错误在开发期发现
- 跨平台一致行为

**代价**：
- 绘制一个三角形需要 1000+ 行代码
- 必须理解 GPU 工作原理
- 内存管理、同步都要手动
- 调试 Vulkan 代码本身很困难

**工程师的建议**：
> 如果你只是想快速出效果，用 OpenGL 或 Unity。如果你想理解渲染引擎底层，或者做引擎开发，Vulkan 是必经之路。它很难，但值得。

---

## 第二章：环境搭建

**难度**：入门 | **优先级**：必学

### 2.1 安装 Vulkan SDK

**Windows**：
```bash
# 1. 下载 Vulkan SDK
# https://vulkan.lunarg.com/sdk/home
# 选择 Windows 版本

# 2. 安装（默认路径）
# C:\VulkanSDK\1.3.xxx\x86_64

# 3. 设置环境变量
# 系统会自动设置，如果没有：
# VULKAN_SDK = C:\VulkanSDK\1.3.xxx\x86_64
# PATH 添加 %VULKAN_SDK%\bin

# 4. 验证
vulkaninfo --summary
```

**Linux**：
```bash
# Ubuntu/Debian
sudo apt install vulkan-tools libvulkan-dev

# 验证
vulkaninfo --summary
```

**macOS**：
```bash
# 通过 MoltenVK 支持（Vulkan 转 Metal）
# 下载 Vulkan SDK，安装时选择 MoltenVK
```

### 2.2 验证安装

```cpp
// test_vulkan.cpp
#include <vulkan/vulkan.h>
#include <iostream>

int main() {
    // 检查 Vulkan 版本
    std::cout << "Vulkan Header Version: " 
              << VK_HEADER_VERSION << std::endl;
    
    // 创建实例（测试）
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    
    if (result == VK_SUCCESS) {
        std::cout << "Vulkan initialized successfully!" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 0;
    } else {
        std::cout << "Failed to initialize Vulkan! Error: " 
                  << result << std::endl;
        return 1;
    }
}
```

**编译运行**：
```bash
# Windows (MSVC)
cl test_vulkan.cpp /I%VULKAN_SDK%\Include /link %VULKAN_SDK%\Lib\vulkan-1.lib
test_vulkan.exe

# Linux
g++ test_vulkan.cpp -lvulkan -o test_vulkan
./test_vulkan

# 预期输出：
# Vulkan Header Version: 268
# Vulkan initialized successfully!
```

### 2.3 第一个 Vulkan 程序

**CMakeLists.txt**：
```cmake
cmake_minimum_required(VERSION 3.16)
project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Vulkan
find_package(Vulkan REQUIRED)

# 添加可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 链接 Vulkan
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# 包含头文件
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
```

**项目结构**：
```
VulkanTriangle/
├── CMakeLists.txt
├── src/
│   └── main.cpp
└── shaders/
    ├── shader.vert
    └── shader.frag
```

**常见错误**：
1. **找不到 Vulkan 头文件**
   - 检查 `VULKAN_SDK` 环境变量
   - CMake 中 `find_package(Vulkan REQUIRED)` 是否成功

2. **链接错误**
   - Windows：确认链接了 `vulkan-1.lib`
   - Linux：确认安装了 `libvulkan-dev`

3. **运行时崩溃**
   - 检查 GPU 驱动是否支持 Vulkan
   - 运行 `vulkaninfo` 查看支持的版本

---

## 第三章：实例与设备

**难度**：入门 | **优先级**：必学

### 3.1 核心概念：Vulkan 的对象模型

**类比**：Vulkan 的对象模型像公司组织架构。

```
VkInstance（公司总部）
    └─► VkPhysicalDevice（分公司/ GPU）
        └─► VkDevice（部门/ 逻辑设备）
            ├─► VkQueue（团队/ 命令队列）
            ├─► VkSwapchain（窗口/ 交换链）
            ├─► VkPipeline（流程/ 渲染管线）
            └─► VkBuffer（资源/ 缓冲区）
```

**关键原则**：
1. **创建顺序**：从上到下（Instance → Device → Resources）
2. **销毁顺序**：从下到上（Resources → Device → Instance）
3. **生命周期**：父对象销毁前，必须先销毁所有子对象

### 3.2 创建实例

**核心 API**：
```cpp
VkResult vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance
);
```

**代码实现**：
```cpp
class VulkanInstance {
    VkInstance instance = VK_NULL_HANDLE;
    
public:
    void init() {
        // 1. 应用信息（可选，但推荐）
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        // 2. 扩展（平台相关）
        std::vector<const char*> extensions;
        #ifdef _WIN32
        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_win32_surface");
        #endif
        
        // 3. 验证层（Debug 模式）
        std::vector<const char*> layers;
        #ifndef NDEBUG
        layers.push_back("VK_LAYER_KHRONOS_validation");
        #endif
        
        // 4. 创建实例
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
        
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
        
        std::cout << "Vulkan instance created successfully!" << std::endl;
    }
    
    void cleanup() {
        if (instance) {
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }
    }
    
    VkInstance get() const { return instance; }
};
```

**关键参数解释**：
- `pApplicationInfo`：应用信息（名称、版本、API 版本）
- `enabledExtensionCount`：启用的扩展数量
- `ppEnabledExtensionNames`：扩展名称数组
- `enabledLayerCount`：启用的验证层数量
- `ppEnabledLayerNames`：验证层名称数组

**常见错误**：
1. **扩展不支持**
   ```cpp
   // 错误：直接添加不存在的扩展
   extensions.push_back("VK_KHR_nonexistent");
   
   // 正确：先查询支持的扩展
   uint32_t extensionCount = 0;
   vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
   std::vector<VkExtensionProperties> extensions(extensionCount);
   vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
   ```

2. **验证层不可用**
   ```cpp
   // 错误：假设验证层一定存在
   layers.push_back("VK_LAYER_KHRONOS_validation");
   
   // 正确：检查是否支持
   bool checkValidationLayerSupport() {
       uint32_t layerCount;
       vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
       std::vector<VkLayerProperties> availableLayers(layerCount);
       vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
       
       for (const char* layerName : {"VK_LAYER_KHRONOS_validation"}) {
           bool found = false;
           for (const auto& layer : availableLayers) {
               if (strcmp(layerName, layer.layerName) == 0) {
                   found = true;
                   break;
               }
           }
           if (!found) return false;
       }
       return true;
   }
   ```

### 3.3 选择物理设备

**核心 API**：
```cpp
VkResult vkEnumeratePhysicalDevices(
    VkInstance instance,
    uint32_t* pPhysicalDeviceCount,
    VkPhysicalDevice* pPhysicalDevices
);
```

**代码实现**：
```cpp
VkPhysicalDevice pickPhysicalDevice(VkInstance instance) {
    // 1. 枚举所有物理设备
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    
    // 2. 选择最合适的设备
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }
    
    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
    
    return physicalDevice;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    // 优先选择独立显卡
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
```

**设备类型**：
- `VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU`：独立显卡（性能最强）
- `VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU`：集成显卡（功耗低）
- `VK_PHYSICAL_DEVICE_TYPE_CPU`：CPU 模拟（极慢，调试用）

**常见错误**：
1. **没有 GPU 支持 Vulkan**
   - 检查 `vulkaninfo` 输出
   - 更新 GPU 驱动

2. **选择了错误的设备**
   - 笔记本可能有两个 GPU（集成 + 独立）
   - 优先选择独立显卡

### 3.4 创建逻辑设备

**核心 API**：
```cpp
VkResult vkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice
);
```

**代码实现**：
```cpp
class LogicalDevice {
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice;
    uint32_t graphicsFamily = UINT32_MAX;
    
public:
    void create(VkPhysicalDevice physDevice) {
        physicalDevice = physDevice;
        
        // 1. 查找队列族
        findQueueFamilies();
        
        // 2. 创建队列信息
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        // 3. 设备特性
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        
        // 4. 扩展
        std::vector<const char*> extensions;
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        
        // 5. 创建设备
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }
        
        // 6. 获取队列
        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        
        std::cout << "Logical device created successfully!" << std::endl;
    }
    
    void cleanup() {
        if (device) {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }
    }
    
private:
    void findQueueFamilies() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = i;
                break;
            }
        }
    }
};
```

**队列族类型**：
- `VK_QUEUE_GRAPHICS_BIT`：图形队列（必须）
- `VK_QUEUE_COMPUTE_BIT`：计算队列
- `VK_QUEUE_TRANSFER_BIT`：传输队列
- `VK_QUEUE_SPARSE_BINDING_BIT`：稀疏绑定

**常见错误**：
1. **忘记启用扩展**
   ```cpp
   // 错误：使用交换链但未启用扩展
   extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);  // 必须！
   ```

2. **队列族索引错误**
   ```cpp
   // 错误：使用不存在的队列族
   queueCreateInfo.queueFamilyIndex = 999;  // 崩溃！
   
   // 正确：先查询队列族
   findQueueFamilies();
   ```

---

## 第四章：交换链

**难度**：入门 | **优先级**：必学

### 4.1 核心概念：双缓冲与三缓冲

**类比**：交换链像翻页动画。

**单缓冲**（会撕裂）：
```
帧 1: [绘制中] → 显示
帧 2: [绘制中] → 显示（可能画到一半就显示）
```

**双缓冲**（无撕裂）：
```
缓冲 A: [绘制帧 1] → 显示
缓冲 B: [绘制帧 2] → 等待
交换 → 缓冲 B 显示，缓冲 A 绘制帧 3
```

**三缓冲**（更低延迟）：
```
缓冲 A: [绘制帧 1] → 显示
缓冲 B: [绘制帧 2] → 等待
缓冲 C: [绘制帧 3] → 等待
交换 → A 显示，B 绘制，C 等待
```

### 4.2 创建交换链

**核心 API**：
```cpp
VkResult vkCreateSwapchainKHR(
    VkDevice device,
    const VkSwapchainCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchain
);
```

**代码实现**：
```cpp
class SwapChain {
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkDevice device;
    VkSurfaceKHR surface;
    
    std::vector<VkImage> images;
    VkFormat imageFormat;
    VkExtent2D extent;
    
public:
    void create(VkDevice dev, VkPhysicalDevice physDevice, VkSurfaceKHR surf,
                uint32_t width, uint32_t height) {
        device = dev;
        surface = surf;
        
        // 1. 查询交换链支持
        SwapChainSupportDetails support = querySwapChainSupport(physDevice);
        
        // 2. 选择配置
        VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes);
        VkExtent2D extent = chooseSwapExtent(support.capabilities, width, height);
        
        // 3. 图像数量（至少 minImageCount + 1）
        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 &&
            imageCount > support.capabilities.maxImageCount) {
            imageCount = support.capabilities.maxImageCount;
        }
        
        // 4. 创建交换链
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        
        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }
        
        // 5. 获取图像
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());
        
        imageFormat = format.format;
        this->extent = extent;
        
        std::cout << "Swap chain created with " << imageCount << " images!" << std::endl;
    }
    
private:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        
        return details;
    }
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return formats[0];
    }
    
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) {
        for (const auto& mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;  // 三重缓冲，最低延迟
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;  // VSync，保证不撕裂
    }
    
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                 uint32_t width, uint32_t height) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = {width, height};
            
            actualExtent.width = std::clamp(actualExtent.width,
                capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height,
                capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            
            return actualExtent;
        }
    }
};
```

**呈现模式选择**：
- `VK_PRESENT_MODE_IMMEDIATE_KHR`：无 VSync，可能撕裂（最快）
- `VK_PRESENT_MODE_FIFO_KHR`：VSync，不撕裂（保证支持）
- `VK_PRESENT_MODE_MAILBOX_KHR`：三重缓冲，低延迟（推荐）

**常见错误**：
1. **窗口大小变化**
   ```cpp
   // 错误：不处理窗口大小变化
   // 结果：VK_ERROR_OUT_OF_DATE_KHR
   
   // 正确：重建交换链
   void handleResize() {
       vkDeviceWaitIdle(device);
       swapChain.cleanup();
       swapChain.create(device, physicalDevice, surface, newWidth, newHeight);
   }
   ```

2. **图像格式不匹配**
   ```cpp
   // 错误：假设格式一定是 RGBA8
   // 正确：查询并选择合适的格式
   VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(support.formats);
   ```

### 4.3 图像视图

**核心 API**：
```cpp
VkResult vkCreateImageView(
    VkDevice device,
    const VkImageViewCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkImageView* pView
);
```

**代码实现**：
```cpp
std::vector<VkImageView> imageViews;

void createImageViews() {
    imageViews.resize(images.size());
    
    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view!");
        }
    }
}
```

**图像视图类型**：
- `VK_IMAGE_VIEW_TYPE_1D`：1D 纹理
- `VK_IMAGE_VIEW_TYPE_2D`：2D 纹理（最常用）
- `VK_IMAGE_VIEW_TYPE_3D`：3D 纹理
- `VK_IMAGE_VIEW_TYPE_CUBE`：立方体贴图

---

## 第五章：命令缓冲

**难度**：进阶 | **优先级**：必学

### 5.1 核心概念：命令录制与执行

**类比**：命令缓冲像录像带。

**OpenGL（即时模式）**：
```cpp
// 立即执行
glBegin(GL_TRIANGLES);
glVertex3f(0, 0, 0);
glVertex3f(1, 0, 0);
glVertex3f(0, 1, 0);
glEnd();
// CPU 等待 GPU 执行
```

**Vulkan（预录制模式）**：
```cpp
// 1. 录制命令（CPU 工作）
vkBeginCommandBuffer(cmd, ...);
vkCmdDraw(cmd, 3, 1, 0, 0);
vkEndCommandBuffer(cmd);

// 2. 提交执行（GPU 工作）
vkQueueSubmit(queue, 1, &cmd, ...);
// CPU 可以继续录制下一个命令缓冲
```

**优势**：
- CPU 和 GPU 并行工作
- 可以并行录制多个命令缓冲
- 可以重复使用命令缓冲

### 5.2 命令池与命令缓冲

**核心 API**：
```cpp
VkResult vkCreateCommandPool(
    VkDevice device,
    const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCommandPool* pCommandPool
);

VkResult vkAllocateCommandBuffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    VkCommandBuffer* pCommandBuffers
);
```

**代码实现**：
```cpp
class CommandPool {
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDevice device;
    
public:
    void create(VkDevice dev, uint32_t queueFamilyIndex) {
        device = dev;
        
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        
        VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }
    
    void cleanup() {
        if (commandPool) {
            vkDestroyCommandPool(device, commandPool, nullptr);
            commandPool = VK_NULL_HANDLE;
        }
    }
    
    VkCommandPool get() const { return commandPool; }
};

class CommandBuffer {
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkDevice device;
    VkCommandPool commandPool;
    
public:
    void allocate(VkDevice dev, VkCommandPool pool) {
        device = dev;
        commandPool = pool;
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        
        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer!");
        }
    }
    
    void beginRecording() {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        
        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
    }
    
    void endRecording() {
        VkResult result = vkEndCommandBuffer(commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
    
    VkCommandBuffer get() const { return commandBuffer; }
};
```

**命令缓冲级别**：
- `VK_COMMAND_BUFFER_LEVEL_PRIMARY`：主命令缓冲（可直接提交）
- `VK_COMMAND_BUFFER_LEVEL_SECONDARY`：辅助命令缓冲（必须从主命令缓冲调用）

**命令池标志**：
- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`：短暂生命周期（频繁重置）
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`：允许单独重置命令缓冲

### 5.3 录制第一个命令

**代码实现**：
```cpp
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // 1. 开始录制
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    // 2. 开始渲染 Pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // 3. 绑定管线
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    
    // 4. 设置视口（动态状态）
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    // 5. 设置裁剪矩形（动态状态）
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    // 6. 绘制
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    
    // 7. 结束渲染 Pass
    vkCmdEndRenderPass(commandBuffer);
    
    // 8. 结束录制
    vkEndCommandBuffer(commandBuffer);
}
```

**常用命令**：
- `vkCmdBindPipeline`：绑定管线
- `vkCmdBindVertexBuffers`：绑定顶点缓冲
- `vkCmdBindIndexBuffer`：绑定索引缓冲
- `vkCmdDraw`：绘制（非索引）
- `vkCmdDrawIndexed`：绘制（索引）
- `vkCmdSetViewport`：设置视口
- `vkCmdSetScissor`：设置裁剪矩形

**常见错误**：
1. **忘记开始/结束渲染 Pass**
   ```cpp
   // 错误：直接绘制
   vkCmdDraw(cmd, 3, 1, 0, 0);  // 崩溃！
   
   // 正确：必须在渲染 Pass 内绘制
   vkCmdBeginRenderPass(cmd, ...);
   vkCmdDraw(cmd, 3, 1, 0, 0);
   vkCmdEndRenderPass(cmd);
   ```

2. **动态状态未设置**
   ```cpp
   // 错误：管线配置了动态视口，但未设置
   // 结果：渲染错误
   
   // 正确：设置动态状态
   vkCmdSetViewport(cmd, 0, 1, &viewport);
   vkCmdSetScissor(cmd, 0, 1, &scissor);
   ```

---

## 第六章：渲染管线

**难度**：进阶 | **优先级**：必学

### 6.1 核心概念：状态对象

**类比**：渲染管线像工厂流水线。

**OpenGL（全局状态）**：
```cpp
// 每次绘制前设置状态
glEnable(GL_BLEND);
glBlendFunc(...);
glUseProgram(program);
glBindVertexArray(vao);
glDrawArrays(...);
// 状态是全局的，改一个影响所有
```

**Vulkan（状态对象）**：
```cpp
// 创建时配置所有状态
VkPipeline pipeline;
vkCreateGraphicsPipelines(..., &pipeline);
// 状态不可变，切换 Pipeline 即可
vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
```

**优势**：
- 驱动可以优化状态切换
- 状态明确，易于调试
- 可以预创建所有变体

### 6.2 创建图形管线

**核心 API**：
```cpp
VkResult vkCreateGraphicsPipelines(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines
);
```

**代码实现**：
```cpp
VkPipeline graphicsPipeline;
VkPipelineLayout pipelineLayout;

void createGraphicsPipeline() {
    // 1. 加载 Shader
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");
    
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    
    // 2. Shader 阶段
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // 3. 顶点输入（暂时为空）
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    // 4. 输入装配
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // 5. 视口和裁剪（动态状态）
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // 6. 光栅化
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // 7. 多重采样
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // 8. 深度模板（暂时禁用）
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    
    // 9. 颜色混合
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // 10. 动态状态
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // 11. 管线布局
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    
    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    
    // 12. 创建管线
    VkGraphicsPipelineCreateInfo pipelineInfo{};
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
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    
    // 13. 清理 Shader 模块
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    return shaderModule;
}
```

**管线状态详解**：
- **顶点输入**：顶点格式、绑定
- **输入装配**：图元类型（点、线、三角形）
- **光栅化**：多边形模式、剔除、正面方向
- **多重采样**：抗锯齿
- **深度模板**：深度测试、模板测试
- **颜色混合**：混合模式、写入掩码
- **动态状态**：运行时可修改的状态

**常见错误**：
1. **Shader 阶段不匹配**
   ```cpp
   // 错误：顶点着色器绑定到片元阶段
   vertShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;  // 崩溃！
   
   // 正确：匹配阶段
   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
   ```

2. **渲染 Pass 不匹配**
   ```cpp
   // 错误：管线的渲染 Pass 与实际使用的不一致
   pipelineInfo.renderPass = wrongRenderPass;  // 渲染错误
   
   // 正确：使用相同的渲染 Pass
   pipelineInfo.renderPass = renderPass;
   ```

### 6.3 渲染 Pass 与帧缓冲

**核心 API**：
```cpp
VkResult vkCreateRenderPass(
    VkDevice device,
    const VkRenderPassCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkRenderPass* pRenderPass
);

VkResult vkCreateFramebuffer(
    VkDevice device,
    const VkFramebufferCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFramebuffer* pFramebuffer
);
```

**代码实现**：
```cpp
VkRenderPass renderPass;
std::vector<VkFramebuffer> framebuffers;

void createRenderPass() {
    // 1. 颜色附件
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // 2. 子 Pass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    // 3. 依赖
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    // 4. 创建渲染 Pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
}

void createFramebuffers() {
    framebuffers.resize(swapChainImageViews.size());
    
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        
        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
    }
}
```

**附件操作**：
- `VK_ATTACHMENT_LOAD_OP_CLEAR`：清除（每帧开始）
- `VK_ATTACHMENT_LOAD_OP_LOAD`：加载（保留内容）
- `VK_ATTACHMENT_LOAD_OP_DONT_CARE`：不关心（性能最优）

**图像布局**：
- `VK_IMAGE_LAYOUT_UNDEFINED`：未定义（初始状态）
- `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`：颜色附件
- `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`：呈现到交换链

---

## 第七章：顶点缓冲

**难度**：进阶 | **优先级**：必学

### 7.1 核心概念：GPU 内存管理

**类比**：GPU 内存像仓库。

**内存类型**：
- `DEVICE_LOCAL`：GPU 本地内存（VRAM，最快，CPU 不可访问）
- `HOST_VISIBLE`：主机可见内存（RAM，CPU 可访问，较慢）
- `HOST_CACHED`：主机缓存内存（RAM，CPU 访问快）

**传输策略**：
```
CPU 数据 → HOST_VISIBLE 缓冲 → DEVICE_LOCAL 缓冲（GPU 使用）
         （Staging Buffer）    （Vertex Buffer）
```

### 7.2 创建顶点缓冲

**核心 API**：
```cpp
VkResult vkCreateBuffer(
    VkDevice device,
    const VkBufferCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBuffer* pBuffer
);

VkResult vkAllocateMemory(
    VkDevice device,
    const VkMemoryAllocateInfo* pAllocateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDeviceMemory* pMemory
);
```

**代码实现**：
```cpp
struct Vertex {
    float pos[3];
    float color[3];
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
};

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;

void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
    // 1. 创建 Staging Buffer（CPU 可访问）
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);
    
    // 2. 拷贝数据到 Staging Buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    // 3. 创建 Vertex Buffer（GPU 本地）
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer, vertexBufferMemory);
    
    // 4. 从 Staging 拷贝到 Vertex Buffer
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    
    // 5. 清理 Staging Buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // 1. 创建缓冲
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    
    // 2. 查询内存需求
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    
    // 3. 分配内存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    
    // 4. 绑定内存
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(commandBuffer);
}
```

**常见错误**：
1. **内存类型不匹配**
   ```cpp
   // 错误：CPU 访问 DEVICE_LOCAL 内存
   vkMapMemory(device, deviceLocalMemory, ...);  // 崩溃！
   
   // 正确：使用 HOST_VISIBLE 内存
   createBuffer(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, ...);
   ```

2. **忘记绑定内存**
   ```cpp
   // 错误：创建缓冲后未绑定内存
   vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
   vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, ...);  // 崩溃！
   
   // 正确：绑定内存
   vkBindBufferMemory(device, buffer, bufferMemory, 0);
   ```

### 7.3 绑定与绘制

**代码实现**：
```cpp
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // ... 开始渲染 Pass ...
    
    // 绑定顶点缓冲
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    // 绘制
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    
    // ... 结束渲染 Pass ...
}
```

**vkCmdDraw 参数**：
- `vertexCount`：顶点数量
- `instanceCount`：实例数量（实例化渲染）
- `firstVertex`：第一个顶点偏移
- `firstInstance`：第一个实例偏移

---

## 第八章：着色器模块

**难度**：进阶 | **优先级**：必学

### 8.1 核心概念：SPIR-V 与 Shader 编译

**类比**：SPIR-V 像字节码。

**编译流程**：
```
GLSL/HLSL（源代码） → glslangValidator/glslc → SPIR-V（中间表示） → Vulkan 驱动 → GPU 机器码
```

**优势**：
- 跨平台（SPIR-V 是标准）
- 预编译（减少运行时开销）
- 优化（编译器优化 Shader）

### 8.2 编写第一个 Shader

**顶点着色器（shader.vert）**：
```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;
}
```

**片元着色器（shader.frag）**：
```glsl
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

**编译 Shader**：
```bash
# Windows
%VULKAN_SDK%\bin\glslc.exe shaders/shader.vert -o shaders/vert.spv
%VULKAN_SDK%\bin\glslc.exe shaders/shader.frag -o shaders/frag.spv

# Linux
glslc shaders/shader.vert -o shaders/vert.spv
glslc shaders/shader.frag -o shaders/frag.spv
```

### 8.3 加载 Shader 模块

**代码实现**：
```cpp
std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    
    return shaderModule;
}

// 使用
auto vertShaderCode = readFile("shaders/vert.spv");
auto fragShaderCode = readFile("shaders/frag.spv");

VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
```

**常见错误**：
1. **Shader 编译失败**
   ```bash
   # 检查错误信息
   glslc shaders/shader.vert -o shaders/vert.spv
   # shaders/shader.vert:10: error: 'inPosition' : undeclared identifier
   ```

2. **入口点不匹配**
   ```cpp
   // 错误：Shader 入口点不是 main
   shaderStageInfo.pName = "main";  // 但 Shader 中是 void vertexShader()
   
   // 正确：匹配入口点
   shaderStageInfo.pName = "main";  // Shader 中是 void main()
   ```

---

## 第九章：纹理

**难度**：进阶 | **优先级**：可跳过（初学）

### 9.1 核心概念：图像与采样器

**类比**：纹理像照片，采样器像放大镜。

**图像（Image）**：
- 存储像素数据
- 多种格式（RGBA8、RGBA16F、Depth）
- 多种布局（优化不同用途）

**采样器（Sampler）**：
- 定义如何读取纹理
- 滤波模式（最近邻、双线性、各向异性）
- 寻址模式（重复、镜像、边框）

### 9.2 创建纹理图像

**代码实现**：
```cpp
VkImage textureImage;
VkDeviceMemory textureImageMemory;

void createTextureImage() {
    // 1. 加载图片数据（使用 stb_image）
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    
    // 2. 创建 Staging Buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    stbi_image_free(pixels);
    
    // 3. 创建纹理图像
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    vkCreateImage(device, &imageInfo, nullptr, &textureImage);
    
    // 4. 分配内存
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, textureImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory);
    vkBindImageMemory(device, textureImage, textureImageMemory, 0);
    
    // 5. 转换布局并拷贝
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
```

### 9.3 采样器与描述符

**代码实现**：
```cpp
VkSampler textureSampler;

void createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
}
```

---

## 第十章：深度测试

**难度**：进阶 | **优先级**：可跳过（初学）

### 10.1 核心概念：深度缓冲

**类比**：深度缓冲像距离标签。

**工作原理**：
```
每个像素存储深度值（0.0 ~ 1.0）
绘制时比较深度：
- 新深度 < 存储深度 → 更新颜色 + 深度
- 新深度 >= 存储深度 → 丢弃
```

### 10.2 创建深度图像

**代码实现**：
```cpp
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;

void createDepthResources() {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    vkCreateImage(device, &imageInfo, nullptr, &depthImage);
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    vkAllocateMemory(device, &allocInfo, nullptr, &depthImageMemory);
    vkBindImageMemory(device, depthImage, depthImageMemory, 0);
    
    createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);
}
```

### 10.3 配置深度测试

**代码实现**：
```cpp
// 在创建管线时启用深度测试
VkPipelineDepthStencilStateCreateInfo depthStencil{};
depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
depthStencil.depthTestEnable = VK_TRUE;
depthStencil.depthWriteEnable = VK_TRUE;
depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
depthStencil.depthBoundsTestEnable = VK_FALSE;
depthStencil.stencilTestEnable = VK_FALSE;
```

---

## 第十一章：帧同步

**难度**：进阶 | **优先级**：必学

### 11.1 核心概念：Fence 与 Semaphore

**类比**：
- **Fence**：CPU 等待 GPU（像等快递）
- **Semaphore**：GPU 队列间同步（像接力赛交接棒）

**同步流程**：
```
帧 N：
1. 等待 Fence（上一帧完成）
2. 获取图像（Semaphore：ImageAvailable）
3. 录制命令
4. 提交命令（等待 ImageAvailable，信号 RenderFinished）
5. 呈现（等待 RenderFinished）
6. 重置 Fence
```

### 11.2 同步原语

**代码实现**：
```cpp
const int MAX_FRAMES_IN_FLIGHT = 2;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
size_t currentFrame = 0;

void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }
}
```

### 11.3 完整的渲染循环

**代码实现**：
```cpp
void drawFrame() {
    // 1. 等待上一帧完成
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    // 2. 获取交换链图像
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    
    // 3. 重置 Fence
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    
    // 4. 录制命令
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    
    // 5. 提交命令
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
    
    // 6. 呈现
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    }
    
    // 7. 更新帧索引
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
```

**常见错误**：
1. **忘记等待 Fence**
   ```cpp
   // 错误：不等待直接提交
   vkQueueSubmit(queue, 1, &submitInfo, fence);  // 可能覆盖上一帧
   
   // 正确：等待 Fence
   vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
   vkResetFences(device, 1, &fence);
   vkQueueSubmit(queue, 1, &submitInfo, fence);
   ```

2. **Semaphore 未信号**
   ```cpp
   // 错误：等待未信号的 Semaphore
   submitInfo.pWaitSemaphores = &semaphore;  // 死锁！
   
   // 正确：确保 Semaphore 已信号
   // 由 vkAcquireNextImageKHR 或 vkQueueSubmit 信号
   ```

---

## 第十二章：从零到三角形的最短路径

### 12.1 最小化清单

**必须的对象**：
1. VkInstance
2. VkPhysicalDevice
3. VkDevice
4. VkQueue（图形队列）
5. VkSwapchain + VkImageViews
6. VkRenderPass + VkFramebuffers
7. VkPipeline + VkPipelineLayout
8. VkCommandPool + VkCommandBuffers
9. VkShaderModules（顶点 + 片元）
10. VkSemaphores + VkFences

**可以跳过的**：
- 顶点缓冲（硬编码顶点）
- 纹理
- 深度缓冲
- Uniform 缓冲
- 描述符集

### 12.2 完整代码

**最小三角形（约 500 行）**：

```cpp
// main.cpp
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>

struct Vertex {
    float pos[3];
    float color[3];
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
};

class HelloTriangleApplication {
    GLFWwindow* window;
    
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    
    uint32_t currentFrame = 0;
    
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
    
private:
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        window = glfwCreateWindow(800, 600, "Vulkan Triangle", nullptr, nullptr);
    }
    
    void initVulkan() {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }
    
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        
        vkDeviceWaitIdle(device);
    }
    
    void cleanup() {
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroyFence(device, inFlightFence, nullptr);
        
        vkDestroyCommandPool(device, commandPool, nullptr);
        
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        
        glfwDestroyWindow(window);
    }
    
    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }
    
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
    
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    
    bool isDeviceSuitable(VkPhysicalDevice device) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                return true;
            }
        }
        
        return false;
    }
    
    void createLogicalDevice() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = i;
            }
            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                presentFamily = i;
            }
            
            if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX) {
                break;
            }
        }
        
        float queuePriority = 1.0f;
        
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily, presentFamily};
        
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        VkPhysicalDeviceFeatures deviceFeatures{};
        
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        
        std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        
        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
    }
    
    void createSwapChain() {
        // ... 参考第四章 ...
    }
    
    void createImageViews() {
        // ... 参考第四章 ...
    }
    
    void createRenderPass() {
        // ... 参考第六章 ...
    }
    
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");
        
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
        
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
        
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        
        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
        
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }
    
    void createFramebuffers() {
        // ... 参考第六章 ...
    }
    
    void createCommandPool() {
        // ... 参考第五章 ...
    }
    
    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
        
        vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    }
    
    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence);
    }
    
    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence);
        
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
        
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &imageIndex;
        
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }
    
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        
        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        
        vkCmdEndRenderPass(commandBuffer);
        
        vkEndCommandBuffer(commandBuffer);
    }
    
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        VkShaderModule shaderModule;
        vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
        
        return shaderModule;
    }
    
    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        
        return buffer;
    }
};

int main() {
    HelloTriangleApplication app;
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
```

### 12.3 运行与调试

**编译**：
```bash
# CMake
mkdir build && cd build
cmake ..
make

# 运行
./VulkanTriangle
```

**调试**：
1. **黑屏**
   - 检查 Shader 是否编译成功
   - 检查顶点坐标是否在 [-1, 1] 范围内
   - 检查清除颜色是否为黑色

2. **验证层错误**
   ```bash
   # 启用验证层
   export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
   
   # 查看错误信息
   ./VulkanTriangle 2>&1 | grep "Validation"
   ```

3. **RenderDoc 调试**
   - 下载 RenderDoc
   - 启动程序时附加 RenderDoc
   - 捕获帧，查看每个 Pass 的输入输出

---

> **工程师的结语**：恭喜你！如果你能运行出这个三角形，你已经跨过了 Vulkan 最陡峭的学习曲线。接下来，你可以逐步添加顶点缓冲、纹理、深度测试、Uniform 缓冲等特性。记住，Vulkan 很难，但每前进一步，你对图形学的理解都会更深一层。加油！
