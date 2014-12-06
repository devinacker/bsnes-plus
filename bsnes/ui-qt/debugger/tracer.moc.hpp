class Tracer : public QObject {
  Q_OBJECT

public:
  void stepCpu();
  void stepSmp();
  void stepSa1();

  Tracer();
  ~Tracer();

public slots:
  void setCpuTraceState(int);
  void setSmpTraceState(int);
  void setSa1TraceState(int);
  void setTraceMaskState(int);

private:
  file tracefile;
  bool traceCpu;
  bool traceSmp;
  bool traceSa1;
  bool traceMask;

  uint8_t *traceMaskCPU;
  uint8_t *traceMaskSMP;
  uint8_t *traceMaskSA1;
};

extern Tracer *tracer;
