class Tracer : public QObject {
  Q_OBJECT

public:
  void stepCpu();
  void stepSmp();
  void stepSa1();
  void stepSfx();
  void stepSgb();

  Tracer();
  ~Tracer();

public slots:
  void setCpuTraceState(int);
  void setSmpTraceState(int);
  void setSa1TraceState(int);
  void setSfxTraceState(int);
  void setSgbTraceState(int);
  void setTraceMaskState(bool);

  void resetTraceState();

private:
  void setTraceState(bool);

  file tracefile;
  bool traceCpu;
  bool traceSmp;
  bool traceSa1;
  bool traceSfx;
  bool traceSgb;
  bool traceMask;

  uint8_t *traceMaskCPU;
  uint8_t *traceMaskSMP;
  uint8_t *traceMaskSA1;
  uint8_t *traceMaskSFX;
  uint8_t *traceMaskSGB;
};

extern Tracer *tracer;
