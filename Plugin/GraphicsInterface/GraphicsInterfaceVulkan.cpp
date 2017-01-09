#include "pch.h"
#include "giInternal.h"

#ifdef giSupportVulkan
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

namespace gi {

template<class T> class unique_handle;

#define Def(Type, Deleter)\
    template<> class unique_handle<Type>\
    {\
    public:\
        unique_handle(VkDevice dev, Type h = nullptr) : m_device(dev), m_handle(h) {}\
        ~unique_handle() { if (m_handle) { Deleter(m_device, m_handle, nullptr); } }\
        void detach() { m_handle = nullptr; }\
        Type get() { return m_handle; }\
        Type* addr() { return &m_handle; }\
        Type& ref() { return m_handle; }\
    private:\
        VkDevice m_device;\
        Type m_handle;\
    };

Def(VkDeviceMemory, vkFreeMemory)
Def(VkBuffer, vkDestroyBuffer)
Def(VkImage, vkDestroyImage)
Def(VkCommandPool, vkDestroyCommandPool);

#undef Def

template<> class unique_handle<VkCommandBuffer>
{
public:
    unique_handle(VkDevice dev, VkCommandPool pool, VkCommandBuffer h = nullptr) : m_device(dev), m_pool(pool), m_handle(h) {}
    ~unique_handle() { if (m_handle) { vkFreeCommandBuffers(m_device, m_pool, 1, &m_handle); } }
    void detach() { m_handle = nullptr; }
    VkCommandBuffer get() { return m_handle; }
    VkCommandBuffer* addr() { return &m_handle; }
    VkCommandBuffer& ref() { return m_handle; }
private:
    VkDevice m_device;
    VkCommandPool m_pool;
    VkCommandBuffer m_handle;
};



class GraphicsInterfaceVulkan : public GraphicsInterface
{
public:
    GraphicsInterfaceVulkan(void *device);
    ~GraphicsInterfaceVulkan() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void   releaseTexture2D(void *tex) override;
    Result readTexture2D(void *dst, size_t read_size, void *tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void   releaseBuffer(void *buf) override;
    Result readBuffer(void *dst, void *src_buf, size_t read_size, BufferType type) override;
    Result writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

    enum class StagingFlag {
        Upload,
        Readback,
    };
    VkResult createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);
    VkResult createStagingBuffer(VkBuffer target, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);
    VkResult createStagingBuffer(VkImage target, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);

    // Body: [](void *mapped_memory) -> void
    template<class Body> VkResult map(VkDeviceMemory device_memory, const Body& body);

    // Body: [](VkCommandBuffer clist) -> void
    template<class Body> VkResult executeCommands(const Body& body);

private:
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_device = nullptr;
    VkQueue m_cqueue = nullptr;
    unique_handle<VkCommandPool> m_cpool = unique_handle<VkCommandPool>(nullptr);

    VkPhysicalDeviceMemoryProperties m_memory_properties;
};


GraphicsInterface* CreateGraphicsInterfaceVulkan(void *device)
{
    return new GraphicsInterfaceVulkan(device);
}

struct VulkanParams
{
    VkPhysicalDevice physical_device;
    VkDevice device;
};


GraphicsInterfaceVulkan::GraphicsInterfaceVulkan(void *device)
{
    auto& vkparams = *(VulkanParams*)device;
    m_physical_device = vkparams.physical_device;
    m_device = vkparams.device;

    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

    uint32_t graphicsQueueIndex = 0;
    vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &m_cqueue);

    m_cpool = unique_handle<VkCommandPool>(m_device);
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = 0;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device, &info, nullptr, m_cpool.addr());
}

GraphicsInterfaceVulkan::~GraphicsInterfaceVulkan()
{
}

void GraphicsInterfaceVulkan::release()
{
    delete this;
}

void* GraphicsInterfaceVulkan::getDevicePtr()
{
    return nullptr;
}

DeviceType GraphicsInterfaceVulkan::getDeviceType()
{
    return DeviceType::Vulkan;
}

static Result TranslateReturnCode(VkResult vr)
{
    switch (vr) {
    case VK_SUCCESS: return Result::OK;
    case VK_ERROR_OUT_OF_HOST_MEMORY: return Result::OutOfMemory;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return Result::OutOfMemory;
    }
    return Result::Unknown;
}

