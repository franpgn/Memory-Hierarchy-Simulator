#include "memory_hierarchy_simulator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

FILE *fp;
FILE *disco;
FILE *program;

PageTableEntry page_table[PAGE_TABLE_SIZE];
CacheBlock cache[SETS][VIAS];
TLBEntry tlb[TLB_SIZE];
int main_memory[MAIN_MEMORY_SIZE];
int cacheHit = 0, cacheMiss = 0, tlbHit = 0, tlbMiss = 0, tpHit = 0, tpMiss = 0;
struct timespec timestamp;

void initialize_cache() {
  int i, j;
  for (i = 0; i < SETS; i++) {
    for (j = 0; j < VIAS; j++) {
      cache[i][j].dirty_bit = 0;
      cache[i][j].valid = 0;
    }
  }
}

void initialize_memory() {
  for (int i = 0; i < MAIN_MEMORY_SIZE; ++i) {
    main_memory[i] = 0;
  }
}

void initialize_tlb() {
  int i;
  for (i = 0; i < TLB_SIZE; i++) {
    tlb[i].valid = 0;
  }
}

void random_numbers(char *str, int num) {
  char random_numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  int random_numbers_length = sizeof(random_numbers) / sizeof(char);
  for (int i = 0; i < num; ++i) {
    str[i] = random_numbers[rand() % random_numbers_length];
  }
}

int initialize_disc() {
  srand(time(NULL));
  disco = fopen("disco.txt", "w+");
  if (disco == NULL) {
    printf("Failed to open file.\n");

    return 500;
  }

  char str[3];
  for (int i = 0; i < 1000; ++i) {
    if (i != 0)
      fprintf(disco, "\n");
    for (int j = 0; j < PAGE_SIZE; ++j) {
      random_numbers(str, 3);
      fprintf(disco, "%s", str);
      if (j < PAGE_SIZE - 1)
        fprintf(disco, " ");
    }
  }

  return 200;
}

int create_program() {
  srand(time(NULL));
  program = fopen("program.txt", "w+");
  if (program == NULL) {
    printf("Failed to open file.\n");

    return 500;
  }

  for (int i = 0; i < 100; ++i) {
    if (i != 0 && i != 255)
      fprintf(program, "\n");
    long op = (rand() % 2);
    // 0 load 1 store
    if (op != 0) {
      fprintf(program, "%s ", "S");
      fprintf(program, "%d ", (rand() % 256));
      fprintf(program, "%d", (rand() % 63));
    } else {
      fprintf(program, "%s ", "L");
      fprintf(program, "%d", (rand() % 63));
    }
  }
  fclose(program);

  return 0;
}

void write_memory(int byte_position, long physical_address) {
  int memory_address =
      (byte_position + (((physical_address / PAGE_SIZE) % 100) * PAGE_SIZE));
  int search_disk_address =
      (byte_position + ((physical_address / PAGE_SIZE) * PAGE_SIZE));
  fseek(disco, (search_disk_address * sizeof(int)), SEEK_SET);
  fscanf(disco, "%d", &main_memory[memory_address]);
}

void update_last_used(PageTableEntry *entry) {
	clock_gettime(CLOCK_REALTIME, &timestamp);
	entry->last_used = (timestamp.tv_sec * 1000);;
}

void write_back() {
  for (int i = 0; i < SETS; ++i) {
    for (int j = 0; j < VIAS; ++j) {
      if (cache[i][j].dirty_bit != 0) {
        long block_start_address = ((cache[i][j].tag % 100) * BLOCK_SIZE);
        for (int k = 0; k < BLOCK_SIZE; ++k) {
          main_memory[block_start_address + k] = cache[i][j].data[k];
        }
        cache[i][j].dirty_bit = 0;
      }
    }
  }
}

long load_write_cache(int op_code, long physical_address, long value) {
  long set_index = (physical_address / BLOCK_SIZE) % SETS;
  int via_index = 0;
  long block_start_address =
      (((physical_address / BLOCK_SIZE) % 100) * BLOCK_SIZE);
  for (int i = 0; i < VIAS; ++i) {
    if (cache[set_index][i].valid &&
        cache[set_index][i].tag == (physical_address / BLOCK_SIZE)) {
      cacheHit++;
      // Se store aloca value na cache e marca dirty bit
      if (op_code != 0) {
        cache[set_index][i].data[physical_address % BLOCK_SIZE] = value;
        cache[set_index][i].dirty_bit = 1;
      }
      // Atualizar last_used na tabela de páginas
      for (int k = 0; k < PAGE_TABLE_SIZE; k++) {
        if (page_table[k].valid &&
            page_table[k].physical_page == physical_address / PAGE_SIZE) {
          	update_last_used(&page_table[k]);
          break;
        }
      }
      return cache[set_index][i].data[physical_address % BLOCK_SIZE];
    }
  }

  cacheMiss++;
  // Verifica qual a via disponível naquele momento
  int set_full = 0;
  for (int i = 0; i < VIAS; ++i) {
    if (!cache[set_index][i].valid) {
      via_index = i;
      set_full = 0;
      break;
    } else {
      set_full = 1;
    }
  }

  // Verifica qual a via LRU para o conjunto alvo
  if (set_full) {
    clock_gettime(CLOCK_REALTIME, &timestamp);
    long max_time = (timestamp.tv_sec * 1000);
    for (int i = 0; i < VIAS; ++i) {
      if (max_time > cache[set_index][i].last_used) {
        max_time = cache[set_index][i].last_used;
        via_index = i;
      }
    }
  }

	for (int i = 0; i < PAGE_SIZE; ++i) {
    write_memory(i, physical_address);
  }
  for (int j = 0; j < BLOCK_SIZE; j++) {
    cache[set_index][via_index].data[j] = main_memory[block_start_address + j];
  }

  if (op_code != 0) {
    cache[set_index][via_index].data[physical_address % BLOCK_SIZE] = value;
    cache[set_index][via_index].dirty_bit = 1;
  }

  clock_gettime(CLOCK_REALTIME, &timestamp);
  usleep(500000);
  cache[set_index][via_index].last_used = (timestamp.tv_sec * 1000);
  cache[set_index][via_index].valid = 1;
  cache[set_index][via_index].tag = physical_address / BLOCK_SIZE;

  return cache[set_index][via_index].data[physical_address % BLOCK_SIZE];
}

