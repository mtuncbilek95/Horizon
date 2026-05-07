# RenderBatch System — Vulkan Secondary Command Buffer Toplama Mimarisi

## Amaç

`RenderSystem::OnSync()` fonksiyonu **sadece** swapchain yönetimi, toplama ve present ile ilgilensin. Diğer sistemler (Mesh, Particle, UI vs.) kendi secondary command buffer'larını kaydedip bir batch'e atsın. `OnSync` çağrıldığında ne varsa toplasın, primary CB içinde execute etsin ve present etsin.

---

## Genel Akış

```
[ MeshSystem ]──────┐
[ ParticleSystem ]───┤──▶ RenderBatch ──▶ OnSync() ──▶ Present
[ UISystem ]─────────┘
```

1. Her sistem kendi secondary CB'sini kaydeder
2. `RenderBatch::Submit()` ile batch'e atar
3. `OnSync()` → `Flush()` ile hepsini toplar
4. Primary CB içinde `ExecuteCommands` ile çalıştırır
5. Present eder

---

## RenderBatch Sınıfı

```cpp
#pragma once
#include <vector>
#include <mutex>

class RenderBatch
{
public:
    // Herhangi bir sistem secondary CB kaydettikten sonra bunu çağırır.
    // Thread-safe.
    void Submit(CommandBuffer* secondaryCB, u32 layer = 0)
    {
        std::lock_guard lock(m_mutex);
        m_entries.push_back({ secondaryCB, layer });
    }

    // OnSync tarafından çağrılır. Tüm birikmiş CB'leri sıralayıp döner.
    // Çağrıldıktan sonra batch boşalır.
    std::vector<CommandBuffer*> Flush()
    {
        std::lock_guard lock(m_mutex);

        // Layer'a göre sırala (küçük layer önce çizilir)
        std::sort(m_entries.begin(), m_entries.end(),
            [](const Entry& a, const Entry& b) {
                return a.layer < b.layer;
            });

        std::vector<CommandBuffer*> result;
        result.reserve(m_entries.size());
        for (auto& entry : m_entries)
            result.push_back(entry.cb);

        m_entries.clear();
        return result;
    }

private:
    struct Entry
    {
        CommandBuffer* cb;
        u32 layer;
    };

    std::mutex          m_mutex;
    std::vector<Entry>  m_entries;
};
```

### Layer Değerleri (Öneri)

| Layer | Kullanım |
|-------|----------|
| 0     | Skybox   |
| 100   | Opaque Geometry |
| 200   | Transparent Geometry |
| 300   | Particles |
| 400   | Post-Process |
| 500   | UI / HUD |

---

## OnSync — Güncellenmiş Hali

```cpp
void RenderSystem::OnSync()
{
    // 1. Önceki frame'in bitmesini bekle
    m_fences[m_frameIndex]->WaitIdle();
    m_fences[m_frameIndex]->Reset();

    // 2. Swapchain'den image al
    u32 imageIndex = m_swapchain->AcquireNextImage(
        m_imageAvailableSemaphores[m_frameIndex].get(), nullptr);
    auto* swapImage = m_swapchain->Image(imageIndex);

    // 3. Batch'teki tüm secondary CB'leri topla
    auto secondaries = m_renderBatch.Flush();

    // 4. Primary CB'yi kaydet
    m_cmdBuffers[m_frameIndex]->BeginRecord();

    m_cmdBuffers[m_frameIndex]->ImageBarrier(ImageBarrierDesc()
        .setImage(swapImage)
        .setOldLayout(ImageLayout::Undefined)
        .setNewLayout(ImageLayout::ColorAttachmentOptimal)
        .setAspect(ImageAspect::Color));

    m_cmdBuffers[m_frameIndex]->BeginRendering(RenderingInfo()
        .setRenderAreaExtent(m_swapchain->ImageSize())
        .addColorAttachment(RenderingAttachmentInfo()
            .setImageView(m_swapchain->ImageView(imageIndex))
            .setImageLayout(ImageLayout::ColorAttachmentOptimal)
            .setLoadOp(AttachmentLoad::Clear)
            .setStoreOp(AttachmentStore::Store)
            .setClearValue(ClearValue().setColor({ 0.f, 0.f, 0.f, 1.f })))
        .setDepthAttachment(nullptr));

    // 5. Toplanan secondary CB'leri execute et
    if (!secondaries.empty())
        m_cmdBuffers[m_frameIndex]->ExecuteCommands(secondaries);

    m_cmdBuffers[m_frameIndex]->EndRendering();

    m_cmdBuffers[m_frameIndex]->ImageBarrier(ImageBarrierDesc()
        .setImage(swapImage)
        .setOldLayout(ImageLayout::ColorAttachmentOptimal)
        .setNewLayout(ImageLayout::PresentSrcKHR)
        .setAspect(ImageAspect::Color));

    m_cmdBuffers[m_frameIndex]->EndRecord();

    // 6. GPU'ya gönder
    m_graphicsQueue->Submit(
        { m_cmdBuffers[m_frameIndex].get() },
        { m_imageAvailableSemaphores[m_frameIndex].get() },
        { m_renderFinishedSemaphores[imageIndex].get() },
        m_fences[m_frameIndex].get(),
        PipelineStageFlags::ColorAttachment);

    // 7. Present
    m_swapchain->Present({ m_renderFinishedSemaphores[imageIndex].get() });

    m_frameIndex = (m_frameIndex + 1) % MaxFrameInFlight;
}
```

