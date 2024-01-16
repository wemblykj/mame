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
		4. PSR-400/PSR-500 Service Manual

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

	void transmute_program_rom(memory_region* region);

	required_device<cpu_device> m_maincpu;

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
	/* PSS-400... additional documentation with reference to 4.
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
	// CE1/ = ~( A20/ & A17/ )
	// CE2/ = ~( A20/ & A17 )
	map(0x000080, 0x03ffff).mirror(0xc0000).ram(); // 2x 1M-bit PSRAM (only one on PSR-400)
	map(0x003fe0, 0x003fff).unmaprw(); // window for more internal SFRs?
	map(0x003fe3, 0x003fe3).noprw(); // ?
	map(0x003fe6, 0x003fe6).nopw(); // ?
	map(0x003fe7, 0x003fe7).noprw(); // ?
	map(0x003fe9, 0x003fe9).nopr(); // ?
	map(0x003fee, 0x003fee).lr8(NAME([]() { return 0x05; })).nopw(); // ?
	map(0x003ff3, 0x003ff3).noprw(); //
	// CS =< AB20 (high)
	map(0x100000, 0x10000f).mirror(0xffff0).rw("gew8", FUNC(multipcm_device::read), FUNC(multipcm_device::write));
	*/

	// 2 MB external data memory space using MMU
	// Not sure how the above PSR-400 maps were derived but I can only assume that all of the
	// MN18801A CPUs additional inputs/outputs are [statically] memory mapped into these [predefined] areas
	// TCI0-7 => Keyboard Matrix (address)
	//   TCI0 => N5
	//   TCI1 => N4
	//   TCI2 => N3
	//   TCI3 => N2
	//   TCI4 => N1
	//   TCI5 => N0
	// S0-7 =>  Keyboard/Button matrix (data)
	//   S0 => BO0 => B0
	//   S1 => BO1 => B1
	//   S2 => BO2 => B2
	//   S3 => BO3 => B3
	//   S4 => BO4 => B4
	//   S5 => BO5 => B5
	//   S6 => BO6 => B6
	//   S7 => BO7 => B7
	// HS0-7 => Keyboard/Button matrix (data)
	//   HS0 => BO8
	//   HS1 => BO9
	//   HS2 => BO10
	//   HS3 => BO11
	//   HS4 => BO12
	//   HS5 => BO13
	//   HS6 => BO14
	//   HS7 => BO15
	// SW0-7 =>	(8 ADC channels)
	//   SW4 => Pitch Bend
	//   SW5 => Vector Synth (axis 1)
	//   SW6 => Vector Synth (axis 2)
	// PI0-7 => Button matrix (address)
	//	 common (data) | B     | B2            | B3               | B4                   | BO8     | BO9
	//   ---------------  +-------+---------------|------------------+----------------------+---------+---------
	//   PI0 => N6 =>	  |       | Single Finger |                  |                      | Pad 1   | Pad 5
	//   PI1 => N7 => 	  |       |               |                  |                      | Pad 2   | Pad 6
	//   PI2 => N8 =>	  |       | Intro         |                  |                      | Pad 3   | Pad 9
	//   PI3 => N9 =>	  |       | Large         |                  |                      | Pad 4   | Pad 10
	//   PI4 => N10 =>	  |       | Bridge        | On/Off           | Start/Stop           |         |
	//   PI5 => N11 =>	  |       |               |                  | Fill To Normal       |         |
	//   PI6 => N12 =>	  |       |               | Mem Bulk Dump    | Fill To Bridge       |         |
	//   PI7 => N13 =>	  |       |               |                  | Synchro Start/Ending |         |
	// TO0-7 7 Segment Display matrix (address)
	//   TO0 => LB0 => C0	 =>	Multi Display (units) |  Style/Voice (units)
	//   TO1 => LB1 => C1	 =>	Multi Display (tens) |  Style/Voice (tens)
	//   TO2 => LB2 => C2    => Multi Display (hundreds) |  
	//   TO3 => LB3 => C3
	//   TO4 => LB4 => C4
	//   TO5 => LB5 => C5

	// 256 Kbit (32KB) DRAM
	map(0x000000, 0x007fff).ram();

	// CS <= /AB15 = AB15 (high)
	map(0x080000, 0x08000f).mirror(0x07ff0).rw("gew6", FUNC(multipcm_device::read), FUNC(multipcm_device::write));
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

