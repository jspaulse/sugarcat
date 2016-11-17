#ifndef ATAGS_H
#define ATAGS_H
#include <stdint.h>
#include <stdbool.h>
#include <types.h>
#include <mm/mem.h>

#define ATAG_ERR_SUCCESS		0
#define ATAG_ERR_INVALID_TAGS	1
#define ATAG_ERR_NOT_FOUND		2

enum tag_t {
	ATAG_NONE		= 0x00000000,
	ATAG_CORE		= 0x54410001,
	ATAG_MEM		= 0x54410002,
	ATAG_VIDEOTEXT	= 0x54410003,
	ATAG_RAMDISK	= 0x54410004,
	ATAG_INITRD2	= 0x54420005,
	ATAG_SERIAL		= 0x54410006,
	ATAG_REVISION	= 0x54410007,
	ATAG_VIDEOLFB	= 0x54410008,
	ATAG_CMDLINE	= 0x54410009
};

/*
#define ATAG_NONE		0x00
#define ATAG_CORE		0x54410001
#define ATAG_MEM		0x54410002
#define ATAG_VIDEOTEXT	0x54410003
#define ATAG_RAMDISK	0x54410004
#define ATAG_INITRD2	0x54420005
#define ATAG_SERIAL		0x54410006
#define ATAG_REVISION	0x54410007
#define ATAG_VIDEOLFB	0x54410008
#define ATAG_CMDLINE	0x54410009
*/

struct atag_hdr {
	uint32_t	size;
	uint32_t	tag;
};

struct atag_core {
	uint32_t	flags;
	uint32_t	pg_sz;
	uint32_t	root_dev;
};

struct atag_mem {
	uint32_t	sz;
	uint32_t	start;
};

struct atag_vidtext {
	uint8_t		x;
	uint8_t		y;
	uint16_t	vid_pg;
	uint8_t		vid_mode;
	uint8_t		vid_cols;
	uint16_t	vid_ega_bx;
	uint8_t		vid_lines;
	uint8_t		is_vga;
	uint16_t	vid_points;
};

struct atag_ramdisk {
	uint32_t 	flags;
	uint32_t	sz;
	uint32_t	start;
};

struct atag_initrd {
	uint32_t	start;
	uint32_t	sz;
};

struct atag_serial {
	uint32_t	low;
	uint32_t	high;
};

struct atag_revision {
	uint32_t	rev;
};

struct atag_vidlfb {
	uint16_t	lfb_width;
	uint16_t	lfb_height;
	uint16_t	lfb_depth;
	uint16_t	lfb_line_base;
	uint32_t	lfb_base;
	uint32_t	lfb_sz;
	uint8_t		red_sz;
	uint8_t		red_pos;
	uint8_t		green_sz;
	uint8_t		green_pos;
	uint8_t		blue_sz;
	uint8_t		blue_pos;
	uint8_t		resv_sz;
	uint8_t		rsvd_pos;
};

struct atag_cmdline {
	char cmd[1];
};

struct atag {
	struct atag_hdr header;
	union {
		struct atag_core		core;
		struct atag_mem			mem;
		struct atag_vidtext		video_text;
		struct atag_ramdisk		ramdisk;
		struct atag_initrd		initrd;
		struct atag_serial		serial;
		struct atag_revision	revis;
		struct atag_vidlfb		videolfb;
		struct atag_cmdline		cmdline;
	} u;
};

/**
 * next_tag
 * 
 * returns the next tag in the list
 * @curr_tag	current tag on list
 * @return next tag
 **/
inline struct atag *next_tag(struct atag *curr_tag) {
	return (struct atag *)((unsigned int *)curr_tag + curr_tag->header.size);
}

/**
 * tag_exists
 * 
 * returns whether or not specified tag exists in list
 * @atag_base	starting address of atag list
 * @tag			tag to search for
 * 
 * @return true if tag exists
 **/
inline bool tag_exists(addr_t atag_base, unsigned int tag) {
	struct atag *sch 	= (struct atag *)atag_base;
	bool ret 			= false;
	
	if (sch->header.tag == ATAG_CORE) {
		while (sch->header.tag != ATAG_NONE) {
			if (sch->header.tag == tag) {
				ret = true;
				break;
			}
			
			sch = next_tag(sch);
		}
	}
	
	return ret;
}

/**
 * is_tag_list_sane
 * 
 * determines if the tag list at base address is sane
 * @return true if sane
 **/
