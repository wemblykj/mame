// license:BSD-3-Clause
// copyright-holders:P.Wightmore (PSS-790), AJR (PSR-400)
/*******************************************************************************

    Skeleton driver for Yamaha PSS-790 PortaSound keyboard.
	
	Using PSR-400 module as a template as the CPU is the same and 
	the GEW8 would appear ancestrally related to the GEW6
	
	CPU: Matsu MN18801A
	ROM: 5 'Program' 1Mbit - Main CPU program
		IC15 -  - 15-bit address bus - enabled when A19 is low (ref 1)
		
	ROM: 'ABC' 4Mbit - Function as yet unknown		 
		IC5 - 17-bit address bus - enabled when A19 is high (ref 1)
	
	RAM: 256Kbit - Work ram
		IC6 - 15-bit address bus - enabled when A15 (/AB15) is high?
		
	Sound: YM7138 (GEW6) - AWM Tone Generator & D/A converter
		IC2 - 4 channel output panned across the stereo field using a suitable resister ladder (ref 1)
		address bus enabled when /AB15 is low?
		
	ROM: 'VOICE' 8M - 
		IC3 - 20-bit address bus (ref 1)
	
	References:
		1. PSS-790 Service Manual
		2. PSS-51 Service Manual
		3. https://dtech.lv/techarticles_yamaha_chips.html

*******************************************************************************/

#include "emu.h"
#include "cpu/mn1880/mn1880.h"
#include "sound/multipcm.h"			// TODO: Create new GEW6 module based on GEW8/multipcm functionality
#include "speaker.h"

namespace {

class pss790_state : public driver_device
{
public:
	pss790_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void pss790(machine_config &config);

protected:
	virtual void driver_start() override;
	virtual void machine_start() override;

private:
	TIMER_CALLBACK_MEMBER(interrupt_hack);

	void program_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mpscpu;

	emu_timer *m_hack_timer;
};

void pss790_state::machine_start()
{
	m_hack_timer = timer_alloc(FUNC(pss790_state::interrupt_hack), this);
	m_hack_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

TIMER_CALLBACK_MEMBER(pss790_state::interrupt_hack)
{
	m_maincpu->set_state_int(mn1880_device::MN1880_IF, m_maincpu->state_int(mn1880_device::MN1880_IF) | (1 << 3));
}

void pss790_state::program_map(address_map &map)
{
	// 1 Mbit (128K) ( external program memory space using MMU
	map(0x000000, 0x01ffff).rom().region("program", 0);
	// 4 Mbit (512KB) external 'ABC' memory space using MMU
	map(0x080000, 0x07ffff).rom().region("abc", 0);
}

void pss790_state::data_map(address_map &map)
{
	/* PSS-400...
	// 2 MB external data memory space using MMU
	map(0x000000, 0x000000).nopw(); // ?
	map(0x000001, 0x000001).nopr(); // ?
	map(0x000011, 0x000011).noprw(); // ?
	map(0x000014, 0x000014).noprw(); // ?
	map(0x000018, 0x000018).lr8(NAME([]() { return 0; })); // serial status?
	map(0x00001a, 0x00001a).nopw(); // serial transmit buffer?
	map(0x000030, 0x000031).ram(); // ?
	map(0x000034, 0x000036).noprw(); // ?
	map(0x00003a, 0x00003b).noprw(); // ?
	map(0x00003e, 0x00003f).noprw(); // ?
	map(0x000050, 0x000053).ram(); // ?
	map(0x000055, 0x000055).noprw(); // ?
	map(0x00005d, 0x00005e).noprw(); // ?
	map(0x000080, 0x03ffff).mirror(0xc0000).ram(); // 2x 1M-bit PSRAM (only one on PSR-400)
	map(0x003fe0, 0x003fff).unmaprw(); // window for more internal SFRs?
	map(0x003fe3, 0x003fe3).noprw(); // ?
	map(0x003fe6, 0x003fe6).nopw(); // ?
	map(0x003fe7, 0x003fe7).noprw(); // ?
	map(0x003fe9, 0x003fe9).nopr(); // ?
	map(0x003fee, 0x003fee).lr8(NAME([]() { return 0x05; })).nopw(); // ?
	map(0x003ff3, 0x003ff3).noprw(); // 
	map(0x100000, 0x10000f).mirror(0xffff0).rw("gew8", FUNC(multipcm_device::read), FUNC(multipcm_device::write));
	*/

	// 256 Kbit (32KB) work space using MMU
	map(0x000000, 0x007fff).ram();

	map(0x080000, 0x08000f).mirror(0x0fff0).rw("gew6", FUNC(multipcm_device::read), FUNC(multipcm_device::write));
}

static INPUT_PORTS_START(pss790)
INPUT_PORTS_END

void pss790_state::pss790(machine_config &config)
{
	MN18801A(config, m_maincpu, 10_MHz_XTAL); // MN18801A (also has 500 kHz secondary resonator connected to XI)
	m_maincpu->set_addrmap(AS_PROGRAM, &pss790_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &pss790_state::data_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gew6_device &gew6(MULTIPCM(config, "gew6", 9.4_MHz_XTAL)); // YM7138
	//multipcm_device &gew8(MULTIPCM(config, "gew8", 9.4_MHz_XTAL)); // YMW-258-F
	gew6.add_route(1, "lspeaker", 1.0);
	gew6.add_route(0, "rspeaker", 1.0);
}

ROM_START(pss790)
	ROM_REGION(0x000000, "program", 0)
	ROM_LOAD("xi105a00.ic15", 0x000000, 0x020000, NO_DUMP)
	
	ROM_REGION(0x080000, "abc", 0)
	ROM_LOAD("xh725a00.ic5", 0x000000, 0x080000, NO_DUMP)

	ROM_REGION(0x100000, "gew6", 0)
	ROM_LOAD("xi104a00.ic3", 0x000000, 0x100000, NO_DUMP)
ROM_END

void pss790_state::driver_start()
{
	memory_region *region = memregion("program");
	u8 *program = static_cast<u8 *>(region->base());
	std::vector<u8> buf(0x20000);

	/* PSR-400...
	// A8-A14 & A16 are scrambled
	for (u32 offset = 0; offset < region->bytes(); )
	{
		std::copy(&program[offset], &program[offset + 0x20000], buf.begin());
		for (int blocks = 0x200; blocks-- > 0; offset += 0x100)
			std::copy_n(&buf[bitswap<9>(offset, 9, 15, 10, 11, 8, 14, 16, 13, 12) << 8], 0x100, &program[offset]);
	}
	*/
}

} // anonymous namespace

SYST(1990, pss790, 0, 0, pss790, pss790, pss790_state, empty_init, "Yamaha", "PSS-790", MACHINE_IS_SKELETON | MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