void print_cache() {
  int i, j, k;
  for (i = 0; i < SETS; i++) {
    printf("Set %d:\n", i);
    for (j = 0; j < VIAS; j++) {
      printf("Via %d: ", j);
      if (cache[i][j].valid) {
        printf("Valid: Yes, Tag: %ld, \nData: ", cache[i][j].tag);
        for (k = 0; k < BLOCK_SIZE; k++) {
          printf("%ld ", cache[i][j].data[k]);
        }
      } else {
        printf("Valid: No");
      }
      printf("\n");
    }
    printf("\n");
  }
}

void print_main_memory() {
  for (int i = 0; i < 100; ++i) {
    printf("\nAddress %d:", i);
    for (int j = 0; j < PAGE_SIZE; ++j) {
      printf("%d ", main_memory[(PAGE_SIZE * i) + j]);
    }
  }
}

long search_physical_address(long virtual_address){
	long page_number = virtual_address / (PAGE_SIZE);
  long offset = virtual_address % (PAGE_SIZE);
	int page_index = 0;
  for (int i = 0; i < TLB_SIZE; i++) {
    if (tlb[i].valid && tlb[i].virtual_page == page_number) {
      tlbHit++;
      return (tlb[i].physical_page * PAGE_SIZE) + offset;
    }
  }

  tlbMiss++;

  // Atualizar last_used na tabela de páginas
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    if (page_table[i].valid && page_table[i].virtual_page == page_number) {
			tpHit++;
			clock_gettime(CLOCK_REALTIME, &timestamp);
      page_table[i].last_used = (timestamp.tv_sec * 1000);
			page_table[i].valid = 1;
      return (page_table[i].physical_page * PAGE_SIZE) + offset;
    }
  }

	tpMiss++;

  long physical_page = page_number;
	int page_table_full = 0;
  for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
    if (!page_table[i].valid) {
			clock_gettime(CLOCK_REALTIME, &timestamp);
      page_table[i].last_used = (timestamp.tv_sec * 1000);
			page_table[i].valid = 1;
			page_table[i].virtual_page = page_number;
			page_table[i].physical_page = physical_page;
			break;
    } else {
      page_table_full = 1;
    }
  }

  // Verifica qual a via LRU para o conjunto alvo
  if (page_table_full) {
    clock_gettime(CLOCK_REALTIME, &timestamp);
    long max_time = (timestamp.tv_sec * 1000);
    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
      if (max_time > page_table[i].last_used) {
        max_time = page_table[i].last_used;
        page_index = i;
      }
    }
		clock_gettime(CLOCK_REALTIME, &timestamp);
    page_table[page_index].last_used = (timestamp.tv_sec * 1000);
		page_table[page_index].valid = 1;
		page_table[page_index].virtual_page = page_number;
		page_table[page_index].physical_page = physical_page;
  }

	int tlb_full = 0;
	for (int i = 0; i < TLB_SIZE; ++i){
	  if (!tlb[i].valid){
			tlb[i].valid = 1;
		  tlb[i].virtual_page = page_number;
		  tlb[i].physical_page = physical_page;
			break;
		}
		tlb_full = 1;
	}
	
	int tlb_entry_index = rand() % TLB_SIZE;
  tlb[tlb_entry_index].valid = 1;
  tlb[tlb_entry_index].virtual_page = page_number;
  tlb[tlb_entry_index].physical_page = physical_page;
	return (tlb[tlb_entry_index].physical_page * PAGE_SIZE) + offset;
}

long read_program() {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  fp = fopen("program.txt", "r");

  if (fp == NULL) {
    printf("Failed to open file.\n");
    return 500;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    long virtual_address;
    char access_type;
    long store_value;
    sscanf(line, "%c %ld", &access_type, &virtual_address);

    if (access_type == 'L') {
			// Procura o endereço virtual pego na instrução na TLB e TP, se não achar, o traduz
      long physical_address = search_physical_address(virtual_address);
      
      printf("Physical address: %ld\t\t", physical_address);
      long value = load_write_cache(0, physical_address, 0);
      printf("Read value: %ld\n", value);

    } else if (access_type == 'S') {
      sscanf(line, "%c %ld %ld", &access_type, &store_value, &virtual_address);
			// Procura o endereço virtual pego na instrução na TLB e TP, se não achar, o traduz
      long physical_address = search_physical_address(virtual_address);
      
      printf("Physical address: %ld\t\t", physical_address);
      load_write_cache(1, physical_address, store_value);
      printf("Write operation completed.\n");
    }
  }

  return 200;
}

void close_files() {
  fclose(fp);
  fclose(disco);
  fclose(program);
}

void print_rates() {
  float total_operations_cache = cacheHit + cacheMiss;
	float total_operations_tlb = tlbHit + tlbMiss;
	float total_operations_tp = tpHit + tpMiss;
  printf("\n\nTaxa de Acertos na Cache: %.2f\n", (cacheHit / total_operations_cache) * 100);
	printf("Taxa de Acertos na TLB: %.2f\n", (tlbHit / total_operations_tlb) * 100);
	printf("Taxa de Acertos na TP: %.2f\n\n", (tpHit / total_operations_tp) * 100);
}