#ifdef SYSTEM_CPP

Scheduler scheduler;

void Scheduler::enter() {
  host_thread = co_active();
  co_switch(thread);
}

void Scheduler::exit(ExitReason reason) {
  exit_reason = reason;
  thread = co_active();
  co_switch(host_thread);
}

void Scheduler::resume(cothread_t& thread) {
  if (mode == Mode::Synchronize)
    desynchronized = true;
  co_switch(thread);
}

void Scheduler::init() {
  host_thread = co_active();
  thread = cpu.thread;
  desynchronized = false;
  mode = Mode::Synchronize;
}

Scheduler::Scheduler() {
  host_thread = 0;
  thread = 0;
  desynchronized = false;
  exit_reason = ExitReason::UnknownEvent;
}

#endif
