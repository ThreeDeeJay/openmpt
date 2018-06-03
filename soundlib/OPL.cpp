/*
 * OPL.cpp
 * -------
 * Purpose: Translate data coming from OpenMPT's mixer into OPL commands to be sent to the Opal emulator.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 *          Schism Tracker contributors (bisqwit, JosepMa, Malvineous, code relicensed from GPL to BSD with permission)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "../common/misc_util.h"
#include "OPL.h"
#include "opal.h"

OPENMPT_NAMESPACE_BEGIN

OPL::OPL()
{
	m_KeyOnBlock.fill(0);
	m_OPLtoChan.fill(CHANNELINDEX_INVALID);
	m_ChanToOPL.fill(OPL_CHANNEL_INVALID);
}


OPL::~OPL()
{
	// This destructor is put here so that we can forward-declare the Opal emulator class.
}


void OPL::Initialize(uint32 samplerate)
{
	if(m_opl == nullptr)
		m_opl = mpt::make_unique<Opal>(samplerate);
	else
		m_opl->SetSampleRate(samplerate);
	Reset();
}


void OPL::Mix(int *target, size_t count)
{
	if(!isActive)
		return;

	while(count--)
	{
		int16 l, r;
		m_opl->Sample(&l, &r);
		target[0] += l * (1 << 13);
		target[1] += r * (1 << 13);
		target += 2;
	}
}


uint16 OPL::ChannelToRegister(uint8 oplCh)
{
	if(oplCh < 9)
		return oplCh;
	else
		return (oplCh - 9) | 0x100;
}


// Translate a channel's first operator address into a register
uint16 OPL::OperatorToRegister(uint8 oplCh)
{
	static const uint8 OPLChannelToOperator[] = { 0, 1, 2, 8, 9, 10, 16, 17, 18 };
	if(oplCh < 9)
		return OPLChannelToOperator[oplCh];
	else
		return OPLChannelToOperator[oplCh - 9] | 0x100;
}


uint8 OPL::GetVoice(CHANNELINDEX c) const
{
	return m_ChanToOPL[c];
}


uint8 OPL::AllocateVoice(CHANNELINDEX c)
{
	// Can we re-use a previous channel?
	if(m_ChanToOPL[c] != OPL_CHANNEL_INVALID)
	{
		return GetVoice(c);
	}
	// Search for unused channel or channel with released note
	uint8 releasedChn = OPL_CHANNEL_INVALID;
	for(uint8 oplCh = 0; oplCh < OPL_CHANNELS; oplCh++)
	{
		if(m_OPLtoChan[oplCh] == CHANNELINDEX_INVALID)
		{
			m_OPLtoChan[oplCh] = c;
			m_ChanToOPL[c] = oplCh;
			return GetVoice(c);
		} else if(!(m_KeyOnBlock[oplCh] & KEYON_BIT))
		{
			releasedChn = oplCh;
		}
	}
	if(releasedChn != OPL_CHANNEL_INVALID)
	{
		m_ChanToOPL[m_OPLtoChan[releasedChn]] = OPL_CHANNEL_INVALID;
		m_OPLtoChan[releasedChn] = c;
		m_ChanToOPL[c] = releasedChn;
	}
	return GetVoice(c);
}


void OPL::NoteOff(CHANNELINDEX c)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;
	m_KeyOnBlock[oplCh] &= ~KEYON_BIT;
	m_opl->Port(KEYON_BLOCK | ChannelToRegister(oplCh), m_KeyOnBlock[oplCh]);
}


void OPL::NoteCut(CHANNELINDEX c)
{
	NoteOff(c);
	Volume(c, 0);
}


void OPL::Frequency(CHANNELINDEX c, uint32 milliHertz, bool keyOff)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	uint16 fnum = 0;
	uint8 block = 0;
	if(milliHertz > 6208431)
	{
		// Frequencies too high to produce
		block = 7;
		fnum = 1023;
	} else
	{
		if(milliHertz > 3104215) block = 7;
		else if(milliHertz > 1552107) block = 6;
		else if(milliHertz > 776053) block = 5;
		else if(milliHertz > 388026) block = 4;
		else if(milliHertz > 194013) block = 3;
		else if(milliHertz > 97006) block = 2;
		else if(milliHertz > 48503) block = 1;
		else block = 0;

		fnum = static_cast<uint16>(Util::muldivr_unsigned(milliHertz, 1 << (20 - block), OPL_BASERATE * 1000));
		MPT_ASSERT(fnum < 1024);
	}

	uint16 channel = ChannelToRegister(oplCh);
	m_KeyOnBlock[oplCh] = (keyOff ? 0 : KEYON_BIT)   // Key on
	    | (block << 2)                               // Octave
	    | ((fnum >> 8) & FNUM_HIGH_MASK);            // F-number high 2 bits
	m_opl->Port(FNUM_LOW    | channel, fnum & 0xFF); // F-Number low 8 bits
	m_opl->Port(KEYON_BLOCK | channel, m_KeyOnBlock[oplCh]);

	isActive = true;
}


uint8 OPL::CalcVolume(uint8 trackerVol, uint8 kslVolume)
{
	if(trackerVol >= 63u)
		return kslVolume;
	if(trackerVol > 0)
		trackerVol++;
	return (kslVolume & KSL_MASK) | (63u - ((63u - (kslVolume & TOTAL_LEVEL_MASK)) * trackerVol) / 64u);
}


void OPL::Volume(CHANNELINDEX c, uint8 vol)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	const auto &patch = m_Patches[oplCh];
	const uint16 modulator = OperatorToRegister(oplCh), carrier = modulator + 3;
	if(patch[10] & CONNECTION_BIT)
	{
		// Set volume of both operators in additive mode
		m_opl->Port(KSL_LEVEL + modulator, CalcVolume(vol, patch[2]));
	}
	m_opl->Port(KSL_LEVEL + carrier, CalcVolume(vol, patch[3]));
}


void OPL::Pan(CHANNELINDEX c, int32 pan)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	const auto &patch = m_Patches[oplCh];
	uint8 fbConn = patch[10] & ~STEREO_BITS;
	// OPL3 only knows hard left, center and right, so we need to translate our
	// continuous panning range into one of those three states.
	// 0...84 = left, 85...170 = center, 171...256 = right
	if(pan <= 170)
		fbConn |= VOICE_TO_LEFT;
	if(pan >= 85)
		fbConn |= VOICE_TO_RIGHT;

	m_opl->Port(FEEDBACK_CONNECTION | ChannelToRegister(oplCh), fbConn);
}


void OPL::Patch(CHANNELINDEX c, const OPLPatch &patch)
{
	uint8 oplCh = AllocateVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	m_Patches[oplCh] = patch;

	const uint16 modulator = OperatorToRegister(oplCh), carrier = modulator + 3;
	for(uint8 op = 0; op < 2; op++)
	{
		const auto opReg = op ? carrier : modulator;
		m_opl->Port(AM_VIB          | opReg, patch[0 + op]);
		m_opl->Port(KSL_LEVEL       | opReg, patch[2 + op]);
		m_opl->Port(ATTACK_DECAY    | opReg, patch[4 + op]);
		m_opl->Port(SUSTAIN_RELEASE | opReg, patch[6 + op]);
		m_opl->Port(WAVE_SELECT     | opReg, patch[8 + op]);
	}

	m_opl->Port(FEEDBACK_CONNECTION | ChannelToRegister(oplCh), patch[10]);
}


void OPL::Reset()
{
	if(isActive)
	{
		for(CHANNELINDEX chn = 0; chn < MAX_CHANNELS; chn++)
		{
			NoteCut(chn);
		}
		isActive = false;
	}

	m_KeyOnBlock.fill(0);
	m_OPLtoChan.fill(CHANNELINDEX_INVALID);
	m_ChanToOPL.fill(OPL_CHANNEL_INVALID);
}

OPENMPT_NAMESPACE_END
