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
#define FDT_ALIGN(x, a)         (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_UP(addr, align)	((addr + (align - 1)) & ~((align - 1)))

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

/* fdt_get_next_node(struct fdt_node *curr); */
/* fdt_get_root_node(addr_t fdt_base); */
/* fdt_get_node(const char *name, addr_t fdt_base) */
    /* if strlen(name) == 0, root node */
/* fdt_get_property(struct fdt_node *, char *property); */
/**
 * fdt_convert_endian
 * 
 * converts the flattened device tree into little endian
 * @dt_base	base address of the device tree
 * @return errno
 **/
inline int fdt_convert_endian(addr_t fdt_base) {
    struct fdt_header	*head		= NULL;
    struct fdt_node	*node		= NULL;
    int			ret 		= ESUCC;
    fdt32_t		tag		= 0x0;
    addr_t		struct_off	= 0x0;
    addr_t		string_off	= 0x0;
    size_t		t_size		= 0;
    
    /* ensure it's actually fdt */
    if (is_using_fdt(fdt_base)) {
	memconvle32((void *)fdt_base, sizeof(struct fdt_header)); /* convert the fdt_header */
	
	/* assign & calculate */
	head 		= (struct fdt_header *)fdt_base;
	struct_off	= fdt_base + head->dt_struct_offset;
	string_off	= fdt_base + head->dt_strings_offset;
	node		= (struct fdt_node *)struct_off;
	
	char tabs[16];
	
	do {
	    tag	= be32_to_cpu(node->tag);
	    
	    if (tag == FDT_BEGIN_NODE) {
		size_t len = strlen(tabs);
		t_size = (sizeof(struct fdt_node) + (strlen(node->data) + 1));
		
		if (strlen(node->data) > 0) {
		    mach_init_printf("%s%s {\n", tabs, node->data);
		    tabs[len] = '\t';
		    tabs[len + 1] = '\0';
		} else {
		    mach_init_printf("{\n");
		    tabs[len] = '\t';
		    tabs[len + 1] = '\0';
		}
		
		//mach_init_printf("FDT_BEGIN_NODE 0x%x, node->data: %s\n", (addr_t)node - fdt_base, node->data);
	    } else if (tag == FDT_PROP) {
		struct fdt_property *prop = (struct fdt_property *)node;
		unsigned int *ptr = (unsigned int *)((addr_t)prop + sizeof(struct fdt_property));
		char *str = NULL;
		
		/* convert */
		memconvle32(prop, sizeof(struct fdt_property));

		str = (char *)string_off + prop->name_offset;
		
		if (strcmp(str, "#address-cells") == 0) {
		    mach_init_printf("%s%s = <0x%x>\n", tabs, str, be32_to_cpu(*ptr));
		} else if (strcmp(str, "#size-cells") == 0) {
		    mach_init_printf("%s%s = <0x%x>\n", tabs, str, be32_to_cpu(*ptr));
		} else if (strcmp(str, "reg") == 0) {
		    mach_init_printf("%s %s = <0x%x 0x%x>\n", tabs, str, be32_to_cpu(*ptr), be32_to_cpu(*ptr + 1));
		} else {
		    mach_init_printf("%s%s = %s\n", tabs, str, prop->data);
		}
		
		mach_init_printf("prop->len: %i\n", prop->length);
		
		//mach_init_printf("FDT_PROP 0x%x, prop->length %i, prop->offset: 0x%x\n", (addr_t)node - fdt_base, prop->length, (prop->name_offset + head->dt_strings_offset));
		//mach_init_printf("%s -> %s\n", str, prop->data);
		
		t_size = sizeof(struct fdt_property) + prop->length;
	    } else if (tag == FDT_END_NODE || tag == FDT_END || tag == FDT_NOP) {
		t_size = sizeof(struct fdt_node);
		

		if (tag == FDT_END_NODE) {
		    tabs[strlen(tabs) - 1] = '\0';
		    //mach_init_printf("FDT_END_NODE 0x%x\n", (addr_t)node - fdt_base);
		    mach_init_printf("%s};\n", tabs);
		} else if (tag == FDT_NOP) {
		    //mach_init_printf("FDT_NOP 0x%x\n", (addr_t)node - fdt_base);
		} else {
		    //mach_init_printf("FDT_END 0x%x\n", (addr_t)node - fdt_base);
		}
	    } else {	/* invalid tag */
		mach_init_printf("fdt_convert_endian - invalid tag 0x%x\n", tag);
		ret = EINVAL;
		break;
	    }

	    /* convert tag */

	    node->tag 	= tag;
	    node 	= (struct fdt_node *)ALIGN_UP((addr_t)node + t_size, 4);
	} while (tag != FDT_END);
    } else {
	ret = EINVAL;
    }
    
    return ret;
}




#endif
