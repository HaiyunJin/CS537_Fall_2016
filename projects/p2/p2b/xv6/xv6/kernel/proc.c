#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h" // haiyun 

int mydebug = 0;

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;


// haiyun: define timeticks in each level
 const int mlpq_timeticks[NLAYER] = { 5, 5, 10, 20}; 
// const int mlpq_timeticks[NLAYER] = { 10, 10, 20, 50}; 
//const int mlpq_timeticks[NLAYER] = { 50, 50, 100, 200}; 
// haiyun: define ninlevel
int ninlevel[NLAYER] = { 0, 0, 0, 0};

// haiyu: laststate to track the level and state every 100 ticks (1s)
struct {
    int level[NPROC];
    int timeticks[NPROC][NLAYER];
} laststate;


static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
// haiyun allocproc
  p->level = 0; // new process at level 0
 // p->timeticks = mlpq_timeticks[0]; // initialize with level 0 time ticks
 int ii;
 for ( ii = 0 ; ii < NLAYER; ++ii) {
   p->timeticks[ii] = 0; // initialize each level with 0 time ticks
 }
//  ninlevel[0]++;
  release(&ptable.lock);

//   // update pstat // haiyun 
//    int index = p - ptable.proc;
//    int i = 0 ;
//    //cprintf("index %d\n",index);  
//    // cprintf("i %d\n",i);
//    // cprintf("pstat point %p\n",pstat);
//    // cprintf("ptable point %p\n",&ptable);
//    pstat.inuse[index] = 1;
//    pstat.pid[index] = p->pid;
//    pstat.priority[index] = 0;
//    pstat.state[index] = EMBRYO;
//    for ( i = 0 ; i < NLAYER ; ++i ) {
//      pstat.ticks[index][i] = 0;
//    }
// 
    // cprintf("before run kstack\n");


  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  ninlevel[p->level]++; // haiyun: new runnable proc
if (mydebug) {
    cprintf("increase level[%d]: userinit process\n",p->level);
    cprintf("after [pid %d] created %d %d %d %d\n",p->pid,
                ninlevel[0],ninlevel[1],ninlevel[2],ninlevel[3]);
}

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  ninlevel[0]++; // haiyun: new runnable proc from fork
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  // haiyun exit: when process exit, decrease ninlevel
  ninlevel[proc->level]--;
if(mydebug) {
  cprintf("decrease level[%d]: proc exit\n",proc->level);
        cprintf("after [pid %d] exit %d %d %d %d\n",proc->pid,
                ninlevel[0],ninlevel[1],ninlevel[2],ninlevel[3]);
        cprintf("\n");
}

  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    // haiyun: call sleep
if(mydebug){
    cprintf("[pid %d] call wait sleep\n",proc->pid);
}
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;


  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);


int ii ;
//loop_nlayer:
for ( ii = 0 ; ii < NLAYER ; ++ii ) { // haiyun loop nlayer
//    loop_proc:
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){

      if(p->state != RUNNABLE)
        continue;

       int highestlevel = 0;
       while ( highestlevel < NLAYER-1 && ninlevel[highestlevel] == 0) {
       //      cprintf("lowestlevel++");
             highestlevel++;
       }
if(mydebug>5)
{
        cprintf(" level %d\n",highestlevel);
}
       //if ( ii == NLAYER ) ii--;
    //   if ( lowestlevel == NLAYER ) lowestlevel--;
    //  cprintf("Now in level %d\n",ii);

      if ( ii != highestlevel ) {
      // new lower level proc created, rescan ptable
        ii = highestlevel-1; // trick the for loop to start from highest level
        break ;
      }

      // haiyun: check for level ii
      if( p->level != ii ) 
        continue;

    // haiyun: instead of when yield or sleep, decrease tt when picked up 
// // MLPQ rule 8. downgrade for 012 level
//     if ( p->level != NLAYER -1 ) {
//         if ( --(p->timeticks) == 0 ) {
//             ninlevel[p->level]--;
// if(mydebug)
// { 
//     cprintf("decrease level[%d]: time slice used up\n",p->level);
// }
//             p->level++;
//             p->timeticks = mlpq_timeticks[p->level];
//             ninlevel[p->level]++;
// if(mydebug)
// {
//     cprintf("increase level[%d]: downgrade from upper level\n",p->level);
// }
//         }
//     }


   // if can reach here, p->level must == highestlevel
//    if ( p->level == highestlevel ) {
 //   }


// // MLPQ rule 7. Use full timeslice if no higher priority proc
//     if ( p->level == highestlevel ) {
//         if (--(p->timeticks) != 0) {
//             p--;
//         } else {
//             p->timeticks = mlpq_timeticks[NLAYER-1];
//         }
//     }
     
//    ninlevel[proc->level]--; // haiyun: not runnable any more


      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;


      switchuvm(p);
      p->state = RUNNING;

        if (++(p->timeticks[p->level])%mlpq_timeticks[p->level] != 0) {
            p--;
        } else { // tt used up
            if ( p->level != NLAYER-1) { // for 012 downgrade
                ninlevel[p->level]--;
if(mydebug) 
{ cprintf("decrease level[%d]: time slice used up\n",p->level); }
                p->level++;
                ninlevel[p->level]++;
if(mydebug)
{ cprintf("increase level[%d]: downgrade from upper level\n",p->level); }
             //   p->timeticks[p->level] = mlpq_timeticks[p->level];
            }
            // for all proc, give new tt
        }


if(mydebug){
      cprintf(" before run %d %d %d %d\n",ninlevel[0],ninlevel[1],ninlevel[2],
                    ninlevel[3]);
      cprintf("about to run %s [pid %d] [lvl %d] [tt %d]\n",
                    proc->name, proc->pid,proc->level,
                    proc->timeticks[proc->level]);
}
      // haiyun
      swtch(&cpu->scheduler, proc->context);
      switchkvm();


      // Process is done running for now.
      // It should have changed its p->state before coming back.

//      if ( proc->state == SLEEPING || proc->state == ZOMBIE ) {
//          ninlevel[proc->level]--;
//    } else if ( proc->state == RUNNABLE ) {
//        ninlevel[proc->level]++; // back to runnable from running
//      }

      proc = 0;

    }

}  // haiyun loop nlayer
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  // ninlevel[proc->level]++; // haiyun: new runnable proc
if(mydebug)
{
  cprintf("Yield [pid %d] %d %s level %d timeticks used %d\n",
            proc->pid,
            proc->state,
            proc->name,
            proc->level,
            proc->timeticks[proc->level]);
  cprintf("on yield %d %d %d %d\n",ninlevel[0],ninlevel[1],ninlevel[2],
            ninlevel[3]);
}
  // haiyun end yield

  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  // haiyun when proc is sleeping, should not count in any level
  ninlevel[proc->level]--;
