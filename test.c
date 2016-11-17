#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

struct mm_entry {
	unsigned int 		status;
	size_t 				size;
	struct mm_entry 	*next;
	struct mm_entry		*prev;
};

#define STATUS_ALLOCATED	0xFFFFFFFF
#define STATUS_FREE			0xAAAAAAAA
#define PAGE_SIZE 			4096
#define MIN_ALLOC 			16
#define MAGIC				0xAABBCCDD

void *heap_start = NULL;

static struct mm_entry *head = NULL;
static bool sig_int = false;

static bool is_power_of_two(unsigned int x);
static unsigned int highest_bit(unsigned int x);

static unsigned int add_offset(void *ptr, unsigned int offset);
static unsigned int sub_offset(void *ptr, unsigned int offset);

static struct mm_entry *break_chunk(struct mm_entry *node, size_t desired_size);
static struct mm_entry *find_best_fit(size_t size, bool *best_fit);

static void dump_entries(void);
static int gen_rand(int min, int max);
void heap_init();
void alloc_test(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
typedef void sigfunc(int);
sigfunc *signal(int, sigfunc*);

struct timeval t1;

void sig_hand(int signo) {
	if (signo == SIGINT) {
		printf("\nreceived sigint\n");
		sig_int = true;
	} else if (signo == SIGTSTP) {
		struct timeval t2;
		double val;
		gettimeofday(&t2, NULL);
		val = t2.tv_sec - t1.tv_sec;
		
		printf("time elapsed: %f\n", val);
		dump_entries();
	}
}

int main(char *argv[], int argc) {
	int ret = 0;
	unsigned char *actual_ptr = NULL;
	
	if ((actual_ptr = malloc(PAGE_SIZE * 2)) != NULL) {
		heap_start = (void *)(add_offset(actual_ptr, PAGE_SIZE - 1) & ~0xFFF);
		heap_init();
		printf("address of actual pointer: 0x%x\n", (unsigned int)actual_ptr);
		printf("address of heap_start: 0x%x\n", (unsigned int)heap_start);
		printf("sizeof's: %u, %u\n", sizeof(void *), sizeof(unsigned int));
		printf("max address: 0x%x\n", (unsigned int)add_offset(actual_ptr, PAGE_SIZE * 2 - 1));
		//dump_entries();
		struct timeval t2;
		double time = 0.0;
		
		gettimeofday(&t1, NULL);
		alloc_test();
		gettimeofday(&t2, NULL);
		time = (t2.tv_sec - t1.tv_sec);
		
		printf("time elapsed: %f\n", time); 
		dump_entries();

		free(actual_ptr);
	} else {
		printf("failed to allocate %i bytes!\n", PAGE_SIZE * 2);
	}
	
	return ret;
}

static int gen_rand(int min, int max) {
	return (min + (rand() % (max - min) + 1));
}

void alloc_test(void) {
	void **ptr_table = (void *)kmalloc(75 * sizeof(void *));
	srand(time(NULL));
	
	if (signal(SIGINT, sig_hand) == SIG_ERR) {
		printf("SIG_ERR\n");
		return;
	}
	
	if (signal(SIGTSTP, sig_hand) == SIG_ERR) {
		printf("SIG_ERR\n");
		return;
	}
	
	if (ptr_table == NULL) {
		printf("ptr_table is null!\n");
	} else {
		while (!sig_int) {
			int index = gen_rand(0, 74);
			int rand = gen_rand(0, 9);
			
			if (rand >= 5) {
				if (ptr_table[index] == NULL) {
					ptr_table[index] = kmalloc(gen_rand(10, 110));
					
					if (ptr_table[index] == NULL) {
						ptr_table[index] = NULL;
					}
				}
			} else {
				if (ptr_table[index] != NULL) {
					kfree(ptr_table[index]);
					ptr_table[index] = NULL;
				}
			}
			
			/*
			struct timespec ts = {
				.tv_sec = 0,
				.tv_nsec = (999999999 + 1) / 4
			};
			
			struct timespec blah;
			
			nanosleep(&ts, &blah);
			//sleep(1);
			*/
		}
		
		kfree(ptr_table);
	}
}

void *kmalloc(size_t size) {
	void *ret = NULL;
	
	if (size > 0) {
		struct mm_entry *node = NULL;
		bool best_fit = false;
		
		if (size < MIN_ALLOC) {
			size = MIN_ALLOC;
		} else if (!is_power_of_two(size)) {
			size = highest_bit(size) << 1;
		}
		
		node = find_best_fit(size, &best_fit);
		
		if (node != NULL) {
			if (!best_fit) {
				struct mm_entry *nnode = break_chunk(node, size);
				
				if (nnode == NULL) {
					printf("no block large enough\n");
					exit(-1); /* fix me */
				} else {
					node = nnode;
				}
			}
			
			node->status = STATUS_ALLOCATED;
			ret = (void *)add_offset(node, sizeof(struct mm_entry));
		} else {
			//printf("need to allocate more memory! requested size: %i\n", size);
			//exit(-1);
		}
	}
	
	return ret;
}

void kfree(void *ptr) {
	struct mm_entry *node = NULL;
	
	if (ptr != NULL) {
		node = (struct mm_entry *)sub_offset(ptr, sizeof(struct mm_entry));
		
		if (node->status == STATUS_ALLOCATED) {
			struct mm_entry *next = node->next; 
			struct mm_entry *prev = node->prev;
			
			node->status = STATUS_FREE;
			
			/* look right */
			if (next != NULL && next->status == STATUS_FREE) { // eat next
				node->next	= next->next;
				
				if (node->next != NULL) {
					node->next->prev = node;
				}
				
				node->size += (next->size + sizeof(struct mm_entry));
			}
			
			/* look left */
			if (prev != NULL && prev->status == STATUS_FREE) { // eat node
				prev->next = node->next;
				
				if (prev->next != NULL) {
					prev->next->prev = prev;
				}
				
				prev->size += (node->size + sizeof(struct mm_entry));
			}
		}
	} else {
		printf("kfree() - ptr null\n");
	}
}

static void dump_entries(void) {
	struct mm_entry *node = head;
	size_t total = 0;
	size_t total_free = 0;
	
	int i = 0;
	
	while (node != NULL) {
		printf("[");
		
		if (node == head) {
			printf("H:");
		} else {
			printf("N:");
		}
		
		if (node->status == STATUS_ALLOCATED) {
			printf("A:");
		} else {
			printf("F:");
			total_free += node->size;
		}
		
		printf("0x%x:%i]", (unsigned int)node, node->size);
		total += node->size;
		
		if (node->next != NULL) {
			printf("->");
		}
		
		node = node->next;
		i++;

	}
	
	printf("[TOTAL: %i],[TOTAL_FREE: %i] [TOTAL NODES: %i, capacity: %i]\n", total + (i * sizeof(struct mm_entry)), total_free, i, i * sizeof(struct mm_entry));
	printf("\n");
}

struct mm_entry *break_chunk(struct mm_entry *node, size_t desired_size) {
	struct mm_entry *ret = NULL;
	size_t actual_size = desired_size + sizeof(struct mm_entry);
	
	if (node->status == STATUS_FREE) {
		if (node->size > actual_size) {
			struct mm_entry *new_node = NULL;
			
			node->size 		-= actual_size;
			new_node		= (struct mm_entry *)add_offset(node, node->size + sizeof(struct mm_entry));
			
			new_node->size	= desired_size;
			new_node->next 	= node->next;
			new_node->prev	= node;
			
			if (new_node->next != NULL) {
				new_node->next->prev = new_node;
			}
			
			node->next = new_node;
			
			ret = new_node;
		}
	}
	
	return ret;
}

void heap_init(void) {
	/* assume we've requested/been given a page to play with */
	struct mm_entry *node = (struct mm_entry *)heap_start;
	
	node->next 		= NULL;
	node->prev 		= NULL;
	node->size 		= PAGE_SIZE - sizeof(struct mm_entry);
	node->status	= STATUS_FREE;
	
	head = node;
}

static struct mm_entry *find_best_fit(size_t size, bool *best_fit) {
	struct mm_entry *ret = NULL;
	struct mm_entry *sch_node = head;
	struct mm_entry *next_best = NULL;
	bool bf = false;

	while (sch_node != NULL) {
		if (sch_node->status == STATUS_FREE) {
			if (sch_node->size == size) {
				ret = sch_node;
				bf 	= true;
				break;
			} else if (next_best != NULL) {
				if (sch_node->size < next_best->size && sch_node->size > (size + sizeof(struct mm_entry))) {
					next_best = sch_node;
				}
			} else if (sch_node->size > (size + sizeof(struct mm_entry))) {
				next_best = sch_node;
			}
		}
			sch_node = sch_node->next;
	}
	
	if (!bf) {
		ret = next_best;
	}
	
	*best_fit = bf;
	
	return ret;
}
	
static unsigned int add_offset(void *ptr, unsigned int offset) {
	return ((unsigned int)ptr + offset);
}

static unsigned int sub_offset(void *ptr, unsigned int offset) {
	return ((unsigned int)ptr - offset);
}

static bool is_power_of_two(unsigned int x) {
  return ((x != 0) && !(x & (x - 1)));
}

static unsigned int highest_bit(unsigned int x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	
	return x - (x >> 1);
}

static int first_set_bit(unsigned int x) {
	return __builtin_ffs(x) - 1;
}
