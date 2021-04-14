#include "gen-cpp/IdaClient.h"
#include "gen-cpp/BsnesDebugger.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/concurrency/ThreadFactory.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

#include <ida.hpp>
#include <dbg.hpp>
#include <auto.hpp>
#include <deque>
#include <mutex>

#include "ida_plugin.h"
#include "ida_debmod.h"
#include "ida_registers.h"

::std::shared_ptr<BsnesDebuggerClient> client;
::std::shared_ptr<TNonblockingServer> srv;
::std::shared_ptr<TTransport> cli_transport;

static ::std::mutex list_mutex;
static eventlist_t events;

static register_info_t registers[] = {
	{"A", 0, RC_CPU, dt_word, NULL, 0},
	{"X", 0, RC_CPU, dt_word, NULL, 0},
	{"Y", 0, RC_CPU, dt_word, NULL, 0},

	{"D", 0, RC_CPU, dt_word, NULL, 0},
	{"DB", 0, RC_CPU, dt_byte, NULL, 0},

	{"PC", REGISTER_IP | REGISTER_ADDRESS, RC_CPU, dt_dword, NULL, 0},
  {"S", REGISTER_SP | REGISTER_ADDRESS, RC_CPU, dt_word, NULL, 0},

	{"P", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0},
  {"m", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0},
  {"x", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0},
	{"e", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0},
};

static const char* register_classes[] = {
	"General Registers",
	NULL
};

static struct apply_codemap_req : public exec_request_t {
private:
  const std::vector<int32_t>& _changed;
public:
  apply_codemap_req(const std::vector<int32_t>& changed) : _changed(changed) {};

  int idaapi execute(void) override {
    for (auto i = _changed.cbegin(); i != _changed.cend(); ++i) {
      ea_t addr = (ea_t)(*i | 0x800000);
      auto_make_code(addr);
      plan_ea(addr);
      show_addr(addr);
    }

    return 0;
  }
};

static void apply_codemap(const std::vector<int32_t>& changed)
{
  if (changed.empty()) return;

  apply_codemap_req req(changed);
  execute_sync(req, MFF_FAST);
}

static void pause_execution()
{
  try {
    if (client) {
      client->pause();
    }
  }
  catch (...) {

  }
}

static void continue_execution()
{
  try {
    if (client) {
      client->resume();
    }
  }
  catch (...) {

  }
}

static void finish_execution()
{
  try {
    if (client) {
      client->exit_emulation();
    }
  }
  catch (...) {

  }
}

void stop_server() {
  try {
    srv->stop();
  }
  catch (...) {

  }
}

class IdaClientHandler : virtual public IdaClientIf {
	
public:
	void pause_event(const int32_t address) override {
    ::std::lock_guard<::std::mutex> lock(list_mutex);

    debug_event_t ev;
    ev.pid = 1;
    ev.tid = 1;
    ev.ea = address | 0x800000;
    ev.handled = true;
    ev.set_eid(PROCESS_SUSPENDED);
    events.enqueue(ev, IN_BACK);
	}


	void start_event() override {
    ::std::lock_guard<::std::mutex> lock(list_mutex);

    debug_event_t ev;
    ev.pid = 1;
    ev.tid = 1;
    ev.ea = BADADDR;
    ev.handled = true;

    ev.set_modinfo(PROCESS_STARTED).name.sprnt("BSNES");
    ev.set_modinfo(PROCESS_STARTED).base = 0;
    ev.set_modinfo(PROCESS_STARTED).size = 0;
    ev.set_modinfo(PROCESS_STARTED).rebase_to = BADADDR;

    events.enqueue(ev, IN_BACK);
	}


	void stop_event() override {
    ::std::lock_guard<::std::mutex> lock(list_mutex);

    debug_event_t ev;
    ev.pid = 1;
    ev.handled = true;
    ev.set_exit_code(PROCESS_EXITED, 0);

    events.enqueue(ev, IN_BACK);
	}


  void add_visited(const std::vector<int32_t>& changed) override {
    apply_codemap(changed);
  }

};

