/*
 * Scheduler.cpp
 *
 *  Created on: May 10, 2024
 *      Author: carlo
 */

#include <scheduler.h>


Scheduler::Scheduler ()
{
  state=TASK_RUNNING;
  state_func[TASK_RUNNING]=&Scheduler::running;
  state_func[TASK_STOP]=&Scheduler::stop;
  scheduler_tick = true;
}

Scheduler::~Scheduler ()
{
  // TODO Auto-generated destructor stub
}

void
Scheduler::running (void)
{

  unsigned int _cntr=0;

  for(std::list<Task>::iterator it=task_list.begin();
        it!=task_list.end();++it)
  {
    if((_cntr=it->getCntr())>0)
      it->setCntr(_cntr--);
    else
    {
      (it->getTask())();
      it->setCntr(it->getPeriod());
    }
  }
}

void
Scheduler::add_task(Task &task)
{
  task_list.push_back(task);
}

void Scheduler::stop (void)
{
}

void Scheduler::exec(void)
{

  typedef void (Scheduler::*_task_ptr)(void);
  Scheduler *temp= new Scheduler();

  _task_ptr _task=nullptr;

  while(1)
  {
    if(scheduler_tick)
    {
        _task=Scheduler::state_func[state];
        if(_task != nullptr)
        {
            temp->*_task_ptr();
        }
    }
  }
}
