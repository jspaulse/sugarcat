/* Copyright (C) 2016 Jacob Paulsen <jspaulse@ius.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <mach/mach.h> /* TODO: tmp */
#include <util/fdt.h>
#include <util/bits.h>
#include <util/str.h>

char tabs[256]; /* TODO: tmp */

#define ALIGN_UP(addr, align) ((addr + (align - 1)) & ~((align - 1)))

static const char *fdt_get_string(addr_t fdt_base, addr_t offset);
static struct fdt_node *fdt_get_next_tag(struct fdt_node *node) ;
static size_t fdt_get_tag_size(struct fdt_node *node);

/**
 * fdt_get_root_node
 * 
 * returns the root node of the flattened device tree.
 * 
 * @fdt_base	base address of the fdt
 * @return root node or null if not found
 **/
struct fdt_node *fdt_get_root_node(addr_t fdt_base) {
    struct fdt_node 	*ret	= NULL;
    struct fdt_header	*head	= (struct fdt_header *)fdt_base;
    
    /* ensure it's actually fdt */
    if (is_using_fdt(fdt_base)) {
	ret = (struct fdt_node *)(fdt_base + be32_to_cpu(head->dt_struct_offset));
    }
    
    return ret;
}

/**
 * fdt_get_next_node
 * 
 * returns the next node in the flattened device tree.
 * @node current node
 * @return next node or null if at end
 **/
struct fdt_node *fdt_get_next_node(struct fdt_node *node) {
    struct fdt_node	*ret 	= NULL;
    struct fdt_node	*iter	= NULL;
    
    if (node != NULL) {
	iter = fdt_get_next_tag(node);
	
	while (iter != NULL && be32_to_cpu(iter->tag) != FDT_END) {
	    if (be32_to_cpu(iter->tag) == FDT_BEGIN_NODE) {
		ret = iter;
		break;
	    }
	    
	    iter = fdt_get_next_tag(iter);
	}
    }
    
    return ret;
}

/**
 * fdt_get_node
 * 
 * searches for and returns (if found) a node with specified name
 * @name	name to search for
 * @fdt_base	base address of flattened device tree
 * @return node if found, null otherwise
 **/
struct fdt_node *fdt_get_node(const char *name, addr_t fdt_base) {
    struct fdt_node *ret 	= NULL;
    struct fdt_node *iter	= NULL;
    
    if (name != NULL && is_using_fdt(fdt_base)) {
	iter = fdt_get_root_node(fdt_base);
	
	while (iter != NULL) {
	    if (strcmp(name, iter->data) == 0) {
		ret = iter;
		break;
	    } else if ((strcmp(name, "/") == 0) 
		&& strlen(iter->data) == 0) { /* special case */
		ret = iter;
		break;
	    }
	    
	    iter = fdt_get_next_node(iter);
	}
    }
    
    return ret;
}

/**
 * fdt_get_property
 * 
 * searches for and returns (if found) the property of a node of
 * specified name.
 * @fdt_base	base address of flattened device tree
 * @node	node to search within
 * @property	name to search for
 * @return property (if found) or null otherwise
 **/
struct fdt_property *fdt_get_property(addr_t fdt_base, struct fdt_node *node, const char *property) {
    struct fdt_property *ret 	= NULL;
    
    if (node != NULL && property != NULL) {
	if (be32_to_cpu(node->tag) == FDT_BEGIN_NODE) {
	    node = fdt_get_next_tag(node);
	    
	    while (node != NULL && (be32_to_cpu(node->tag) != FDT_END_NODE &&
		be32_to_cpu(node->tag) != FDT_BEGIN_NODE)) {
		const char *prop_str = NULL;
		
		if (be32_to_cpu(node->tag) == FDT_PROP) {
		    prop_str = fdt_get_string(fdt_base, 
			be32_to_cpu(((struct fdt_property *)node)->name_offset));
			
		    if (prop_str != NULL && strcmp(property, prop_str) == 0) {
			ret = (struct fdt_property *)node;
			break;
		    }
		}
		
		node = fdt_get_next_tag(node);
	    }
	}
    }
    
    return ret;
}

/**
 * fdt_get_next_property
 * 
 * returns the next property in a node
 * 
 * @prop	current property
 * @return next property (or null).
 **/
struct fdt_property *fdt_get_next_property(struct fdt_property *prop) {
    struct fdt_property *ret 	= NULL;
    struct fdt_node	*iter	= NULL;
    
