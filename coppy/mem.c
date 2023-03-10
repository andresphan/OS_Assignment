
#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

static BYTE _ram[RAM_SIZE];

static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct trans_table_t * get_trans_table(
		addr_t index, 	// Segment level index
		struct seg_table_t * seg_table) { // first level table
	
	/*
	 * TODO: Given the Segment index [index], you must go through each
	 * row of the segment table [seg_table] and check if the v_index
	 * field of the row is equal to the index
	 *
	 * */

	int i;
	for (i = 0; i < seg_table->size; i++) {
		// Enter your code here
		if(((seg_table->table[i]).v_index)==index){
			return (seg_table->table[i]).next_lv;
			}
	}
	return NULL;

}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */

static int translate(
		addr_t virtual_addr, 	// Given virtual address
		addr_t * physical_addr, // Physical address to be returned
		struct pcb_t * proc) {  // Process uses given virtual address

	/* Offset of the virtual address */
	addr_t offset = get_offset(virtual_addr);
	// printf("1 %d\n",offset);
	/* The first layer index */
	addr_t first_lv = get_first_lv(virtual_addr);
	// printf("2 %d\n",first_lv);
	/* The second layer index */
	addr_t second_lv = get_second_lv(virtual_addr);
	// printf("3 %d\n",second_lv);
	addr_t temp;
	// *temp=9;
	// addr_t temp=second_lv*1024+offset;
	// *physical_addr=temp;
	// return 1;
	/* Search in the first level */
	struct trans_table_t * trans_table = NULL;
	// printf("1");
	trans_table = get_trans_table(first_lv, proc->seg_table);
	if (trans_table == NULL) {
		// printf("1\n");
		return 0;
	}
	
	int i;
	for (i = 0; i < trans_table->size; i++) {
		if (trans_table->table[i].v_index == second_lv) {
			/* TODO: Concatenate the offset of the virtual addess
			 * to [p_index] field of trans_table->table[i] to 
			 * produce the correct physical address and save it to
			 * [*physical_addr]  */
			// std::string result = std::to_string(offset) + std::to_string((trans_table->table[i]).p_index);
			// temp=((trans_table->table[i]).p_index << offset);
			// printf("2\n");
			temp = trans_table->table[i].p_index + offset;
			*physical_addr = temp;
			return 1;
		}
	}
	return 0;	
}

