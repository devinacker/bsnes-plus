struct Scheduler : property<Scheduler> {
  enum class Mode : unsigned { Run, Synchronize } mode;
  enum class ExitReason : unsigned { UnknownEvent, FrameEvent, SynchronizeEvent, DesynchronizeEvent, DebuggerEvent };
  readonly<ExitReason> exit_reason;

  cothread_t host_thread;  //program thread (used to exit emulation)
  cothread_t thread;       //active emulation thread (used to enter emulation)
  bool desynchronized;

  void enter();
  void exit(ExitReason);
  void resume(cothread_t& thread);

  inline bool synchronizing() const { return mode == Mode::Synchronize; }
  inline void synchronize() {
    if (mode == Mode::Synchronize) {
      if (desynchronized) {
        desynchronized = false;
        exit(ExitReason::DesynchronizeEvent);
      } else {
        exit(ExitReason::SynchronizeEvent);
      }
    }
  }
  inline void desynchronize() { desynchronized = true; }

  void init();
  Scheduler();
};

extern Scheduler scheduler;
