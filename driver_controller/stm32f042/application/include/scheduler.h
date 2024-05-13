/*
 * Scheduler.h
 *
 *  Created on: May 10, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_SRC_SCHEDULER_H_
#define APPLICATION_SRC_SCHEDULER_H_

#include <list>

class Task
{

public:
  Task(void(*task)(void),unsigned int period)
  {
    this->task= task;
    this->period=period;

    cntr=0;
  }

  virtual ~Task(){};

  unsigned int
  getCntr () const
  {
    return cntr;
  }

  void
  setCntr (unsigned int cntr)
  {
    this->cntr = cntr;
  }

  unsigned int
  getPeriod () const
  {
    return period;
  }

  void (*
  getTask () const )(void)
  {
    return task;
  }

private:

  unsigned int cntr;
  unsigned int period;
  void (*task)(void);

};


class Scheduler
{
public:

  typedef enum
  {
    TASK_RUNNING,
    TASK_STOP,
    MAX_TASK_STATES,

  }STATE_TYPE;

  Scheduler();
  virtual
  ~Scheduler ();

  void running(void);
  void stop(void);
  void exec(void);
  void add_task(Task &task);

private:
  std::list<Task>task_list;
  STATE_TYPE state;
  void (Scheduler::*state_func[MAX_TASK_STATES])(void)={nullptr};
  volatile bool scheduler_tick;

};

#endif /* APPLICATION_SRC_SCHEDULER_H_ */
