/******************************************************************************
    Copyright � 2012-2015 Martin Karsten

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "runtime/RuntimeImpl.h"
#include "runtime/Scheduler.h"
#include "runtime/Stack.h"
#include "runtime/Thread.h"
#include "kernel/Output.h"


//added by James*********
	mword Scheduler::defaultEpochSize;
	mword Scheduler::schedMinGranularity; 
//*************************



Scheduler::Scheduler() : readyCount(0), preemption(0), resumption(0), partner(this),totalPriority(0),prevTSC(0),currTSC(0){
  Thread* idleThread = Thread::create((vaddr)idleStack, minimumStack);
	totalPriority += idleThread->getPriority();
  idleThread->setAffinity(this)->setPriority(idlePriority);
  // use low-level routines, since runtime context might not exist
  idleThread->stackPointer = stackInit(idleThread->stackPointer, &Runtime::getDefaultMemoryContext(), (ptr_t)Runtime::idleLoop, this, nullptr, nullptr);


// ********************************************************************************//
  // Reference to ready queue, changing to use AVLTree: *************  Brad
  //readyQueue[idlePriority].push_back(*idleThread);
  readyQueue.insert(*idleThread);
// ********************************************************************************//
	
  readyCount += 1;
}




static inline void unlock() {}




template<typename... Args>
static inline void unlock(BasicLock &l, Args&... a) {
  l.release();
  unlock(a...);
}

// very simple N-class prio scheduling!
template<typename... Args>


/*
*There are two parts to are modified switchThread
******part A)find next thread
*pops the minNode from our AVL tree 
*sets the minimum Virtual Time to the nodes virtual time
*changes the nodes virtual time to 0.
*decrements the AVL tree readyCount by one and the totalPrioity by node->priority
*recalculate the epoch
*goes to part B
******Part B)shifting
*set time served to 0
*calculates the timesSlice to be given to the task
*takes a TSC snapshot to be used in preempt
*set the task to be executed
*/
inline void Scheduler::switchThread(Scheduler* target, Args&... a) {
  preemption += 1;
  CHECK_LOCK_MIN(sizeof...(Args));
  Thread* nextThread;
  readyLock.acquire();
  for (mword i = 0; i < (target ? idlePriority : maxPriority); i += 1) {
    // **************************************************************************//
    if (!readyQueue.empty()) {
      //nextThread = readyQueue[i].pop_front(); //Reference to readyQueue, changing to use AVL tree
      nextThread = readyQueue.popMin(); 
      // ************************************************************************//
      readyCount -= 1;
//***********James 4b change total priority calc epoch size
		minimumVirtualTime = nextThread->getVR();
		nextThread->setVR(0);
 		totalPriority -= nextThread->getPriority();
		calculateEpochSize();
//***********************************************************

      goto threadFound;
    }
  }
  readyLock.release();
  GENASSERT0(target);
  GENASSERT0(!sizeof...(Args));
  return;                                         // return to current thread

threadFound:
  readyLock.release();
  resumption += 1;
  Thread* currThread = Runtime::getCurrThread();
  GENASSERTN(currThread && nextThread && nextThread != currThread, currThread, ' ', nextThread);

  if (target) currThread->nextScheduler = target; // yield/preempt to given processor
  else currThread->nextScheduler = this;          // suspend/resume to same processor
  unlock(a...);                                   // ...thus can unlock now
  CHECK_LOCK_COUNT(1);
  Runtime::debugS("Thread switch <", (target ? 'Y' : 'S'), ">: ", FmtHex(currThread), '(', FmtHex(currThread->stackPointer), ") to ", FmtHex(nextThread), '(', FmtHex(nextThread->stackPointer), ')');

  Runtime::MemoryContext& ctx = Runtime::getMemoryContext();
//***************************************
	timeServed = 0;
	timeSlice = (epochSize * nextThread->getPriority())/(totalPriority+nextThread->getPriority());
	prevTSC = CPU::readTSC();
//********************************************
  Runtime::setCurrThread(nextThread);
  Thread* prevThread = stackSwitch(currThread, target, &currThread->stackPointer, nextThread->stackPointer);
  // REMEMBER: Thread might have migrated from other processor, so 'this'
  //           might not be currThread's Scheduler object anymore.
  //           However, 'this' points to prevThread's Scheduler object.
  Runtime::postResume(false, *prevThread, ctx);
  if (currThread->state == Thread::Cancelled) {
    currThread->state = Thread::Finishing;
    switchThread(nullptr);
    unreachable();
  }
}