---

## Diğer Sistemler Nasıl Kullanır

### Secondary CB Kaydetme Kuralları

- Her sistem kendi `VkCommandPool`'unu kullanmalı
- Secondary CB `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT` ile başlamalı
- Dynamic rendering kullanıyorsan `VkCommandBufferInheritanceRenderingInfo` gerekli

### Örnek: MeshSystem

```cpp
void MeshSystem::OnSync()
{
    // Secondary CB al (kendi pool'undan)
    auto* cb = m_commandPool->AllocateSecondary();

    cb->BeginRecord(CommandBufferUsage::RenderPassContinue,
        InheritanceInfo()
            .setColorFormats({ m_swapchainFormat })
            .setDepthFormat(Format::Undefined));

    cb->BindPipeline(m_meshPipeline);
    cb->BindVertexBuffer(m_vertexBuffer);
    cb->BindIndexBuffer(m_indexBuffer);

    for (auto& mesh : m_visibleMeshes)
    {
        cb->PushConstants(mesh.transform);
        cb->DrawIndexed(mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
    }

    cb->EndRecord();

    // Batch'e at — layer 100 = opaque geometry
    m_renderSystem->GetBatch().Submit(cb, 100);
}
```

### Örnek: UISystem

```cpp
void UISystem::OnSync()
{
    auto* cb = m_commandPool->AllocateSecondary();

    cb->BeginRecord(CommandBufferUsage::RenderPassContinue,
        InheritanceInfo()
            .setColorFormats({ m_swapchainFormat })
            .setDepthFormat(Format::Undefined));

    cb->BindPipeline(m_uiPipeline);
    // ... UI draw calls ...
    cb->EndRecord();

    // Layer 500 = UI en üstte çizilir
    m_renderSystem->GetBatch().Submit(cb, 500);
}
```

---

## Çalışma Sırası Garantisi

Tüm sistemlerin `OnSync()`'i **RenderSystem::OnSync()**'den **önce** çağrılmalı. ECS veya sistem yöneticisinde sıralama:

```
1. MeshSystem::OnSync()       → secondary CB kaydeder, batch'e atar
2. ParticleSystem::OnSync()   → secondary CB kaydeder, batch'e atar
3. UISystem::OnSync()         → secondary CB kaydeder, batch'e atar
4. RenderSystem::OnSync()     → Flush + Execute + Present
```

Single-threaded akışta bu sıralama yeterli, mutex bile gereksiz olur. Mutex sadece multithread senaryosu için güvenlik sağlar.

---

## Command Pool Yönetimi

Her sistem kendi command pool'una sahip olmalı. Pool'lar `RESET_COMMAND_BUFFER_BIT` ile oluşturulmalı ki secondary CB'ler frame başına resetlenebilsin.

```cpp
// Sistem başlatılırken
m_commandPool = device->CreateCommandPool(
    CommandPoolDesc()
        .setQueueFamily(graphicsQueueFamily)
        .setFlags(CommandPoolFlags::ResetCommandBuffer));

// Her frame başında (OnSync'in en başında)
m_commandPool->Reset(); // veya individual CB reset
```

---

## Özet

| Bileşen | Sorumluluk |
|---------|-----------|
| `RenderBatch` | Secondary CB'leri toplar ve sıralar |
| `RenderSystem::OnSync()` | Swapchain acquire, Flush, Execute, Present |
| Diğer sistemler | Secondary CB kaydeder, `Submit()` ile batch'e atar |
| Layer sistemi | Çizim sırasını belirler (düşük → önce) |