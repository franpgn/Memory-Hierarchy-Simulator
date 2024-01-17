#ifndef MEMORY_HIERARCHY_SIMULATOR
#define MEMORY_HIERARCHY_SIMULATOR

#define WORD_SIZE 4
#define BLOCK_SIZE (4 * WORD_SIZE)
#define BLOCKS_PER_PAGE 2
#define PAGE_SIZE (BLOCK_SIZE * BLOCKS_PER_PAGE)
#define VIAS 4
#define SETS 8
#define CACHE_SIZE (VIAS * BLOCK_SIZE * SETS)
#define TLB_SIZE (2)
#define MAIN_MEMORY_SIZE (PAGE_SIZE * 100)
#define PAGE_TABLE_SIZE 100

typedef struct {
  long valid;
  long tag;
  long data[BLOCK_SIZE];
  long long last_used;
  int dirty_bit;
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
  long long last_used;
} PageTableEntry;

const char *decToHex(long decNum);
void initialize_cache();
void initialize_memory();
void initialize_tlb();
void random_numbers(char *str, int num);
int initialize_disc();
int create_program();
void write_memory(int byte_position, long physical_address);
void update_last_used(PageTableEntry *entry);
long load_write_cache(int op_code, long physical_address, long value);
long search_physical_address(long virtual_address);
void write_back();
void print_cache();
void print_main_memory();
long read_program();
void print_rates();

#endif