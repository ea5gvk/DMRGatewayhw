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

#if !defined(REWRITETYPE_H)
#define	REWRITETYPE_H

#include "Rewrite.h"
#include "DMRData.h"

#include <string>

class CRewriteType : public CRewrite {
public:
	CRewriteType(const std::string& name, unsigned int fromSlot, unsigned int fromTG, unsigned int toSlot, unsigned int toId);
	virtual ~CRewriteType();

	virtual bool process(CDMRData& data, bool trace);

private:
	std::string  m_name;
	unsigned int m_fromSlot;
	unsigned int m_fromTG;
	unsigned int m_toSlot;
	unsigned int m_toId;
};


#endif
