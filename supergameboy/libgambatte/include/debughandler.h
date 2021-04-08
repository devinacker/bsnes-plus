#ifndef GAMBATTE_DEBUGHANDLER_H
#define GAMBATTE_DEBUGHANDLER_H

#include <nall/function.hpp>

namespace gambatte {

class DebugHandler {
public:
	virtual ~DebugHandler() {}

	nall::function<void (uint32_t)> op_step;
	nall::function<void (uint32_t)> op_call;
	nall::function<void (uint32_t)> op_ret;
	nall::function<void (uint32_t)> op_irq;
	nall::function<void (uint32_t, uint8_t)> op_read;
	nall::function<void (uint32_t, uint8_t)> op_readpc;
	nall::function<void (uint32_t, uint8_t)> op_write;
};

}

#endif
