#include "memory_hierarchy_simulator.h"
#include <stdio.h>

//Carlos Eduardo Vidal & Francesco Pagani Galvão

int main() {
  create_program();

  initialize_disc();
  initialize_cache();
  initialize_memory();
  initialize_tlb();

  read_program();
	write_back();
	
  print_rates();
  print_cache();
  print_main_memory();

  return 0;
}