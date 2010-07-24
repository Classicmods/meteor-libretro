// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ameteor/clock.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"
// XXX
extern unsigned long i;
namespace AMeteor
{
	void Clock::Reset ()
	{
		// lcd is enabled by default
		m_cycles = m_lcd = m_sound = m_sound = 0;
		// timers and battery are disabled by default
		m_battery = m_timer[0] = m_timer[1] = m_timer[2] = m_timer[3] =
			INT_MAX;
	}

	void Clock::Commit ()
	{

		unsigned short tocommit;

		// this loop is here because a timer can trigger a dma which will take a
		// long time, during this time the lcd must draw and the timers continue
		// XXX
		// 3 cycles for each instruction (debug) XXX
#if defined METDEBUG && defined METDEBUGLOG
		m_cycles = 3;
#else
		while (m_cycles >= m_first)
#endif
		{
			tocommit = m_cycles;
			m_cycles = 0;

			m_lcd -= tocommit;
			while (m_lcd <= 0)
				LCD.TimeEvent();

			m_sound -= tocommit;
			while (m_sound <= 0)
			{
				SOUND.TimeEvent();
				// XXX freq
				m_sound += SOUND_PERIOD;
			}

#define COMMIT(dev, obj) \
	if (m_##dev != INT_MAX) \
	{ \
		m_##dev -= tocommit; \
		while (m_##dev <= 0) \
			obj.TimeEvent(); \
	}
			COMMIT(timer[0], TIMER0)
			COMMIT(timer[1], TIMER1)
			COMMIT(timer[2], TIMER2)
			COMMIT(timer[3], TIMER3)
			COMMIT(battery, MEM)
#undef COMMIT

			SetFirst();
		}
	}

	void Clock::WaitForNext ()
	{
		m_cycles = m_first;
		Commit();
	}

#define SETFIRST(dev) \
	if (m_##dev < m_first) \
		m_first = m_##dev
	void Clock::SetFirst ()
	{
		m_first = m_lcd;
		SETFIRST(timer[0]);
		SETFIRST(timer[1]);
		SETFIRST(timer[2]);
		SETFIRST(timer[3]);
		SETFIRST(sound);
		SETFIRST(battery);
	}
#undef SETFIRST

	bool Clock::SaveState (gzFile file)
	{
		GZ_WRITE(m_cycles);
		GZ_WRITE(m_first);
		GZ_WRITE(m_lcd);
		GZ_WRITE(m_sound);
		GZ_WRITE(m_battery);

		if (!gzwrite(file, m_timer, sizeof(m_timer)))
			return false;

		return true;
	}

	bool Clock::LoadState (gzFile file)
	{
		GZ_READ(m_cycles);
		GZ_READ(m_first);
		GZ_READ(m_lcd);
		GZ_READ(m_sound);
		GZ_READ(m_battery);

		if (gzread(file, m_timer, sizeof(m_timer)) != sizeof(m_timer))
			return false;

		return true;
	}
}
