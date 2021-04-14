#include <ida.hpp>
#include <idp.hpp>
#include <dbg.hpp>
#include <loader.hpp>

#include "ida_plugin.h"

extern debugger_t debugger;

static bool plugin_inited;

struct m68k_events_visitor_t : public post_event_visitor_t {
  ssize_t idaapi handle_post_event(ssize_t code, int notification_code, va_list va) override {
    switch (notification_code) {
		case processor_t::ev_get_idd_opinfo: {
      idd_opinfo_t* opinf = va_arg(va, idd_opinfo_t*);
      ea_t ea = va_arg(va, ea_t);
      int n = va_arg(va, int);
      int thread_id = va_arg(va, int);
      processor_t::regval_getter_t *getreg = va_arg(va, processor_t::regval_getter_t *);
      const regval_t* regvalues = va_arg(va, const regval_t*);

      //opinf->ea = BADADDR;
      //opinf->debregidx = 0;
      //opinf->modified = false;
      //opinf->value.ival = 0;
      //opinf->value_size = 4;

      insn_t out;
      if (!decode_insn(&out, ea)) {
				return code;
			}

			op_t op = out.ops[n];

      switch (op.dtype) {
      case dt_byte:
        opinf->value_size = 1;
        break;
      case dt_dword:
        opinf->value_size = 4;
        break;
      default:
        opinf->value_size = 2;
        break;
      }

      switch (op.type) {
      case o_mem:
      case o_near:
				opinf->ea = op.addr;
				break;
      case o_imm:
				opinf->ea = op.value;
				break;
			}
			return 1;
		} break;
		}

		return code;
	}
} ctx;

static bool init_plugin(void) {
	return (ph.id == PLFM_65C816);
}

static void print_version()
{
	static const char format[] = NAME " debugger plugin v%s;\nAuthor: DrMefistO [Lab 313] <newinferno@gmail.com>.";
	info(format, VERSION);
	msg(format, VERSION);
}

static plugmod_t* idaapi init(void) {
	if (init_plugin()) {
		dbg = &debugger;
		plugin_inited = true;

		//register_post_event_visitor(HT_IDP, &ctx, nullptr);

		print_version();
		return PLUGIN_KEEP;
	}

	return PLUGIN_SKIP;
}

static void idaapi term(void) {
	if (plugin_inited) {
		//unregister_post_event_visitor(HT_IDP, &ctx);

		plugin_inited = false;
	}
}

static bool idaapi run(size_t arg) {
	return false;
}

char comment[] = NAME " debugger plugin by DrMefistO.";

char help[] =
	NAME " debugger plugin by DrMefistO.\n"
	"\n"
	"This module lets you debug SNES roms in IDA.\n";

plugin_t PLUGIN = {
	IDP_INTERFACE_VERSION,
	PLUGIN_PROC | PLUGIN_DBG,
	init,
	term,
	run,
	comment,
	help,
	NAME " debugger plugin",
	""
};
