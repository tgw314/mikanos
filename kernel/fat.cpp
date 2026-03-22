#include "fat.hpp"

#include <cstdint>
#include <cstring>

namespace fat {
BPB *boot_volume_image;

void Initialize(void *volume_image) {
    boot_volume_image = reinterpret_cast<fat::BPB *>(volume_image);
}

uintptr_t GetClusterAddr(unsigned long cluster) {
    const auto img = boot_volume_image;
    unsigned long sector_num = img->reserved_sector_count +
                               img->num_fats * img->fat_size_32 +
                               (cluster - 2) * img->sectors_per_cluster;
    uintptr_t offset = sector_num * img->bytes_per_sector;

    return reinterpret_cast<uintptr_t>(boot_volume_image) + offset;
}

void ReadName(const DirectoryEntry &entry, char *base, char *ext) {
    memcpy(base, &entry.name[0], 8);
    base[8] = 0;
    for (int i = 7; i >= 0 && base[i] == 0x20; i--) base[i] = 0;

    memcpy(ext, &entry.name[8], 3);
    ext[3] = 0;
    for (int i = 2; i >= 0 && ext[i] == 0x20; i--) ext[i] = 0;
}
}  // namespace fat
