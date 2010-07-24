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

#include "ameteor/eeprom.hpp"
#include "globals.hpp"
#include "debug.hpp"

#include <cstring>

namespace AMeteor
{
	Eeprom::Eeprom (bool big) :
		CartMem(),
		m_state(IDLE),
		m_add(0),
		m_pos(0)
	{
		if (big)
			m_size = 0x2000;
		else
			m_size = 0x0200;

		m_data = new uint8_t[m_size];
		std::memset(m_data, 0, m_size);
	}

	Eeprom::~Eeprom ()
	{
		delete [] m_data;
	}

	void Eeprom::Reset ()
	{
		std::memset(m_data, 0, m_size);
	}

	bool Eeprom::Load (std::istream& f)
	{
		f.read((char*)m_data, m_size);
		return f.good();
	}

	bool Eeprom::Save (std::ostream& f)
	{
		f.write((char*)m_data, m_size);
		return f.good();
	}

	uint8_t Eeprom::Read (uint16_t add)
	{
		_assert("8 bits write to EEPROM");
	}

	uint16_t Eeprom::Read ()
	{
		switch (m_state)
		{
			case READ_GARBAGE:
				++m_pos;
				if (m_pos == 4)
				{
					m_pos = 0;
					m_state = READ_DATA;
				}
				return 0;
			case READ_DATA:
				{
					uint16_t ret =
						(m_data[m_add + m_pos / 8] & (0x1 << (7 - (m_pos % 8)))) ? 1 : 0;
					++m_pos;
					if (m_pos == 64)
						m_state = IDLE;
					return ret;
				}
			default:
				return 1;
		}
	}

	bool Eeprom::Write (uint16_t add, uint8_t val)
	{
		_assert("8 bits write to EEPROM");
	}
	//XXX
#if 0
	bool Eeprom::Write (uint16_t val)
	{
		switch (m_state)
		{
			case IDLE:
				if (val & 0x1)
					m_state = WAITING;
				else
					_assert("First bit is not 1");
				return false;
			case WAITING:
				m_add = 0;
				m_pos = 0;
				if (val & 0x1)
					m_state = READ_ADD;
				else
					m_state = WRITE_ADD;
				return false;

			case READ_ADD:
				m_add <<= 1;
				m_add |= val & 0x1;
				++m_pos;
				if (m_size == 0x0200 && m_pos == 6 ||
						m_size == 0x2000 && m_pos == 14)
				{
					m_state = READ_END;
					if (m_size == 0x2000 && (m_add & 0x3C000))
						_assert("In large EEPROM, 4 upper address bits are not 0");
				}
				return false;
			case READ_END:
				if (val & 0x1)
					_assert("Last bit of EEPROM read request is not 0");
				m_pos = 0;
				m_state = READ_GARBAGE;
				return false;

			case WRITE_ADD:
				m_add <<= 1;
				m_add |= val & 0x1;
				++m_pos;
				if (m_size == 0x0200 && m_pos == 6 ||
						m_size == 0x2000 && m_pos == 14)
				{
					m_state = WRITE_DATA;
					m_pos = 0;
					if (m_size == 0x2000 && (m_add & 0x3C000))
						_assert("In large EEPROM, 4 upper address bits are not 0");
				}
				return false;
			case WRITE_DATA:
				{
					uint8_t& d = m_data[m_add + m_pos / 8];
					d <<= 1;
					d |= val & 0x1;
				}
				++m_pos;
				if (m_pos == 64)
					m_state = WRITE_END;
				return true;
			case WRITE_END:
				if (val & 0x1)
					_assert("Last bit of EEPROM write request is not 0");
				return false;
		}
	}
#endif

	bool Eeprom::Write (uint16_t* pData, uint16_t size)
	{
		if (!(*pData & 0x1))
			_assert("Bit 1 is not 1 in EEPROM DMA");
		++pData;

		uint16_t add = 0;
		if (*pData & 0x1) // read
		{
			if (size != 9 && size != 17)
				_assert("Invalid size for read");
			++pData;

			// read address
			for (uint8_t i = 0, end = (m_size == 0x0200) ? 6 : 14; i < end;
					++i, ++pData)
				add = (add << 1) | (*pData & 0x1);
			if (m_size == 0x2000 && (add & 0x3C00))
				_assert("In large EEPROM, 4 upper address bits are not 0");

			//XXX
			/*if (*pData & 0x1)
				_assert("Last bit of EEPROM read request is not 0");*/

			m_add = add*8;
			m_state = READ_GARBAGE;
			m_pos = 0;

			return false;
		}
		else // write
		{
			if (size != 73 && size != 81)
				_assert("Invalid size for write");
			++pData;

			// read address
			for (uint8_t i = 0, end = (m_size == 0x0200) ? 6 : 14; i < end;
					++i, ++pData)
				add = (add << 1) | (*pData & 0x1);
			if (m_size == 0x2000 && (add & 0x3C00))
				_assert("In large EEPROM, 4 upper address bits are not 0");

			// read data
			uint8_t* pMem = m_data + add*8;
			for (uint8_t i = 0; i < 8; ++i, ++pMem)
			{
				for (uint8_t j = 0; j < 8; ++j, ++pData)
				{
					*pMem <<= 1;
					*pMem |= (*pData & 0x1);
				}
			}

			if (*pData & 0x1)
				_assert("Last bit of EEPROM write request is not 0");

			m_state = IDLE;

			return true;
		}
	}

	//XXX
#if 0
	void Eeprom::Read (uint16_t* pOut)
	{
		if (m_state != READ)
			_assert("Read in invalid EEPROM state");

		pOut += 4; // ignore these
		uint8_t* pData = m_data + m_add;
		uint8_t cur;
		for (uint8_t i = 0; i < 8; ++i, ++pData)
		{
			cur = *pData;
			pOut += 7;
			for (uint8_t j = 0; j < 8; ++j, --pOut, cur >>= 1)
				*pOut = cur & 0x1;
			pOut += 9;
		}

		m_state = NORMAL;
	}
#endif

	bool Eeprom::SaveState (gzFile file)
	{
		//XXX TODO
		GZ_WRITE(m_size);
		GZ_WRITE(m_state);
		GZ_WRITE(m_add);

		if (!gzwrite(file, m_data, m_size))
			return false;

		return true;
	}

	bool Eeprom::LoadState (gzFile file)
	{
		GZ_READ(m_size);
		GZ_READ(m_state);
		GZ_READ(m_add);

		if (gzread(file, m_data, m_size) != (signed)m_size)
			return false;

		return true;
	}
}