if(mydebug){
  cprintf("decrease level[%d]: go sleep\n",proc->level);
  cprintf("[pid %d] goes sleep\n",proc->pid);
  cprintf("after [pid %d] sleep %d %d %d %d\n",proc->pid,
            ninlevel[0],ninlevel[1],ninlevel[2],ninlevel[3]);
}
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan) {
      p->state = RUNNABLE;
      // haiyun when proc wakeup, go to corresponding level
      ninlevel[p->level]++;
if(mydebug){
    cprintf("[pid %d] waked up\n",p->pid);
    cprintf("increase level[%d]: from wake up\n",p->level);
    cprintf("after [pid %d] waked up %d %d %d %d\n",p->pid,
            ninlevel[0],ninlevel[1],ninlevel[2],ninlevel[3]);
}
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING) {
        p->state = RUNNABLE;
        ninlevel[p->level]++;// haiyun when proc wakeup, go to corresponding level
if(mydebug){
    cprintf("increase level[%d]: waked up in kill method\n",p->level);
}
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}


void // haiyun check starvation
check_starve_and_boost(void) {
  struct proc *p;
  int i ;
  acquire(&ptable.lock); 
  for ( i = 0 ; i < NPROC ; ++i) {
    p = &ptable.proc[i];
    if ( p->state != RUNNABLE )
      continue;

    if (  p->level != 0 // skip level 0
       && p->level == laststate.level[i] // same level  
       && p->timeticks[p->level] == laststate.timeticks[i][p->level]) {// same ttick
    // STARVATION HAPPEN!
    // boost up 1 level
      ninlevel[p->level]--;
      p->level--;
      p->timeticks[p->level] = mlpq_timeticks[p->level];
      ninlevel[p->level]++;
    }
    // update lastestate array
    laststate.level[i] = p->level;
    laststate.timeticks[i][p->level] = p->timeticks[p->level];
  }
  release(&ptable.lock); 
}


// haiyun: update the pstat content passed by user
int
getpinfo(struct pstat* pstat) {
    int i;
    for (i = 0 ; i < NPROC ; ++i ) {
         // if ( ptable.proc[i].state == UNUSED ) {
         //     pstat->inuse[i] = 0;
         // } else {
         //     pstat->inuse[i] = 1;
         // } // this 5 lines can be simplified as next line, since unused=0
        pstat->inuse[i] = (ptable.proc[i]).state;
        if ( pstat->inuse[i]) {
            pstat->pid[i]      = ptable.proc[i].pid;
            pstat->priority[i] = ptable.proc[i].level;
            pstat->state[i]    = ptable.proc[i].state;
            // need to pass the total ticks in each level
            // haiyun todo
            int ii;
            for ( ii = 0 ; ii < NLAYER; ++ ii) {
                pstat->ticks[i][ii] = ptable.proc[i].timeticks[ii];
            }
         }
     }
    return 0;
}


// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    // cprintf("%d %s %s", p->pid, state, p->name);
    cprintf("%d %s %s lvl %d ttl %d", p->pid, state, p->name,
            p->level, p->timeticks[p->level]); //haiyun
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}


