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

#include "../ui-base.hpp"

static ::std::shared_ptr<IdaClientClient> client;
static ::std::shared_ptr<TNonblockingServer> srv;
static ::std::shared_ptr<TTransport> cli_transport;

static ::std::mutex list_mutex;
::std::set<int32_t> visited;

static void send_visited(bool is_step) {
  const auto part = visited.size();

  ::std::lock_guard<::std::mutex> lock(list_mutex);

  try {
    if (client) {
      client->add_visited(visited, is_step);
    }
  }
  catch (...) {

  }

  visited.clear();
}

static void stop_client() {
  try {
    if (client) {
      send_visited(false);
      client->stop_event();
    }
    cli_transport->close();
  }
  catch (...) {

  }
}

static void init_ida_client() {
  ::std::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 9091));
  cli_transport = ::std::shared_ptr<TTransport>(new TFramedTransport(socket));
  ::std::shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(cli_transport));
  client = ::std::shared_ptr<IdaClientClient>(new IdaClientClient(protocol));

  while (true) {
    try {
      cli_transport->open();
      break;
    }
    catch (...) {
      Sleep(10);
    }
  }

  atexit(stop_client);
}

static void toggle_pause(bool enable) {
  application.debug = enable;
  application.debugrun = enable;

  if (enable) {
    audio.clear();
  }
}

class BsnesDebuggerHandler : virtual public BsnesDebuggerIf {

public:
  int32_t get_cpu_reg(const BsnesRegister::type reg) override {
    switch (reg) {
    case BsnesRegister::pc:
    case BsnesRegister::a:
    case BsnesRegister::x:
    case BsnesRegister::y:
    case BsnesRegister::s:
    case BsnesRegister::d:
    case BsnesRegister::db:
    case BsnesRegister::p:
      return SNES::cpu.getRegister((SNES::CPUDebugger::Register)reg);
    case BsnesRegister::mflag:
      return (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagM) ? 1 : 0;
    case BsnesRegister::xflag:
      return (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagX) ? 1 : 0;
    case BsnesRegister::eflag:
      return (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagE) ? 1 : 0;
    }
  }


  void get_cpu_regs(BsnesRegisters& _return) override {
    _return.pc = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterPC);
    _return.a = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterA);
    _return.x = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterX);
    _return.y = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterY);
    _return.s = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterS);
    _return.d = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterD);
    _return.db = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterDB);
    _return.p = SNES::cpu.getRegister(SNES::CPUDebugger::Register::RegisterP);

    _return.mflag = (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagM) ? 1 : 0;
    _return.xflag = (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagX) ? 1 : 0;
    _return.eflag = (SNES::cpu.usage[SNES::cpu.regs.pc] & SNES::CPUDebugger::UsageFlagE) ? 1 : 0;
  }


  void set_cpu_reg(const BsnesRegister::type reg, const int32_t value) override {
    switch (reg) {
    case BsnesRegister::pc:
    case BsnesRegister::a:
    case BsnesRegister::x:
    case BsnesRegister::y:
    case BsnesRegister::s:
    case BsnesRegister::d:
    case BsnesRegister::db:
    case BsnesRegister::p:
      SNES::cpu.setRegister((SNES::CPUDebugger::Register)reg, value);
    }
  }

  void add_breakpoint(const DbgBreakpoint& bpt) override {
    SNES::Debugger::Breakpoint add;
    add.addr = bpt.bstart;
    add.addr_end = bpt.bend;
    add.mode = bpt.type;
    add.source = (SNES::Debugger::Breakpoint::Source)bpt.src;
    SNES::debugger.breakpoint.append(add);
  }


  void del_breakpoint(const DbgBreakpoint& bpt) override {
    for (auto i = 0; i < SNES::debugger.breakpoint.size(); ++i) {
      auto b = SNES::debugger.breakpoint[i];

      if (b.source == (SNES::Debugger::Breakpoint::Source)bpt.src && b.addr == bpt.bstart && b.addr_end == bpt.bend && b.mode == bpt.type) {
        SNES::debugger.breakpoint.remove(i);
        break;
      }
    }
  }


  void read_memory(std::string& _return, const DbgMemorySource::type src, const int32_t address, const int32_t size) override {
    _return.clear();

    SNES::debugger.bus_access = true;
    for (auto i = 0; i < size; ++i) {
      _return += SNES::debugger.read((SNES::Debugger::MemorySource)src, address + i);
    }
    SNES::debugger.bus_access = false;
  }


  void write_memory(const DbgMemorySource::type src, const int32_t address, const std::string& data) override {
    SNES::debugger.bus_access = true;
    for (auto i = 0; i < data.size(); ++i) {
      SNES::debugger.write((SNES::Debugger::MemorySource)src, address, data[i]);
    }
    SNES::debugger.bus_access = false;
  }


  void exit_emulation() override {
    try {
      if (client) {
        send_visited(false);
        client->stop_event();
      }
    }
    catch (...) {

    }

    application.app->exit();
  }


  void pause() override {
    step_into();
  }


  void resume() override {
    toggle_pause(false);
  }


  void start_emulation() override {
    init_ida_client();

    try {
      if (client) {
        client->start_event();
        visited.clear();
        client->pause_event(SNES::cpu.getRegister(SNES::CPUDebugger::RegisterPC));
      }
    }
    catch (...) {

    }
  }


  void step_into() override {
    SNES::debugger.step_type = SNES::Debugger::StepType::StepInto;
    application.debugrun = true;

    SNES::debugger.step_cpu = true;
  }


  void step_over() override {
    SNES::debugger.step_type = SNES::Debugger::StepType::StepOver;
    SNES::debugger.step_over_new = true;
    SNES::debugger.call_count = 0;
    application.debugrun = true;

    SNES::debugger.step_cpu = true;
  }

};

static void stop_server() {
  srv->stop();
}

void init_dbg_server() {
  ::std::shared_ptr<BsnesDebuggerHandler> handler(new BsnesDebuggerHandler());
  ::std::shared_ptr<TProcessor> processor(new BsnesDebuggerProcessor(handler));
  ::std::shared_ptr<TNonblockingServerTransport> serverTransport(new TNonblockingServerSocket(9090));
  ::std::shared_ptr<TFramedTransportFactory> transportFactory(new TFramedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  srv = ::std::shared_ptr<TNonblockingServer>(new TNonblockingServer(processor, protocolFactory, serverTransport));
  ::std::shared_ptr<ThreadFactory> tf(new ThreadFactory());
  ::std::shared_ptr<Thread> thread = tf->newThread(srv);
  thread->start();

  atexit(stop_server);

  SNES::debugger.breakpoint.reset();

  SNES::debugger.step_type = SNES::Debugger::StepType::StepInto;
  application.debugrun = true;
  SNES::debugger.step_cpu = true;
}

void send_pause_event(bool is_step) {
  try {
    if (client) {
      client->pause_event(SNES::cpu.getRegister(SNES::CPUDebugger::RegisterPC));
      send_visited(is_step);
    }
  }
  catch (...) {

  }
}