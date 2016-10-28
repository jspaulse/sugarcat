#ifndef ATAGS_H
#define ATAGS_H
#include <stdint.h>
#include <mm/mem.h>
#define ATAG_ERR_TAG_NOT_FOUND	-1
#define ATAG_ERR_SUCCESS		0
#define ATAG_ERR_INVALID_TAGS	1


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

#define next_tag(t) ((struct tag *)((unsigned int *)(t) + (t)->header.size))
		
inline int find_tag(unsigned int atag_base, unsigned int tag, struct atag *out_tag) {
	int ret = ATAG_ERR_TAG_NOT_FOUND;
	struct atag *sch = (struct atag *)atag_base;

	/* verify that the first tag is ATAG_CORE */
	if (sch->header.tag != ATAG_CORE) {
		ret = ATAG_ERR_INVALID_TAGS;
	} else {
		while (sch->header.tag != ATAG_NONE) {
			if (sch->header.tag == tag) {
				memcpy(out_tag, sch, sizeof(struct atag));
				ret = ATAG_ERR_SUCCESS;
				break;
			}
			
			sch = (struct atag *)next_tag(sch);
		}
	}
	
	return ret;
}
	

#endif
