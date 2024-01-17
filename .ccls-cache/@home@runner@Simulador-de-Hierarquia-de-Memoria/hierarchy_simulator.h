#ifndef MEMORY_HIERARCHY_SIMULATOR
#define MEMORY_HIERARCHY_SIMULATOR

#define WORD_SIZE 4
#define BLOCK_SIZE (4 * WORD_SIZE)
#define BLOCKS_PER_PAGE 2
#define PAGE_SIZE (BLOCK_SIZE * BLOCKS_PER_PAGE)
#define VIAS 4
#define SETS 8
#define CACHE_SIZE (VIAS * BLOCK_SIZE * SETS)
#define TLB_SIZE 2
#define MAIN_MEMORY_SIZE (PAGE_SIZE * 100)
#define PAGE_TABLE_SIZE 100

typedef struct {
  long valid;
  long tag;
  long data[BLOCK_SIZE];
} CacheBlock;

typedef struct {
  long valid;
  long virtual_page;
  long physical_page;
} TLBEntry;

typedef struct {
  long valid;
  long virtual_page;
  long physical_page;
  long last_used;
} PageTableEntry;

PageTableEntry page_table[PAGE_TABLE_SIZE];
CacheBlock cache[SETS][VIAS];
TLBEntry tlb[TLB_SIZE];
int main_memory[MAIN_MEMORY_SIZE];

void initialize_cache();
void initialize_tlb();
void random_numbers(char* str, int num);
int initialize_disc();
int create_program();
void write_memory(int byte_position, long physical_address);
void update_last_used(PageTableEntry* entry);
long get_least_recently_used_page();
void write_to_cache(long physical_address, long value);
long read_from_cache(long physical_address);
long translate_virtual_to_physical(long virtual_address);
void print_cache();
void print_main_memory();
long read_program();

#endif