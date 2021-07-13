#pragma once
#include "stdafx.h"
#include "clsPathVertex.h"
#include "clsLIE.h"
#include "clsEdge.h"
#include "clsTriangle.h"
#include "clsBNPObject.h"
#include "clsSQMDebugData.h"

#include "../BrawlerEngineLib/clsMath.h"

namespace GenerationLib
{
	clsPathVertex::clsPathVertex( )
	{

	}

	clsPathVertex::~clsPathVertex( )
	{

	}

	void clsPathVertex::SetLIE( clsLIE* a_LIE )
	{
		m_LIE = a_LIE;
	}

	void clsPathVertex::SetBNPPointer( clsBNPObject* a_BNPObject )
	{
		m_BNPObject = a_BNPObject;
	}

	void clsPathVertex::SetConnectedPathVertex( clsPathVertex* a_PathVertex )
	{
		m_ConnectedPathVertex = a_PathVertex;
	}

	clsPathVertex* clsPathVertex::GetConnectedPathVertex( )
	{
		return m_ConnectedPathVertex;
	}

	clsLIE* clsPathVertex::GetLIE( )
	{
		return m_LIE;
	}
}
