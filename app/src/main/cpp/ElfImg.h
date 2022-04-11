#ifndef ELF_IMG_H
#define ELF_IMG_H

#include <string_view>
#include <unordered_map>
#include <linux/elf.h>
#include <sys/types.h>
#include <link.h>
#include <string>

#define SHT_GNU_HASH 0x6ffffff6

class ElfImg {
public:
    ElfImg(std::string_view elf);

    constexpr ElfW(Addr) getSymbolOffset(std::string_view name) const {
        return getSymbolOffset(name, GnuHash(name), ElfHash(name));
    }

    constexpr ElfW(Addr) getSymbolAddress(std::string_view name) const {
        ElfW(Addr) offset = getSymbolOffset(name);
        if (offset > 0 && base != nullptr) {
            return static_cast<ElfW(Addr)>((uintptr_t) base + offset - bias);
        } else {
            return 0;
        }
    }

    template<typename T>
    constexpr T getSymbolAddress(std::string_view name) const {
        return reinterpret_cast<T>(getSymbolAddress(name));
    }

    bool isValid() const {
        return base != nullptr;
    }

    const std::string name() const {
        return elf;
    }

    ~ElfImg();

private:
    ElfW(Addr) getSymbolOffset(std::string_view name, uint32_t gnu_hash, uint32_t elf_hash) const;

    ElfW(Addr) ElfLookup(std::string_view name, uint32_t hash) const;

    ElfW(Addr) GnuLookup(std::string_view name, uint32_t hash) const;

    ElfW(Addr) LinearLookup(std::string_view name) const;

    constexpr static uint32_t ElfHash(std::string_view name);

    constexpr static uint32_t GnuHash(std::string_view name);

    bool findModuleBase();

    std::string elf;
    void *base = nullptr;
    char *buffer = nullptr;
    off_t size = 0;
    off_t bias = -4396;
    ElfW(Ehdr) *header = nullptr;
    ElfW(Shdr) *section_header = nullptr;
    ElfW(Shdr) *symtab = nullptr;
    ElfW(Shdr) *strtab = nullptr;
    ElfW(Shdr) *dynsym = nullptr;
    ElfW(Sym) *symtab_start = nullptr;
    ElfW(Sym) *dynsym_start = nullptr;
    ElfW(Sym) *strtab_start = nullptr;
    ElfW(Off) symtab_count = 0;
    ElfW(Off) symstr_offset = 0;
    ElfW(Off) symstr_offset_for_symtab = 0;
    ElfW(Off) symtab_offset = 0;
    ElfW(Off) dynsym_offset = 0;
    ElfW(Off) symtab_size = 0;

    uint32_t nbucket_{};
    uint32_t *bucket_ = nullptr;
    uint32_t *chain_ = nullptr;

    uint32_t gnu_nbucket_{};
    uint32_t gnu_symndx_{};
    uint32_t gnu_bloom_size_;
    uint32_t gnu_shift2_;
    uintptr_t *gnu_bloom_filter_;
    uint32_t *gnu_bucket_;
    uint32_t *gnu_chain_;

    mutable std::unordered_map<std::string_view, ElfW(Sym) *> symtabs_;
};

constexpr uint32_t ElfImg::ElfHash(std::string_view name) {
    uint32_t h = 0, g = 0;
    for (unsigned char p: name) {
        h = (h << 4) + p;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

constexpr uint32_t ElfImg::GnuHash(std::string_view name) {
    uint32_t h = 5381;
    for (unsigned char p: name) {
        h += (h << 5) + p;
    }
    return h;
}

#endif