static void init_ida_server() {
	try {
    ::std::shared_ptr<IdaClientHandler> handler(new IdaClientHandler());
    ::std::shared_ptr<TProcessor> processor(new IdaClientProcessor(handler));
    ::std::shared_ptr<TNonblockingServerTransport> serverTransport(new TNonblockingServerSocket(9091));
    ::std::shared_ptr<TFramedTransportFactory> transportFactory(new TFramedTransportFactory());
    ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    srv = ::std::shared_ptr<TNonblockingServer>(new TNonblockingServer(processor, protocolFactory, serverTransport));
    ::std::shared_ptr<ThreadFactory> tf(new ThreadFactory());
    ::std::shared_ptr<Thread> thread = tf->newThread(srv);
    thread->start();
	} catch (...) {
		
	}
}

static void init_emu_client() {
  ::std::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 9090));
  cli_transport = ::std::shared_ptr<TTransport>(new TFramedTransport(socket));
  ::std::shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(cli_transport));
  client = ::std::shared_ptr<BsnesDebuggerClient>(new BsnesDebuggerClient(protocol));

  show_wait_box("Waiting for BSNES-PLUS emulation...");

  while (true) {
    if (user_cancelled()) {
      break;
    }

    try {
      cli_transport->open();
      break;
    }
    catch (...) {

    }
  }

  hide_wait_box();
}


// Initialize debugger
// Returns true-success
// This function is called from the main thread
static drc_t idaapi init_debugger(const char* hostname, int portnum, const char* password, qstring* errbuf)
{
  return DRC_OK;
}

// Terminate debugger
// Returns true-success
// This function is called from the main thread
static drc_t idaapi term_debugger(void)
{
  finish_execution();
  return DRC_OK;
}

// Return information about the n-th "compatible" running process.
// If n is 0, the processes list is reinitialized.
// 1-ok, 0-failed, -1-network error
// This function is called from the main thread
static drc_t s_get_processes(procinfo_vec_t* procs, qstring* errbuf) {
  process_info_t info;
  info.name.sprnt("bsnes");
  info.pid = 1;
  procs->add(info);

  return DRC_OK;
}

// Start an executable to debug
// 1 - ok, 0 - failed, -2 - file not found (ask for process options)
// 1|CRC32_MISMATCH - ok, but the input file crc does not match
// -1 - network error
// This function is called from debthread
static drc_t idaapi s_start_process(const char* path,
  const char* args,
  const char* startdir,
  uint32 dbg_proc_flags,
  const char* input_path,
  uint32 input_file_crc32,
  qstring* errbuf = NULL)
{
  ::std::lock_guard<::std::mutex> lock(list_mutex);
  events.clear();

  init_ida_server();
  init_emu_client();

  try {
    if (client) {
      client->start_emulation();
    }
  }
  catch (...) {

  }

  return DRC_OK;
}

// Prepare to pause the process
// This function will prepare to pause the process
// Normally the next get_debug_event() will pause the process
// If the process is sleeping then the pause will not occur
// until the process wakes up. The interface should take care of
// this situation.
// If this function is absent, then it won't be possible to pause the program
// 1-ok, 0-failed, -1-network error
// This function is called from debthread
static drc_t idaapi prepare_to_pause_process(qstring* errbuf)
{
  pause_execution();
  return DRC_OK;
}

// Stop the process.
// May be called while the process is running or suspended.
// Must terminate the process in any case.
// The kernel will repeatedly call get_debug_event() and until PROCESS_EXIT.
// In this mode, all other events will be automatically handled and process will be resumed.
// 1-ok, 0-failed, -1-network error
// This function is called from debthread
static drc_t idaapi emul_exit_process(qstring* errbuf)
{
  finish_execution();

  return DRC_OK;
}

// Get a pending debug event and suspend the process
// This function will be called regularly by IDA.
// This function is called from debthread
static gdecode_t idaapi get_debug_event(debug_event_t* event, int timeout_ms)
{
  while (true)
  {
    ::std::lock_guard<::std::mutex> lock(list_mutex);

    // are there any pending events?
    if (events.retrieve(event))
    {
      return events.empty() ? GDE_ONE_EVENT : GDE_MANY_EVENTS;
    }
    if (events.empty())
      break;
  }
  return GDE_NO_EVENT;
}