extern "C" Thread* postSwitch(Thread* prevThread, Scheduler* target) {
  CHECK_LOCK_COUNT(1);
  if fastpath(target) Scheduler::resume(*prevThread);
  return prevThread;
}

extern "C" void invokeThread(Thread* prevThread, Runtime::MemoryContext* ctx, funcvoid3_t func, ptr_t arg1, ptr_t arg2, ptr_t arg3) {
  Runtime::postResume(true, *prevThread, *ctx);
  func(arg1, arg2, arg3);
  Runtime::getScheduler()->terminate();
}


/*
*the original enqueue has been modified to push to our AVL Tree rather than the queue 
*we have also modified it to check the minimum virtual time of the scheduler AVL tree being enqueue *to incase the task is far behind and will be given unfair execution time trying to catch up. 
*our enqueue also incements the readyCount aswell as the totalPriorit of the tree. 
*the Epoch size is then recalculated.
*/
void Scheduler::enqueue(Thread& t) {
  GENASSERT1(t.priority < maxPriority, t.priority);
  readyLock.acquire();
  // ***************************************************************************** //
  // Replace with AVLTree implementation

  t.incrementVR(); // increment the virtual runtime for thread 't'

  //readyQueue[t.priority].push_back(t);
  // AVL Tree implementation:
  readyQueue.insert(t);
  // ***************************************************************************** //
  bool wake = (readyCount == 0);


	mword adjustedVR = t.getVR()+ minimumVirtualTime;
	t.setVR(adjustedVR);
  totalPriority += t.getPriority();
  readyCount += 1;
  calculateEpochSize();
  readyLock.release();
  Runtime::debugS("Thread ", FmtHex(&t), " queued on ", FmtHex(this));
  if (wake) Runtime::wakeUp(this);
}

void Scheduler::resume(Thread& t) {
  GENASSERT1(&t != Runtime::getCurrThread(), Runtime::getCurrThread());
  if (t.nextScheduler) t.nextScheduler->enqueue(t);
  else Runtime::getScheduler()->enqueue(t);
}




/*
*Preempt is called evertime there is an interupt 
*our implementation of preempt calculates the difference between the TSC snapshot
*taken at last interupt and the current TSC snapshot if the difference plus the 
*previous time served is equal to the time slice given to the task 
*a scheduler is selected based on affinity and switchThread is called.
*else a new TSC snapshot is taken and the task is allowed to continue
*/
void Scheduler::preempt() {               // IRQs disabled, lock count inflated
//*******************Added by James ...maybe kinda right? -> RIGHT! -.-  ********************
	currTSC = CPU::readTSC();
	mword diff = currTSC - prevTSC;
	timeServed += diff;
	Thread * currentThread = Runtime::getCurrThread();
	mword VRuntime = currentThread->getVR()+ (diff/currentThread->getPriority());
	Runtime::getCurrThread()->setVR(VRuntime);
	if(timeServed >= timeSlice){
				//**choosing which scheduler to run on taken from their preemt code**
		Scheduler* target = Runtime::getCurrThread()->getAffinity();
		if (!target) target = (partner->readyCount + 2 < readyCount) ? partner : this;
		switchThread(target);
	}
	else //keep executing current thread
		prevTSC = CPU::readTSC();

//***********************************

}

void Scheduler::suspend(BasicLock& lk) {
  Runtime::FakeLock fl;
  switchThread(nullptr, lk);
}

void Scheduler::suspend(BasicLock& lk1, BasicLock& lk2) {//when something goes ot sleep
  Runtime::FakeLock fl;
  switchThread(nullptr, lk1, lk2);
}

void Scheduler::terminate() {//called when thread completely done
  Runtime::RealLock rl;
  Thread* thr = Runtime::getCurrThread();
  GENASSERT1(thr->state != Thread::Blocked, thr->state);
  thr->state = Thread::Finishing;
  switchThread(nullptr);
  unreachable();
}

/**********written for Assignment4b*********
*calculate the epoch size to be used by the schduler. 
*if the calculated epoch size needs to be larger than the default us that
*else use the default epoch Length
*/
void Scheduler::calculateEpochSize(){

	mword temp = (readyCount +1) * schedMinGranularity;
	if(temp>defaultEpochSize)
		epochSize=temp;
	else
		epochSize = defaultEpochSize;
}
