#pragma once

#define RC_CPU (1 << 0)
#define RC_PPU (1 << 1)

enum class SNES_REGS : uint8_t
{
	SR_A,
	SR_X,
	SR_Y,
  SR_D,
  SR_DB,
	SR_PC,
	SR_S,
	SR_P,
	SR_MFLAG,
	SR_XFLAG,
	SR_EFLAG,
};