// Continue after handling the event
// 1-ok, 0-failed, -1-network error
// This function is called from debthread
static drc_t idaapi continue_after_event(const debug_event_t* event)
{
  dbg_notification_t req = get_running_notification();
  switch (event->eid())
  {
  case STEP:
  case PROCESS_SUSPENDED:
    if (req == dbg_null || req == dbg_run_to) {
      continue_execution();
    }
    break;
  case PROCESS_EXITED:
    stop_server();
    break;
  }

  return DRC_OK;
}

// The following function will be called by the kernel each time
// when it has stopped the debugger process for some reason,
// refreshed the database and the screen.
// The debugger module may add information to the database if it wants.
// The reason for introducing this function is that when an event line
// LOAD_DLL happens, the database does not reflect the memory state yet
// and therefore we can't add information about the dll into the database
// in the get_debug_event() function.
// Only when the kernel has adjusted the database we can do it.
// Example: for imported PE DLLs we will add the exported function
// names to the database.
// This function pointer may be absent, i.e. NULL.
// This function is called from the main thread
static void idaapi stopped_at_debug_event(bool dlls_added)
{
}

// The following functions manipulate threads.
// 1-ok, 0-failed, -1-network error
// These functions are called from debthread
static drc_t idaapi s_set_resume_mode(thid_t tid, resume_mode_t resmod) // Run one instruction in the thread
{
  switch (resmod)
  {
  case RESMOD_INTO:    ///< step into call (the most typical single stepping)
    try {
      if (client) {
        client->step_into();
      }
    }
    catch (...) {

    }

    break;
  case RESMOD_OVER:    ///< step over call
    try {
      if (client) {
        client->step_over();
      }
    }
    catch (...) {

    }
    break;
  }

  return DRC_OK;
}

// Read thread registers
//	tid	- thread id
//	clsmask- bitmask of register classes to read
//	regval - pointer to vector of regvals for all registers
//			 regval is assumed to have debugger_t::registers_size elements
// 1-ok, 0-failed, -1-network error
// This function is called from debthread
static drc_t idaapi read_registers(thid_t tid, int clsmask, regval_t* values, qstring* errbuf)
{
  if (clsmask & RC_CPU)
  {
		BsnesRegisters regs;

    try {
      if (client) {
        client->get_cpu_regs(regs);

				values[static_cast<int>(SNES_REGS::SR_PC)].ival = regs.pc | 0x800000;
				values[static_cast<int>(SNES_REGS::SR_A)].ival = regs.a;
				values[static_cast<int>(SNES_REGS::SR_X)].ival = regs.x;
				values[static_cast<int>(SNES_REGS::SR_Y)].ival = regs.y;
				values[static_cast<int>(SNES_REGS::SR_S)].ival = regs.s;
				values[static_cast<int>(SNES_REGS::SR_D)].ival = regs.d;
				values[static_cast<int>(SNES_REGS::SR_DB)].ival = regs.db;
				values[static_cast<int>(SNES_REGS::SR_P)].ival = regs.p;
        values[static_cast<int>(SNES_REGS::SR_MFLAG)].ival = regs.mflag;
        values[static_cast<int>(SNES_REGS::SR_XFLAG)].ival = regs.xflag;
				values[static_cast<int>(SNES_REGS::SR_EFLAG)].ival = regs.eflag;
      }
    }
    catch (...) {

    }
	}

	return DRC_OK;
}

// Write one thread register
//	tid	- thread id
//	regidx - register index
//	regval - new value of the register
// 1-ok, 0-failed, -1-network error
// This function is called from debthread
static drc_t idaapi write_register(thid_t tid, int regidx, const regval_t* value, qstring* errbuf)
{
  if (regidx >= static_cast<int>(SNES_REGS::SR_PC) && regidx <= static_cast<int>(SNES_REGS::SR_EFLAG)) {
    try {
      if (client) {
        client->set_cpu_reg(static_cast<BsnesRegister::type>(regidx), value->ival & 0xFFFFFFFF);
      }
    }
    catch (...) {

    }
	}

	return DRC_OK;

}