uint32_t GraphicsInterfaceVulkan::getMemoryType(uint32_t type_bits, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < 32; i++) {
        if ((type_bits & 1) == 1) {
            if ((m_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        type_bits >>= 1;
    }

    return 0;
}

VkResult GraphicsInterfaceVulkan::createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkBufferCreateInfo buf_info = {};
    buf_info.sType      = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size       = size;
    buf_info.sharingMode= VK_SHARING_MODE_EXCLUSIVE;
    buf_info.usage      = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (flag == StagingFlag::Upload) {
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (flag == StagingFlag::Readback) {
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    auto vr = vkCreateBuffer(m_device, &buf_info, nullptr, &buffer);
    if (vr != VK_SUCCESS) { return vr; }

    VkMemoryRequirements mem_required = {};
    vkGetBufferMemoryRequirements(m_device, buffer, &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return vr; }

    vr = vkBindBufferMemory(m_device, buffer, memory, 0);
    if (vr != VK_SUCCESS) { return vr; }

    return VK_SUCCESS;
}

VkResult GraphicsInterfaceVulkan::createStagingBuffer(VkBuffer target, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkMemoryRequirements mem_required;
    vkGetBufferMemoryRequirements(m_device, target, &mem_required);
    return createStagingBuffer(mem_required.size, flag, buffer, memory);
}

VkResult GraphicsInterfaceVulkan::createStagingBuffer(VkImage target, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkMemoryRequirements mem_required;
    vkGetImageMemoryRequirements(m_device, target, &mem_required);
    return createStagingBuffer(mem_required.size, flag, buffer, memory);
}

template<class Body>
VkResult GraphicsInterfaceVulkan::map(VkDeviceMemory device_memory, const Body& body)
{
    void *mapped_memory = nullptr;
    auto vr = vkMapMemory(m_device, device_memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
    if (vr != VK_SUCCESS) { return vr; }

    body(mapped_memory);

    vkUnmapMemory(m_device, device_memory);

    VkMappedMemoryRange mapped_range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE , nullptr, device_memory , 0, VK_WHOLE_SIZE };
    vr = vkFlushMappedMemoryRanges(m_device, 1, &mapped_range);
    if (vr != VK_SUCCESS) { return vr; }

    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsInterfaceVulkan::executeCommands(const Body& body)
{
    auto clist = unique_handle<VkCommandBuffer>(m_device, m_cpool.get());

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = m_cpool.get();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    auto vr = vkAllocateCommandBuffers(m_device, &alloc_info, clist.addr());
    if (vr != VK_SUCCESS) { return vr; }

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vr = vkBeginCommandBuffer(clist.get(), &begin_info);
    if (vr != VK_SUCCESS) { return vr; }

    body(clist.get());

    vr = vkEndCommandBuffer(clist.get());
    if (vr != VK_SUCCESS) { return vr; }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = clist.addr();
    vr = vkQueueSubmit(m_cqueue, 1, &submit_info, VK_NULL_HANDLE);
    if (vr != VK_SUCCESS) { return vr; }

    vr = vkQueueWaitIdle(m_cqueue);
    if (vr != VK_SUCCESS) { return vr; }

    return VK_SUCCESS;
}



void GraphicsInterfaceVulkan::sync()
{
    auto vr = vkQueueWaitIdle(m_cqueue);
}


static VkFormat TranslateFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Ru8:     return VK_FORMAT_R8_UNORM;
    case TextureFormat::RGu8:    return VK_FORMAT_R8G8_UNORM;
    case TextureFormat::RGBAu8:  return VK_FORMAT_R8G8B8A8_UNORM;

    case TextureFormat::Rf16:    return VK_FORMAT_R16_SFLOAT;
    case TextureFormat::RGf16:   return VK_FORMAT_R16G16_SFLOAT;
    case TextureFormat::RGBAf16: return VK_FORMAT_R16G16B16A16_SFLOAT;

    case TextureFormat::Ri16:    return VK_FORMAT_R16_SNORM;
    case TextureFormat::RGi16:   return VK_FORMAT_R16G16_SNORM;
    case TextureFormat::RGBAi16: return VK_FORMAT_R16G16B16A16_SNORM;

    case TextureFormat::Rf32:    return VK_FORMAT_R32_SFLOAT;
    case TextureFormat::RGf32:   return VK_FORMAT_R32G32_SFLOAT;
    case TextureFormat::RGBAf32: return VK_FORMAT_R32G32B32A32_SFLOAT;

    case TextureFormat::Ri32:    return VK_FORMAT_R32_SINT;
    case TextureFormat::RGi32:   return VK_FORMAT_R32G32_SINT;
    case TextureFormat::RGBAi32: return VK_FORMAT_R32G32B32A32_SINT;
    }
    return VK_FORMAT_UNDEFINED;
}

Result GraphicsInterfaceVulkan::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    if (!dst_tex) { return Result::InvalidParameter; }

    auto ret = unique_handle<VkImage>(m_device);
    auto memory = unique_handle<VkDeviceMemory>(m_device);

    VkImageCreateInfo image_info = {};
    image_info.sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType      = VK_IMAGE_TYPE_2D;
    image_info.format         = TranslateFormat(format);
    image_info.mipLevels      = 1;
    image_info.arrayLayers    = 1;
    image_info.samples        = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling         = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage          = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout  = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_info.extent         = { (uint32_t)width, (uint32_t)height, 1 };
    image_info.usage          = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.flags          = 0;
    auto vr = vkCreateImage(m_device, &image_info, nullptr, ret.addr());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    VkMemoryRequirements mem_required = {};
    vkGetImageMemoryRequirements(m_device, ret.get(), &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, memory.addr());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindImageMemory(m_device, ret.get(), memory.get(), 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    *dst_tex = ret.get();

    if (data) {
        writeTexture2D(ret.get(), width, height, format, data, width * height * GetTexelSize(format));
    }

    ret.detach();
    memory.detach();

    return Result::OK;
}

void GraphicsInterfaceVulkan::releaseTexture2D(void *tex_)
{
    if (!tex_) { return; }

    auto tex = (VkImage)tex_;
    vkDestroyImage(m_device, tex, nullptr);
}

Result GraphicsInterfaceVulkan::readTexture2D(void *dst, size_t read_size, void *src_tex_, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_tex_) { return Result::InvalidParameter; }

    auto src_tex = (VkImage)src_tex_;

    auto staging_buffer = unique_handle<VkBuffer>(m_device);
    auto staging_memory = unique_handle<VkDeviceMemory>(m_device);
    auto vr = createStagingBuffer(src_tex, StagingFlag::Readback, staging_buffer.ref(), staging_memory.ref());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = executeCommands([&](VkCommandBuffer clist) {
        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyImageToBuffer(clist, src_tex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buffer.get(), 1, &region);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = map(staging_memory.get(), [&](void *mapped_memory) {
        memcpy(dst, mapped_memory, read_size);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    return Result::OK;
}

Result GraphicsInterfaceVulkan::writeTexture2D(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_tex_ || !src) { return Result::InvalidParameter; }

    auto dst_tex = (VkImage)dst_tex_;

    auto staging_buffer = unique_handle<VkBuffer>(m_device);
    auto staging_memory = unique_handle<VkDeviceMemory>(m_device);
    auto vr = createStagingBuffer(dst_tex, StagingFlag::Upload, staging_buffer.ref(), staging_memory.ref());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = map(staging_memory.get(), [&](void *mapped_memory) {
        memcpy(mapped_memory, src, write_size);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = executeCommands([&](VkCommandBuffer clist) {
        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(clist, staging_buffer.get(), dst_tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    return Result::OK;
}


Result GraphicsInterfaceVulkan::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    if (!dst_buf) { return Result::InvalidParameter; }

    auto ret = unique_handle<VkBuffer>(m_device);
    auto memory = unique_handle<VkDeviceMemory>(m_device);

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = size;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (type) {
    case BufferType::Index:
        buf_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferType::Vertex:
        buf_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case BufferType::Constant:
        buf_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case BufferType::Compute:
        buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    auto vr = vkCreateBuffer(m_device, &buf_info, nullptr, ret.addr());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    VkMemoryRequirements mem_required = {};
    vkGetBufferMemoryRequirements(m_device, ret.get(), &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, memory.addr());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindBufferMemory(m_device, ret.get(), memory.get(), 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    *dst_buf = ret.get();

    if (data) {
        writeBuffer(ret.get(), data, size, type);
    }

    ret.detach();
    memory.detach();

    return Result::OK;
}

void GraphicsInterfaceVulkan::releaseBuffer(void *buf_)
{
    if (!buf_) { return; }

    auto buf = (VkBuffer)buf_;
    vkDestroyBuffer(m_device, buf, nullptr);
}

Result GraphicsInterfaceVulkan::readBuffer(void *dst, void *src_buf, size_t read_size, BufferType type)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_buf) { return Result::InvalidParameter; }

    auto buf = (VkBuffer)src_buf;

    auto staging_buffer = unique_handle<VkBuffer>(m_device);
    auto staging_memory = unique_handle<VkDeviceMemory>(m_device);
    auto vr = createStagingBuffer(buf, StagingFlag::Readback, staging_buffer.ref(), staging_memory.ref());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = executeCommands([&](VkCommandBuffer clist) {
        VkBufferCopy region = { 0, 0, read_size };
        vkCmdCopyBuffer(clist, buf, staging_buffer.get(), 1, &region);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = map(staging_memory.get(), [&](void *mapped_memory) {
        memcpy(dst, mapped_memory, read_size);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    return Result::OK;
}

Result GraphicsInterfaceVulkan::writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_buf || !src) { return Result::InvalidParameter; }

    auto buf = (VkBuffer)dst_buf;

    auto staging_buffer = unique_handle<VkBuffer>(m_device);
    auto staging_memory = unique_handle<VkDeviceMemory>(m_device);
    auto vr = createStagingBuffer(buf, StagingFlag::Upload, staging_buffer.ref(), staging_memory.ref());
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = map(staging_memory.get(), [&](void *mapped_memory) {
        memcpy(mapped_memory, src, write_size);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = executeCommands([&](VkCommandBuffer clist) {
        VkBufferCopy region = {0, 0, write_size };
        vkCmdCopyBuffer(clist, staging_buffer.get(), buf, 1, &region);
    });
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    return Result::OK;
}

} // namespace gi
#endif // giSupportVulkan
