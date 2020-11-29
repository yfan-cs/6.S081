// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void freerange(void *pa_start, void *pa_end, int i);

void kfree_init(void *pa, int i);

void *kalloc_steal(int i);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

/*
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;
*/

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    struct cpu *c = &cpus[i];
    initlock(&c->kmem.lock, "kmem");
    uint64 from = (uint64)end + (PHYSTOP - (uint64)end) / NCPU * i;
    uint64 to = (uint64)end + (PHYSTOP - (uint64)end) / NCPU * (i + 1);
    freerange((void*)from, (void*)to, i);
  }
}

void
freerange(void *pa_start, void *pa_end, int i)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree_init(p, i);
}

// Free the page of physical memory pointed at by v.
// On CPU i
void
kfree_init(void *pa, int i)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // no need to use lock when initializing
  r->next = cpus[i].kmem.freelist;
  cpus[i].kmem.freelist = r;
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc(). 
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off(); // disable interrupt
  struct cpu *c = mycpu();
  pop_off();  // enable interrupt

  acquire(&c->kmem.lock);
  r->next = c->kmem.freelist;
  c->kmem.freelist = r;
  release(&c->kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cpu_id = cpuid();
  pop_off();

  acquire(&cpus[cpu_id].kmem.lock);
  r = cpus[cpu_id].kmem.freelist;
  if(r)
    cpus[cpu_id].kmem.freelist = r->next;
  else {
    // steal from other CPUs' freelist
    r = kalloc_steal(cpu_id);
  }
  release(&cpus[cpu_id].kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void *
kalloc_steal(int i)
{
  struct run *r;
  for (int j = 0; j < NCPU; j++) {
    if (j != i) {
      acquire(&cpus[j].kmem.lock);
      r = cpus[j].kmem.freelist;
      if (r) {
        cpus[j].kmem.freelist = r->next;
        release(&cpus[j].kmem.lock);
        return r;
      }
      release(&cpus[j].kmem.lock);
    }
  }
  return r;
}