// The following functions manipulate bytes in the memory.
//
// Get information on the memory areas
// The debugger module fills 'areas'. The returned vector MUST be sorted.
// Returns:
//   -3: use idb segmentation
//   -2: no changes
//   -1: the process does not exist anymore
//	0: failed
//	1: new memory layout is returned
// This function is called from debthread
static drc_t idaapi get_memory_info(meminfo_vec_t& areas, qstring* errbuf)
{
  memory_info_t info;

  info.start_ea = 0x0000;
  info.end_ea = 0x01FFF;
  info.sclass = "STACK";
  info.bitness = 0;
  info.perm = SEGPERM_READ | SEGPERM_WRITE;
  areas.push_back(info);

  // Don't remove this loop
  for (int i = 0; i < get_segm_qty(); ++i)
  {
    segment_t* segm = getnseg(i);

    info.start_ea = segm->start_ea;
    info.end_ea = segm->end_ea;

    qstring buf;
    get_segm_name(&buf, segm);
    info.name = buf;

    get_segm_class(&buf, segm);
    info.sclass = buf;

    info.sbase = get_segm_base(segm);

    info.perm = segm->perm;
    info.bitness = segm->bitness;
    areas.push_back(info);
  }
  // Don't remove this loop

	return DRC_OK;
}

// Read process memory
// Returns number of read bytes
// 0 means read error
// -1 means that the process does not exist anymore
// This function is called from debthread
static ssize_t idaapi read_memory(ea_t ea, void* buffer, size_t size, qstring* errbuf)
{
  std::string mem;

  try {
    if (client) {
      client->read_memory(mem, DbgMemorySource::CPUBus, (int32_t)ea, (int32_t)size);

      memcpy(&((unsigned char*)buffer)[0], mem.c_str(), size);
    }
  }
  catch (...) {

  }

  return size;
}

// Write process memory
// Returns number of written bytes, -1-fatal error
// This function is called from debthread
static ssize_t idaapi write_memory(ea_t ea, const void* buffer, size_t size, qstring* errbuf)
{
  std::string mem((const char*)buffer);

  try {
    if (client) {
      client->write_memory(DbgMemorySource::CPUBus, (int32_t)ea, mem);
    }
  }
  catch (...) {

  }

  return size;
}

// Is it possible to set breakpoint?
// Returns: BPT_...
// This function is called from debthread or from the main thread if debthread
// is not running yet.
// It is called to verify hardware breakpoints.
static int idaapi is_ok_bpt(bpttype_t type, ea_t ea, int len)
{
  DbgMemorySource::type btype = DbgMemorySource::CPUBus;

  switch (btype) {
  case DbgMemorySource::CPUBus:
  case DbgMemorySource::APURAM:
  case DbgMemorySource::DSP:
  case DbgMemorySource::VRAM:
  case DbgMemorySource::OAM:
  case DbgMemorySource::CGRAM:
  case DbgMemorySource::SA1Bus:
  case DbgMemorySource::SFXBus:
    break;
  default:
    return BPT_BAD_TYPE;
  }

  switch (type)
  {
  case BPT_EXEC:
  case BPT_READ:
  case BPT_WRITE:
  case BPT_RDWR:
    return BPT_OK;
  }

  return BPT_BAD_TYPE;
}

