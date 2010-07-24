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

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>
#include <climits>
#include <vector>
#include <zlib.h>

namespace AMeteor
{
	class Clock
	{
		public :
			Clock () :
				m_first(0)
			{
				Reset();
			}

			void Reset ();

			void TimePass (unsigned short cycles)
			{
				m_cycles += cycles;
			}
			void Commit ();
			void WaitForNext ();

			void AddLcd (uint32_t cycles)
			{
				// The lcd clock is always enabled
				m_lcd += cycles;
				SetFirst();
			}

			void AddTimer (uint8_t num, uint32_t cycles)
			{
				if (m_timer[num] == INT_MAX)
					m_timer[num] = cycles;
				else
					m_timer[num] += cycles;
				SetFirst();
			}
			void SetTimer (uint8_t num, uint32_t cycles)
			{
				m_timer[num] = cycles;
				SetFirst();
			}
			void DisableTimer (uint8_t num)
			{
				m_timer[num] = INT_MAX;
				SetFirst();
			}
			int GetTimer (uint8_t num)
			{
				return m_timer[num];
			}

			void SetBattery (uint32_t cycles)
			{
				m_battery = cycles;
			}
			void DisableBattery ()
			{
				m_battery = INT_MAX;
				// no need to SetFirst since battery will be disabled only in TimeEvent
			}

			bool SaveState (gzFile file);
			bool LoadState (gzFile file);

		private :
			// XXX freq
			static const int SOUND_PERIOD = 380;

			unsigned short m_cycles;
			unsigned short m_first;
			int m_lcd, m_timer[4], m_sound, m_battery;

			void SetFirst ();
	};
}

#endif