/* Documenting PSR-400 equivalent from ref 4.
ROM_START(psr500)
	// 2Mbit ROM
	// CS =< AB20 (low)
	ROM_REGION(0x200000, "program", 0)
	ROM_LOAD("xj920c0.ic4", 0x000000, 0x040000, CRC(bd45d962) SHA1(fe46ceae5584b56e36f31f27bedd9e7d578eb35b))
	// mirroring due to unreferenced address lines AB19->AB17
	ROM_RELOAD(0x040000, 0x040000)
	ROM_RELOAD(0x080000, 0x040000)
	ROM_RELOAD(0x0c0000, 0x040000)

	// 8Mbit ROM
	// CS =< AB20 (high)
	ROM_LOAD("xj921b0.ic5", 0x100000, 0x100000, CRC(dd1a8afc) SHA1(5d5b47577faeed165f0bd73283f148d112e4d1e9))
	
	ROM_REGION(0x100000, "gew8", 0)
	ROM_LOAD("xj426b0.ic3", 0x000000, 0x100000, CRC(ef566734) SHA1(864f5689dbaa82bd8a1be4e53bdb21ec71be03cc))

	ROM_REGION(0x1000, "mpscpu", 0)
	ROM_LOAD("xj450a00.ic1", 0x0000, 0x1000, NO_DUMP)
ROM_END
*/
ROM_START(pss790)
	// CS <= AB19 (low)
	ROM_REGION(0x100000, "program", 0)
	ROM_LOAD("xi105a00.ic15", 0x000000, 0x020000, NO_DUMP)

	// mirroring due to unreferenced address lines AB18->AB17
	ROM_RELOAD(0x020000, 0x020000)
	ROM_RELOAD(0x040000, 0x020000)
	ROM_RELOAD(0x060000, 0x020000)

	// CS <= AB19 (high)
	ROM_LOAD("xh725a00.ic5", 0x080000, 0x080000, NO_DUMP)

	ROM_REGION(0x100000, "gew6", 0)
	ROM_LOAD("xi104a00.ic3", 0x000000, 0x100000, NO_DUMP)
ROM_END

void transmute_program_rom(memory_region* region)
{
	// get pointer to start of rom
	u8* base = static_cast<u8*>(region->base());
	const u32 rom_size = region->bytes();
	const int chunk_size = 0x20000;
	const int block_size = 0x100;
	const int block_count = chunk_size / block_size;

	// 32KB buffer 
	std::vector<u8> chunk(chunk_size);

	// iterate through entire rom (in 32 KB blocks)
	for (u32 block_offset = 0; block_offset < rom_size; )
	{
		// copy 32KB from offset into temporary buffer
		std::copy(&rom[block_offset], &base[offset + chunk_size], chunk.begin());

		// iterate over all blocks in the 32KB chunk
		for (int blocks = block_count; blocks-- > 0; block_offset += block_size)
		{
			// auto transmuted_offset = bitswap<9>(offset, 9, 15, 10, 11, 8, 14, 16, 13, 12) << 8	// PSR-400 scrambling
			// Address Lines | A16 | A15 | A14 | A13 | A12 | A11|  A10 |  A9 |  A8
			// Chip Inputs   | A10 | A15 | A11 |  A9 |  A8 | A13 | A14 | A16 | A12
			u32 transmuted_offset = bitswap<9>(block_offset, 10, 15, 11, 9, 8, 13, 14, 16, 12) << 8;	// PSS-790 scrambling

			// copy the block into its transmuted location in the rom image
			std::copy_n(&chunk[transmuted_offset], block_size, &base[block_offset]);
		}
	}
}

void pss790_state::driver_start()
{
	// Akin to the PSR-400...
	//   'A8-A14 & A16 are scrambled'
	// This would appear to affect both PROGRAM and ABC roms [ref 1.]
	// The PSS-790 schematic would infer that A15 is not scrambled
	// +---------+---------+
	// | Address |  Chip   |
	// |   Line  |  Input  |
	// +-------------------|
	// |      A8 | A12	   |
	// |      A9 | A16	   |
	// |     A10 | A14	   |
	// |     A11 | A13	   |
	// |     A12 | A8	   |
	// |     A13 | A9	   |
	// |     A14 | A11	   |
	// |     A16 | A10	   |
	// +---------+---------+

	transmute_program_rom(memregion("program"));
	transmute_program_rom(memregion("abc"));
}

} // anonymous namespace

SYST(1990, pss790, 0, 0, pss790, pss790, pss790_state, empty_init, "Yamaha", "PSS-790", MACHINE_IS_SKELETON | MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