// Add/del breakpoints.
// bpts array contains nadd bpts to add, followed by ndel bpts to del.
// returns number of successfully modified bpts, -1-network error
// This function is called from debthread
static drc_t idaapi update_bpts(int* nbpts, update_bpt_info_t* bpts, int nadd, int ndel, qstring* errbuf)
{
  for (int i = 0; i < nadd; ++i)
  {
    ea_t start = bpts[i].ea;
    ea_t end = bpts[i].ea + bpts[i].size - 1;

    DbgBreakpoint bp;
    bp.bstart = start;
    bp.bend = end;
    bp.enabled = true;

    switch (bpts[i].type)
    {
    case BPT_EXEC:
      bp.type = BpType::BP_PC;
      break;
    case BPT_READ:
      bp.type = BpType::BP_READ;
      break;
    case BPT_WRITE:
      bp.type = BpType::BP_WRITE;
      break;
    case BPT_RDWR:
      bp.type = BpType::BP_READ;
      break;
    }

    DbgMemorySource::type type = DbgMemorySource::CPUBus;

    switch (type) {
    case DbgMemorySource::CPUBus:
      bp.src = DbgBptSource::CPUBus;
      break;
    case DbgMemorySource::APURAM:
      bp.src = DbgBptSource::APURAM;
      break;
    case DbgMemorySource::DSP:
      bp.src = DbgBptSource::DSP;
      break;
    case DbgMemorySource::VRAM:
      bp.src = DbgBptSource::VRAM;
      break;
    case DbgMemorySource::OAM:
      bp.src = DbgBptSource::OAM;
      break;
    case DbgMemorySource::CGRAM:
      bp.src = DbgBptSource::CGRAM;
      break;
    case DbgMemorySource::SA1Bus:
      bp.src = DbgBptSource::SA1Bus;
      break;
    case DbgMemorySource::SFXBus:
      bp.src = DbgBptSource::SFXBus;
      break;
    default:
      continue;
    }

    try {
      if (client) {
        client->add_breakpoint(bp);
      }
    }
    catch (...) {

    }

    bpts[i].code = BPT_OK;
  }

  for (int i = 0; i < ndel; ++i)
  {
    ea_t start = bpts[nadd + i].ea;
    ea_t end = bpts[nadd + i].ea + bpts[nadd + i].size - 1;

    DbgBreakpoint bp;
    bp.bstart = start;
    bp.bend = end;
    bp.enabled = true;

    switch (bpts[i].type)
    {
    case BPT_EXEC:
      bp.type = BpType::BP_PC;
      break;
    case BPT_READ:
      bp.type = BpType::BP_READ;
      break;
    case BPT_WRITE:
      bp.type = BpType::BP_WRITE;
      break;
    case BPT_RDWR:
      bp.type = BpType::BP_READ;
      break;
    }

    DbgMemorySource::type type = DbgMemorySource::CPUBus;

    switch (type) {
    case DbgMemorySource::CPUBus:
      bp.src = DbgBptSource::CPUBus;
      break;
    case DbgMemorySource::APURAM:
      bp.src = DbgBptSource::APURAM;
      break;
    case DbgMemorySource::DSP:
      bp.src = DbgBptSource::DSP;
      break;
    case DbgMemorySource::VRAM:
      bp.src = DbgBptSource::VRAM;
      break;
    case DbgMemorySource::OAM:
      bp.src = DbgBptSource::OAM;
      break;
    case DbgMemorySource::CGRAM:
      bp.src = DbgBptSource::CGRAM;
      break;
    case DbgMemorySource::SA1Bus:
      bp.src = DbgBptSource::SA1Bus;
      break;
    case DbgMemorySource::SFXBus:
      bp.src = DbgBptSource::SFXBus;
      break;
    default:
      continue;
    }

    try {
      if (client) {
        client->del_breakpoint(bp);
      }
    }
    catch (...) {

    }

    bpts[nadd + i].code = BPT_OK;
  }

  *nbpts = (ndel + nadd);
  return DRC_OK;
}