addr_t alloc_mem(uint32_t size, struct pcb_t * proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;
	/* TODO: Allocate [size] byte in the memory for the
	 * process [proc] and save the address of the first
	 * byte in the allocated memory region to [ret_mem].
	 * */
	
	// addr_t temp = 0;
	// addr_t	previous = 0;
	// ret_mem = proc-> regs[0];
	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE :
		size / PAGE_SIZE + 1; // Number of pages we will use
	// printf("num page%d\n",num_pages);	
	int mem_avail = 0; // We could allocate new memory region or not?
	// int nums = 0;
	
	// int a = 0;
	// printf("NUM_PAGES %d\n",NUM_PAGES);

	for(int a= 0; a <=NUM_PAGES;a++){
		if(_mem_stat[a].proc==0){			
			ret_mem=(a)*PAGE_SIZE;
			break;			
		}
	}
	// mem_avail=1;		
	if(proc->bp<=RAM_SIZE-size){
		mem_avail=1;
	}
	int num_pages_avail=0;
	for(int a = 0; a<= NUM_PAGES;a++){
		if(_mem_stat[a].proc==0){
			num_pages_avail++;
		}
		if(num_pages_avail>num_pages){
			mem_avail=1;
			break;
		}
	}
	/* First we must check if the amount of free memory in
	 * virtual address space and physical address space is
	 * large enough to represent the amount of required 
	 * memory. If so, set 1 to [mem_avail].
	 * Hint: check [proc] bit in each page of _mem_stat
	 * to know whether this page has been used by a process.
	 * For virtual memory space, check bp (break pointer).
	 * */
	
	// if(1){
	// 	mem_avail=1;
	// }
	if (mem_avail) {
		/* We could allocate new memory region to the process */
		// addr_t offset = get_offset(ret_mem);
		// printf("a1 %d\n",offset);
		/* The first layer index */
		addr_t first= -1;
		addr_t first_lv = get_first_lv(ret_mem);
		
		addr_t second_lv = get_second_lv(ret_mem);
		addr_t _size;
		
		// trans_table.table[0].v_index=0;
		// trans_table->table[0].v_index=1;
		if(proc->seg_table->size==0){
			proc->seg_table->table[0].v_index=first_lv;
			first = 0;
			// proc->seg_table->table[first].next_lv->size=0;
			proc->seg_table->size=1;
		}
		for(int u=0;u<proc->seg_table->size;u++){
			if(proc->seg_table->table[u].v_index==first_lv){
				first=u;
				break;
			}
		}
		if(first==-1){
			proc->seg_table->table[proc->seg_table->size].v_index=first_lv;
			first=proc->seg_table->size;
			proc->seg_table->size++;
		}
		// printf("first lv %d\n",first);
		// printf("_size %d\n",first_lv);
		// printf("second lv %d\n",proc->seg_table->table[first].next_lv->size);

		/* The second layer index */
		
		// printf("a3 %d\n",second_lv);
		int mem_num=0;
		addr_t temp = ret_mem/PAGE_SIZE;
		addr_t previous = temp;
		int num = 0;
		// ret_mem = proc->bp;
		// =0x000;
		// strcpy(&(trans_table->table[0].v_index),0);
		// printf("bp1 %d\n",trans_table->table[0].v_index);
		proc->bp += num_pages * PAGE_SIZE;
		// printf("bp2 %d\n",proc->seg_table->table[first].next_lv==NULL);

		/* Update status of physical pages which will be allocated
		 * to [proc] in _mem_stat. Tasks to do:
		 * 	- Update [proc], [index], and [next] field
		 * 	- Add entries to segment table page tables of [proc]
		 * 	  to ensure accesses to allocated memory slot is
		 * 	  valid. */
		// int num_temp
		for (int i = temp;i<=PAGE_SIZE;i++){
			if(_mem_stat[i].proc==0){
				_mem_stat[i].proc = proc->pid;
				_mem_stat[i].index=mem_num;
				mem_num++;
				// proc->seg_table->table[1].next_lv->size=1;

				if(proc->seg_table->table[first].next_lv==NULL){
					struct trans_table_t  trans_table;
					trans_table.table[0].v_index=0;
					trans_table.table[0].p_index=0;
					trans_table.size=0;
					proc->seg_table->table[first].next_lv=&trans_table;
				}
				// printf("i %d\n",i);
				// printf("transize3 %d\n",proc->seg_table->table[first].next_lv->size);
				proc->seg_table->table[first].next_lv->table[proc->seg_table->table[first].next_lv->size].v_index=(i);
				proc->seg_table->table[first].next_lv->table[proc->seg_table->table[first].next_lv->size].p_index=first_lv*32768;
				proc->seg_table->table[first].next_lv->table[proc->seg_table->table[first].next_lv->size].p_index+=(i)*1024;
				// printf("v_index %d\n",proc->seg_table->table[first].next_lv->table[proc->seg_table->table[first].next_lv->size].v_index);
				// printf("p_index %d\n",proc->seg_table->table[first].next_lv->table[proc->seg_table->table[first].next_lv->size].p_index);

				proc->seg_table->table[first].next_lv->size++;
				
				// ->table[_size]->v_index=second_lv;
				// proc->seg_table->table[first_lv].next_lv->table[_size]->p_index=first_lv<<(OFFSET_LEN+PAGE_LEN)+(second_lv<<SECOND_LV_LEN);
				// printf("abs %d\n",proc->seg_table->table[first_lv].next_lv->table->p_index);


				// proc->seg_table->table[first_lv].next_lv->size++;

				// printf("seg %d\n",proc->seg_table->table[mem`_num].next_lv);	
				// proc->seg_table->size++;
			}
			if (mem_num>=1){
				_mem_stat[previous].next=i;
			}
			previous=i;
			if(mem_num>=num_pages+1){
				_mem_stat[i].next=-1;
				// mem_avail=1;
				break;
			}

		}

		for(int i = 0; i < proc->seg_table->table[first].next_lv->size;i++ ){
		printf("id1 %d\n",proc->seg_table->table[first].next_lv->table[i].v_index);
		printf("pid1 %d\n",proc->seg_table->table[first].next_lv->table[i].p_index);
		}
		// printf("id13 %d\n",proc->seg_table->table[first].next_lv->table[14].v_index);
		printf("--------------\n");

		
	}
	
	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/*TODO: Release memory region allocated by [proc]. The first byte of
	 * this region is indicated by [address]. Task to do:
	 * 	- Set flag [proc] of physical page use by the memory block
	 * 	  back to zero to indicate that it is free.
	 * 	- Remove unused entries in segment table and page tables of
	 * 	  the process [proc].
	 * 	- Remember to use lock to protect the memory from other
	 * 	  processes.  */
	pthread_mutex_lock(&mem_lock);
	addr_t temp = (address/PAGE_SIZE);
	addr_t first = temp;
	addr_t first_lv = get_first_lv(address);
	addr_t second_lv = get_second_lv(address);
	// temp=2;
	// _mem_stat[address].proc=0;
	// printf("%d\n",address);
	// if(){
	// 	
	// }
	int num_pages = 1;
	// if(_mem_stat[temp].next==-1){
	// 	_mem_stat[temp].proc=0;
	// 	_mem_stat[temp-1].proc=0;
	// 	// printf("id %d\n",temp);
	// 	num_pages++;
	// 	// temp=_mem_stat[temp].next;
	// 	// _mem_stat[_mem_stat[temp].next].proc=0;
	// 	proc->bp -= PAGE_SIZE;
	// 	// pthread_mutex_unlock(&mem_lock);
	// 	// return 0;
	// }
	// for(int i = 0; i < proc->seg_table->table[0].next_lv->size;i++ ){
	// 	printf("id %d\n",proc->seg_table->table[0].next_lv->table[i].v_index);
	// }

	while(_mem_stat[temp].next!=-1){
		// 
		// for(int i = 0; i < proc->seg_table->table[first_lv].next_lv->size;i++ ){
		// 	if((proc->seg_table->table[first_lv].next_lv->table[i].v_index)==temp){
		// 		first=i;
		// 		break;
		// 	}
		// }
		// 	{
		// 		printf("1\n");
		// 		// proc->seg_table->table[first_lv].next_lv->table[i].v_index=-1;
		// 		// for(int j = i; j<=proc->seg_table->table[first_lv].next_lv->size;j++){
		// 		// 	proc->seg_table->table[first_lv].next_lv->table[j]=
		// 		// 		proc->seg_table->table[first_lv].next_lv->table[j+1];
		// 		// }
		// 		proc->seg_table->table[first_lv].next_lv->size--;
		// 	}
		// }
		// proc->seg_table->table[first].next_lv->size--;
		_mem_stat[temp].proc=0;
		// _mem_stat[temp].index=-1;
		temp=_mem_stat[temp].next;
		num_pages++;
	}

	_mem_stat[temp].proc=0;
	// int a = proc->seg_table->table[first_lv].next_lv->size;
	for(int i = first; i <second_lv+num_pages;i++){
		
		proc->seg_table->table[first_lv].next_lv->size--;
		// printf("%d\n",proc->seg_table->table[first_lv].next_lv->size);
	}
	// proc->seg_table->table[0].next_lv->table[0]=proc->seg_table->table[0].next_lv->table[14];
	// proc->seg_table->table[0].next_lv->table[1]=proc->seg_table->table[0].next_lv->table[15];
	if(proc->seg_table->table[first_lv].next_lv->size==0){
		proc->seg_table->table[first_lv].next_lv=NULL;
	}
	else{
		for(int i = 0; i <proc->seg_table->table[first_lv].next_lv->size;i++){
			proc->seg_table->table[first_lv].next_lv->table[i]=proc->seg_table->table[first_lv].next_lv->table[i+second_lv+num_pages];
		}
	}
	// printf("id1 %d\n",proc->seg_table->table[first].next_lv->table[i].v_index);
	// printf("id1 %d\n",proc->seg_table->table[first].next_lv->table[i].v_index);
	// for(int i = 0; i <proc->seg_table->table[first_lv].next_lv->size--;i++){
		// proc->seg_table->table[first_lv].next_lv->table[i].v_index=
			// proc->seg_table->table[first_lv].next_lv->table[i+first+num_pages].v_index;
	// }

	// _mem_stat[temp].proc=0;
	// printf("num page%d\n",num_pages);
	proc->bp -= (num_pages) * PAGE_SIZE;
	// if(proc->bp==0)proc->bp=1024;
	pthread_mutex_unlock(&mem_lock);
	return 0;
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	addr_t physical_addr;
	
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	}else{
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	addr_t physical_addr;
	// printf("mem num %d\n",address);
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	}else{
		return 1;
	}
}

void dump(void) {
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (	j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
}