    if (prop != NULL) {
	iter = fdt_get_next_tag((struct fdt_node *)prop);
	
	while (iter != NULL && (be32_to_cpu(iter->tag) != FDT_END_NODE
	    && be32_to_cpu(iter->tag) != FDT_BEGIN_NODE)) {
		if (be32_to_cpu(iter->tag) == FDT_PROP) {
		    ret = (struct fdt_property *)iter;
		    break;
		}
		
		iter = fdt_get_next_tag(iter);
	}
    }
    
    return ret;
}

/**
 * fdt_get_cell_size
 * 
 * returns the cell size of the node or, if it doesn't exist,
 * returns the default size-cells from the root node.
 * 
 * 
 * @fdt_base	base address of fdt
 * @node	node to search cell-size
 * @return cell size
 **/
fdt32_t fdt_get_cell_size(addr_t fdt_base, struct fdt_node *node) {
    struct fdt_property	*prop 	= NULL;
    fdt32_t 		*ptr 	= NULL;
    fdt32_t		ret	= 0; //((fdt32_t *)&prop->data);
    
    if (node != NULL) {
	prop = fdt_get_property(fdt_base, node, "#size-cells");
	
	if (prop == NULL) {
	     prop = fdt_get_property(fdt_base, 
		fdt_get_root_node(fdt_base), "#size-cells");
	    
	    if (prop != NULL) {
		ptr = ((fdt32_t *)prop->data);
		
		ret = be32_to_cpu(*ptr);
	    }
	}
    }
    
    return ret;
}

/**
 * fdt_get_string
 * 
 * returns the string based on the offset
 * @fdt_base	base address of fdt
 * @offset	offset
 * @return string
 **/
static const char *fdt_get_string(addr_t fdt_base, addr_t offset) {
    return (const char *)(fdt_base + offset +
	be32_to_cpu(((struct fdt_header *)fdt_base)->dt_strings_offset));
}

/**
 * fdt_get_next_tag
 * 
 * returns the next tag within the fdt if current node != FDT_END
 * @node current node
 * @return next tag or null if invalid
 **/
static struct fdt_node *fdt_get_next_tag(struct fdt_node *node) {
    struct fdt_node *ret = NULL;
    
    if (node != NULL && (be32_to_cpu(node->tag)) != FDT_END) {
	ret = (struct fdt_node *)(ALIGN_UP((addr_t)node +
	    fdt_get_tag_size(node), 4));
    }
    
    return ret;
}

/**
 * fdt_get_tag_size
 * 
 * returns the size of a tag within the fdt.
 * @node	tag to check
 * @return size of tag for offset
 **/
static size_t fdt_get_tag_size(struct fdt_node *node) {
    size_t	ret = 0;
    fdt32_t	tag = 0;
    
    if (node != NULL) {
	tag = be32_to_cpu(node->tag);
	
	if (tag == FDT_BEGIN_NODE) {
	    ret = (sizeof(struct fdt_node) + (strlen(node->data) + 1));
	} else if (tag == FDT_PROP) {
	    ret = (sizeof(struct fdt_property) 
		+ be32_to_cpu(((struct fdt_property *)node)->length));
	} else {
	    ret = sizeof(struct fdt_node);
	}
    }
    
    return ret;
}

/* TODO: tmp */
void dump_fdt(addr_t fdt_base) {
    struct fdt_node 	*node = fdt_get_root_node(fdt_base);
    struct fdt_property *prop = NULL;
    
    while (node != NULL) {
	fdt32_t tag = be32_to_cpu(node->tag);
	
	if (tag == FDT_BEGIN_NODE) {
	    size_t sz = strlen(tabs);
	    
	    if (strlen(node->data) > 0) {
		mach_early_kprintf("%s%s {\n", tabs,  node->data);
	    } else {
		mach_early_kprintf("/ {\n");
	    }
	    
	    tabs[sz] = '\t';
	    tabs[sz + 1] = '\0';
	    
	} else if (tag == FDT_END_NODE) {
	    tabs[strlen(tabs) - 1] = '\0';
	    
	    mach_early_kprintf("%s};\n", tabs);
	} else if (tag == FDT_PROP) {
	    const char *prop_str = NULL;
	    
	    prop 	= (struct fdt_property *)node;
	    prop_str	= fdt_get_string(fdt_base, 
		be32_to_cpu(prop->name_offset));
	    
	    mach_early_kprintf("%s%s\n", tabs, prop_str);
	}
	
	node = fdt_get_next_tag(node);
    }
}
    