static ssize_t idaapi idd_notify(void*, int msgid, va_list va) {
  drc_t retcode = DRC_NONE;
  qstring* errbuf;

  switch (msgid)
  {
  case debugger_t::ev_init_debugger:
  {
    const char* hostname = va_arg(va, const char*);

    int portnum = va_arg(va, int);
    const char* password = va_arg(va, const char*);
    errbuf = va_arg(va, qstring*);
    QASSERT(1522, errbuf != NULL);
    retcode = init_debugger(hostname, portnum, password, errbuf);
  }
  break;

  case debugger_t::ev_term_debugger:
    retcode = term_debugger();
    break;

  case debugger_t::ev_get_processes:
  {
    procinfo_vec_t* procs = va_arg(va, procinfo_vec_t*);
    errbuf = va_arg(va, qstring*);
    retcode = s_get_processes(procs, errbuf);
  }
  break;

  case debugger_t::ev_start_process:
  {
    const char* path = va_arg(va, const char*);
    const char* args = va_arg(va, const char*);
    const char* startdir = va_arg(va, const char*);
    uint32 dbg_proc_flags = va_arg(va, uint32);
    const char* input_path = va_arg(va, const char*);
    uint32 input_file_crc32 = va_arg(va, uint32);
    errbuf = va_arg(va, qstring*);
    retcode = s_start_process(path,
      args,
      startdir,
      dbg_proc_flags,
      input_path,
      input_file_crc32,
      errbuf);
  }
  break;

  //case debugger_t::ev_attach_process:
  //{
  //    pid_t pid = va_argi(va, pid_t);
  //    int event_id = va_arg(va, int);
  //    uint32 dbg_proc_flags = va_arg(va, uint32);
  //    errbuf = va_arg(va, qstring*);
  //    retcode = s_attach_process(pid, event_id, dbg_proc_flags, errbuf);
  //}
  //break;

  //case debugger_t::ev_detach_process:
  //    retcode = g_dbgmod.dbg_detach_process();
  //    break;

  case debugger_t::ev_get_debapp_attrs:
  {
    debapp_attrs_t* out_pattrs = va_arg(va, debapp_attrs_t*);
    out_pattrs->addrsize = 3;
    out_pattrs->is_be = false;
    out_pattrs->platform = "snes";
    out_pattrs->cbsize = sizeof(debapp_attrs_t);
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_rebase_if_required_to:
  {
    ea_t new_base = va_arg(va, ea_t);
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_request_pause:
    errbuf = va_arg(va, qstring*);
    retcode = prepare_to_pause_process(errbuf);
    break;

  case debugger_t::ev_exit_process:
    errbuf = va_arg(va, qstring*);
    retcode = emul_exit_process(errbuf);
    break;

  case debugger_t::ev_get_debug_event:
  {
    gdecode_t* code = va_arg(va, gdecode_t*);
    debug_event_t* event = va_arg(va, debug_event_t*);
    int timeout_ms = va_arg(va, int);
    *code = get_debug_event(event, timeout_ms);
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_resume:
  {
    debug_event_t* event = va_arg(va, debug_event_t*);
    retcode = continue_after_event(event);
  }
  break;

  //case debugger_t::ev_set_exception_info:
  //{
  //    exception_info_t* info = va_arg(va, exception_info_t*);
  //    int qty = va_arg(va, int);
  //    g_dbgmod.dbg_set_exception_info(info, qty);
  //    retcode = DRC_OK;
  //}
  //break;

  //case debugger_t::ev_suspended:
  //{
  //    bool dlls_added = va_argi(va, bool);
  //    thread_name_vec_t* thr_names = va_arg(va, thread_name_vec_t*);
  //    retcode = DRC_OK;
  //}
  //break;

  case debugger_t::ev_thread_suspend:
  {
    thid_t tid = va_argi(va, thid_t);
    pause_execution();
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_thread_continue:
  {
    thid_t tid = va_argi(va, thid_t);
    continue_execution();
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_set_resume_mode:
  {
    thid_t tid = va_argi(va, thid_t);
    resume_mode_t resmod = va_argi(va, resume_mode_t);
    retcode = s_set_resume_mode(tid, resmod);
  }
  break;

  case debugger_t::ev_read_registers:
  {
    thid_t tid = va_argi(va, thid_t);
    int clsmask = va_arg(va, int);
    regval_t* values = va_arg(va, regval_t*);
    errbuf = va_arg(va, qstring*);
    retcode = read_registers(tid, clsmask, values, errbuf);
  }
  break;

  case debugger_t::ev_write_register:
  {
    thid_t tid = va_argi(va, thid_t);
    int regidx = va_arg(va, int);
    const regval_t* value = va_arg(va, const regval_t*);
    errbuf = va_arg(va, qstring*);
    retcode = write_register(tid, regidx, value, errbuf);
  }
  break;

  case debugger_t::ev_get_memory_info:
  {
    meminfo_vec_t* ranges = va_arg(va, meminfo_vec_t*);
    errbuf = va_arg(va, qstring*);
    retcode = get_memory_info(*ranges, errbuf);
  }
  break;

  case debugger_t::ev_read_memory:
  {
    size_t* nbytes = va_arg(va, size_t*);
    ea_t ea = va_arg(va, ea_t);
    void* buffer = va_arg(va, void*);
    size_t size = va_arg(va, size_t);
    errbuf = va_arg(va, qstring*);
    ssize_t code = read_memory(ea, buffer, size, errbuf);
    *nbytes = code >= 0 ? code : 0;
    retcode = code >= 0 ? DRC_OK : DRC_NOPROC;
  }
  break;

  case debugger_t::ev_write_memory:
  {
    size_t* nbytes = va_arg(va, size_t*);
    ea_t ea = va_arg(va, ea_t);
    const void* buffer = va_arg(va, void*);
    size_t size = va_arg(va, size_t);
    errbuf = va_arg(va, qstring*);
    ssize_t code = write_memory(ea, buffer, size, errbuf);
    *nbytes = code >= 0 ? code : 0;
    retcode = code >= 0 ? DRC_OK : DRC_NOPROC;
  }
  break;

  case debugger_t::ev_check_bpt:
  {
    int* bptvc = va_arg(va, int*);
    bpttype_t type = va_argi(va, bpttype_t);
    ea_t ea = va_arg(va, ea_t);
    int len = va_arg(va, int);
    *bptvc = is_ok_bpt(type, ea, len);
    retcode = DRC_OK;
  }
  break;

  case debugger_t::ev_update_bpts:
  {
    int* nbpts = va_arg(va, int*);
    update_bpt_info_t* bpts = va_arg(va, update_bpt_info_t*);
    int nadd = va_arg(va, int);
    int ndel = va_arg(va, int);
    errbuf = va_arg(va, qstring*);
    retcode = update_bpts(nbpts, bpts, nadd, ndel, errbuf);
  }
  break;

  //case debugger_t::ev_update_lowcnds:
  //{
  //  int* nupdated = va_arg(va, int*);
  //  const lowcnd_t* lowcnds = va_arg(va, const lowcnd_t*);
  //  int nlowcnds = va_arg(va, int);
  //  errbuf = va_arg(va, qstring*);
  //  retcode = update_lowcnds(nupdated, lowcnds, nlowcnds, errbuf);
  //}
  //break;

  //case debugger_t::ev_eval_lowcnd:
  //{
  //    thid_t tid = va_argi(va, thid_t);
  //    ea_t ea = va_arg(va, ea_t);
  //    errbuf = va_arg(va, qstring*);
  //    retcode = g_dbgmod.dbg_eval_lowcnd(tid, ea, errbuf);
  //}
  //break;

  //case debugger_t::ev_bin_search:
  //{
  //    ea_t* ea = va_arg(va, ea_t*);
  //    ea_t start_ea = va_arg(va, ea_t);
  //    ea_t end_ea = va_arg(va, ea_t);
  //    const compiled_binpat_vec_t* ptns = va_arg(va, const compiled_binpat_vec_t*);
  //    int srch_flags = va_arg(va, int);
  //    errbuf = va_arg(va, qstring*);
  //    if (ptns != NULL)
  //        retcode = g_dbgmod.dbg_bin_search(ea, start_ea, end_ea, *ptns, srch_flags, errbuf);
  //}
  //break;
  default:
    retcode = DRC_NONE;
  }

  return retcode;
}

debugger_t debugger{
	IDD_INTERFACE_VERSION,
	NAME,
	0x8000 + 6581, // (6)
	"65816",

	DBG_FLAG_NOHOST | DBG_FLAG_CAN_CONT_BPT | DBG_FLAG_SAFE | DBG_FLAG_FAKE_ATTACH | DBG_FLAG_NOPASSWORD |
	DBG_FLAG_NOSTARTDIR | DBG_FLAG_NOPARAMETERS | DBG_FLAG_ANYSIZE_HWBPT | DBG_FLAG_DEBTHREAD | DBG_FLAG_PREFER_SWBPTS,
	DBG_HAS_GET_PROCESSES | DBG_HAS_REQUEST_PAUSE | DBG_HAS_SET_RESUME_MODE | DBG_HAS_THREAD_SUSPEND | DBG_HAS_THREAD_CONTINUE | DBG_HAS_CHECK_BPT,

	register_classes,
	RC_CPU,
	registers,
	qnumber(registers),

	0x1000,

	NULL,
	0,
	0,

	DBG_RESMOD_STEP_INTO | DBG_RESMOD_STEP_OVER,

	NULL,
	idd_notify
};