inline bool is_tag_list_sane(addr_t atag_base) {
	struct atag 	*sch	= (struct atag *)atag_base;
	bool 			ret 	= false;
	
	if (sch->header.tag == ATAG_CORE) {
		ret = true;
	}
	
	return ret;
}

/**
 * get_tag_count
 * 
 * returns the number of instances of specified tag
 * @atag_base	starting address of atag list
 * @tag			tag to search for
 * @return		number of instances of specified tag
 **/
inline int get_tag_count(addr_t atag_base, unsigned int tag) {
	struct atag *sch	= (struct atag *)atag_base;
	int ret 			= 0;
	
	if (sch->header.tag == ATAG_CORE) {
		while (sch->header.tag != ATAG_NONE) {
			if (sch->header.tag == tag) {
				ret++;
			}
			
			sch = next_tag(sch);
		}
	}
	
	return ret;
}

/**
 * get_next_tag
 * 
 * returns a pointer to the next found instance of a specified tag
 * 
 * this function is used in conjunction with get_tag() but can be 
 * used exclusively by setting sch = (struct atag *)atag_base
 * and using is_tag_list_sane()
 * 
 * @sch		pointer to previous instance of specified tag
 * @tag		specified tag to search for
 * @return a pointer to the next found instance of a specified tag or null if not found
 **/ 
inline struct atag *get_next_tag(struct atag *sch, enum tag_t tag) {
	struct atag *ret = NULL;
	
	if (sch != NULL) {
		if (sch->header.tag != ATAG_NONE) {
			sch = next_tag(sch);
			
			while (sch->header.tag != ATAG_NONE) {
				if (sch->header.tag == tag) {
					ret = sch;
					break;
				}
				
				sch = next_tag(sch);
			}
		}
	}
	
	return ret;
}
		
/**
 * get_tag
 * 
 * returns a pointer to the first found instance of a specified tag
 * 
 * @atag_base	starting address of atag list
 * @tag			tag to search for
 * 
 * @return a pointer to the first instance of the tag, or null if not found
 **/
inline struct atag *get_tag(addr_t atag_base, enum tag_t tag) {
	struct atag *sch 	= (struct atag *)atag_base;
	struct atag *ret	= NULL;
	
	if (sch->header.tag == ATAG_CORE) {
		sch = next_tag(sch);
		
		while (sch->header.tag != ATAG_NONE) {
			if (sch->header.tag == tag) {
				ret = sch;
				break;
			}
			
			sch = next_tag(sch);
		}
	}
	
	return ret;
}

/** TMP **/

extern void mach_init_printf(const char *format, ...);

inline void dump_tags(addr_t atag_base) {
	struct atag *sch = (struct atag *)atag_base;
	
	if (sch->header.tag == ATAG_CORE) {
		sch = next_tag(sch);
		
		while (sch->header.tag != ATAG_NONE) {
			switch(sch->header.tag) {
				case ATAG_MEM:
					mach_init_printf("ATAG_MEM:\n");
					mach_init_printf("start 0x%x, size: %i\n\n", sch->u.mem.start, sch->u.mem.sz);
					break;
				case ATAG_VIDEOTEXT:
					mach_init_printf("ATAG_VIDEOTEXT:\n");
					mach_init_printf("(x,y): (%i, %i)\n\n", sch->u.video_text.x, sch->u.video_text.y);
					break;
				case ATAG_RAMDISK:
					mach_init_printf("ATAG_RAMDISK:\n");
					mach_init_printf("start 0x%x, size %i\n\n", sch->u.ramdisk.start, sch->u.ramdisk.sz);
					break;
				case ATAG_INITRD2:
					mach_init_printf("ATAG_INITRD:\n");
					mach_init_printf("start 0x%x, size %i\n\n", sch->u.initrd.start, sch->u.initrd.sz);
					break;
				case ATAG_SERIAL:
					mach_init_printf("ATAG_SERIAL:\n");
					mach_init_printf("low:high 0x%x:0x%x\n\n", sch->u.serial.low, sch->u.serial.high);
					break;
				case ATAG_REVISION:
					mach_init_printf("ATAG_REVISION:\n");
					mach_init_printf("revision: 0x%x\n", sch->u.revis.rev);
					break;
				case ATAG_VIDEOLFB:
					mach_init_printf("ATAG_VIDEOLFB:\n\n");
					break;
				case ATAG_CMDLINE:
					mach_init_printf("ATAG_CMDLINE:\n");
					mach_init_printf("size: %i\n", sch->header.size);
					break;
			}
			
			sch = next_tag(sch);
		}
	}
}


#endif
