#ifndef FDT_H
#define FDT_H
#include <util/bits.h>
#include <mm/mem.h>
#include <stdint.h>
#include <types.h>

typedef	uint32_t fdt32_t;
typedef uint64_t fdt64_t;

#define FDT_HEADER_MAGIC	0xD00DFEED
#define FDT_BEGIN_NODE		0x1
#define FDT_END_NODE		0x2
#define FDT_PROP		0x3
#define FDT_NOP			0x4
#define FDT_END			0x9

/**
 * fdt_header
 * 
 * defines a flattened device tree header
 * @magic		should contain 0xd00dfeed
 * @total_sz		total size of device tree data structure
 * @dt_struct_offset	offset to structure block
 * @dt_strings_offset	offset to strings block
 * @mem_resv_map_offset	offset to mem. reservation block
 * @version		version of device tree
 * @last_compat_version	last compatible version
 * @boot_cpuid_phys	physical id of boot cpu
 * @dt_strings_sz	size of the strings block
 * @dt_struct_sz	size of the structure block
 **/
struct fdt_header {
    fdt32_t	magic;			/* should be 0xd00dfeed */
    fdt32_t	total_sz;		/* total size of dt data structure */
    fdt32_t	dt_struct_offset;	/* offset to structure block */
    fdt32_t	dt_strings_offset;	/* offset to strings block */
    fdt32_t	mem_resv_map_offset;	/* offset to mem. reservation block */
    fdt32_t	version;		/* current version */
    fdt32_t	last_compat_version;	/* last compatible version */
    fdt32_t	boot_cpuid_phys;	/* physical id of boot cpu */
    fdt32_t	dt_strings_sz;		/* size of strings block */
    fdt32_t	dt_struct_sz;		/* size of structure block */
} __attribute__((packed));

/**
 * fdt_reserve_entry
 * 
 * defines a flattened device tree reserve entry
 * @address	address of reserved memory
 * @sz		size of reserved memory
 **/
struct fdt_reserve_entry {
    fdt64_t	address;
    fdt64_t	size;
} __attribute__((packed));

struct fdt_node {
    fdt32_t	tag;
    char	data[];
} __attribute__((packed));

struct fdt_property {
    fdt32_t	tag;
    fdt32_t	length;
    fdt32_t	name_offset;
    char	data[];		
} __attribute__((packed));

/**
 * is_using_fdt
 * 
 * determines whether or not the kernel is using
 * device trees.
 * @fdt_base	base address provided during init.
 * @return true if using device trees
 **/
inline bool is_using_fdt(addr_t fdt_base) {
    struct fdt_header 	*hdr 	= (struct fdt_header *)fdt_base;
    bool 		ret	= false;
    
    if (be32_to_cpu(hdr->magic) == FDT_HEADER_MAGIC) {
	ret = true;
    }
    
    return ret;
}

/* fdt.c */
struct fdt_property *fdt_get_property(addr_t fdt_base, struct fdt_node *node, const char *property);
struct fdt_property *fdt_get_next_property(struct fdt_property *prop);
struct fdt_node *fdt_get_node(const char *name, addr_t fdt_base);
struct fdt_node *fdt_get_next_node(struct fdt_node *node);
struct fdt_node *fdt_get_root_node(addr_t fdt_base);
fdt32_t fdt_get_cell_size(addr_t fdt_base, struct fdt_node *node);
void dump_fdt(addr_t fdt_base); /* todo - tmp */

#endif
