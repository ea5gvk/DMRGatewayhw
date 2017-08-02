/*
*   Copyright (C) 2017 by Jonathan Naylor G4KLX
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "DMRDataHeader.h"
#include "DMRFullLC.h"
#include "DMRCSBK.h"
#include "Rewrite.h"

#include <cstdio>

CRewrite::CRewrite() :
m_lc(),
m_embeddedLC(),
m_data(NULL),
m_writeNum(0U),
m_readNum(0U)
{
	m_data = new CDMREmbeddedData[2U];
}

CRewrite::~CRewrite()
{
	delete[] m_data;
}

void CRewrite::processMessage(CDMRData& data)
{
	unsigned char dataType = data.getDataType();

	switch (dataType) {
	case DT_VOICE_LC_HEADER:
	case DT_TERMINATOR_WITH_LC:
		processHeader(data, dataType);
		break;

	case DT_VOICE:
		processVoice(data);
		break;

	case DT_CSBK:
		processCSBK(data);
		break;

	case DT_DATA_HEADER:
		processDataHeader(data);
		break;

	case DT_RATE_12_DATA:
	case DT_RATE_34_DATA:
	case DT_RATE_1_DATA:
		// Nothing to do
		break;

	case DT_VOICE_SYNC:
		swap();
		break;

	default:
		// Not sure what to do
		break;
	}
}

void CRewrite::setLC(FLCO flco, unsigned int srcId, unsigned int dstId)
{
	if (flco == m_lc.getFLCO() && srcId == m_lc.getSrcId() && dstId == m_lc.getDstId())
		return;

	m_lc.setFLCO(flco);
	m_lc.setSrcId(srcId);
	m_lc.setDstId(dstId);

	m_embeddedLC.setLC(m_lc);

	m_readNum  = 0U;
	m_writeNum = 0U;
}

unsigned char CRewrite::processEmbeddedData(unsigned char* data, unsigned char n)
{
	unsigned char lcss = 0U;

	switch (n) {
	case 1U:
		lcss = 1U;
		break;
	case 4U:
		lcss = 2U;
		break;
	case 2U:
	case 3U:
		lcss = 3U;
		break;
	default:
		break;
	}

	m_data[m_writeNum].addData(data, lcss);

	if (m_readNum == 0U && m_writeNum == 0U)
		return m_embeddedLC.getData(data, n);

	CDMRLC* lc = m_data[m_readNum].getLC();
	if (lc == NULL)
		return m_embeddedLC.getData(data, n);

	FLCO flco = lc->getFLCO();

	delete lc;

	// Replace any identity embedded data with the new one
	if (flco == FLCO_GROUP || flco == FLCO_USER_USER)
		return m_embeddedLC.getData(data, n);
	else
		return m_data[m_readNum].getData(data, n);
}

void CRewrite::swap()
{
	if (m_readNum == 0U && m_writeNum == 0U) {
		m_writeNum = 1U;
		return;
	}

	if (m_readNum == 0U)
		m_readNum = 1U;
	else
		m_readNum = 0U;

	if (m_writeNum == 0U)
		m_writeNum = 1U;
	else
		m_writeNum = 0U;
}

void CRewrite::processHeader(CDMRData& data, unsigned char dataType)
{
	setLC(data.getFLCO(), data.getSrcId(), data.getDstId());

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewrite::processVoice(CDMRData& data)
{
	setLC(data.getFLCO(), data.getSrcId(), data.getDstId());

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	processEmbeddedData(buffer, data.getN());

	data.setData(buffer);
}

void CRewrite::processDataHeader(CDMRData& data)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRDataHeader dataHeader;
	bool ret = dataHeader.put(buffer);
	if (!ret)
		return;

	dataHeader.setGI(data.getFLCO() == FLCO_GROUP);
	dataHeader.setSrcId(data.getSrcId());
	dataHeader.setDstId(data.getDstId());

	dataHeader.get(buffer);

	data.setData(buffer);
}

void CRewrite::processCSBK(CDMRData& data)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRCSBK csbk;
	bool ret = csbk.put(buffer);
	if (!ret)
		return;

	csbk.setGI(data.getFLCO() == FLCO_GROUP);
	csbk.setSrcId(data.getSrcId());
	csbk.setDstId(data.getDstId());

	csbk.get(buffer);

	data.setData(buffer);
}
