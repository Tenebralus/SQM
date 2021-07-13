#pragma once
#include "stdafx.h"
#include "clsEdge.h"
#include "clsTriangle.h"
#include "clsBNPObject.h"
#include "clsLIE.h"

#include "../OpenGLLib/clsVertex.h"

namespace GenerationLib
{
	clsEdge::clsEdge( )
	{
		InitData( );
	}

	clsEdge::clsEdge( OpenGLLib::Skeleton::clsVertex* a_v1, OpenGLLib::Skeleton::clsVertex* a_v2 )
	{
		InitData( );
		Init( a_v1, a_v2 );
	}

	clsEdge::~clsEdge( )
	{

	}

	void clsEdge::Init( OpenGLLib::Skeleton::clsVertex* a_v1, OpenGLLib::Skeleton::clsVertex* a_v2 )
	{
		m_v1 = a_v1;
		m_v2 = a_v2;
	}

	void clsEdge::InitData( )
	{
		m_v1 = nullptr;
		m_v2 = nullptr;
		m_t1 = nullptr;
		m_t2 = nullptr;
		m_LIE1 = nullptr;
		m_LIE2 = nullptr;
		m_IsReverseInLIE = false;
	}

	void clsEdge::PointToTriangle( clsTriangle* a_Triangle )
	{
		if (m_t1 == nullptr)
		{
			m_t1 = a_Triangle;
		}
		else if (m_t2 == nullptr)
		{
			m_t2 = a_Triangle;
		}
		else
		{
			assert( "Both pointers already set." == 0 );
		}
	}

	void clsEdge::SetLIE( clsLIE* a_LIE )
	{
		if (m_LIE1 == nullptr)
		{
			m_LIE1 = a_LIE;
		}
		else if (m_LIE2 == nullptr)
		{
			m_LIE2 = a_LIE;
		}
		else
		{
			assert( "Both LIEs set already." == 0 );
		}
	}

	void clsEdge::SetVertex1( OpenGLLib::Skeleton::clsVertex* a_Vertex )
	{
		m_v1 = a_Vertex;
	}

	void clsEdge::SetVertex2( OpenGLLib::Skeleton::clsVertex* a_Vertex )
	{
		m_v2 = a_Vertex;
	}

	void clsEdge::SetTriangle1( clsTriangle* a_Triangle )
	{
		m_t1 = a_Triangle;
	}

	void clsEdge::SetTriangle2( clsTriangle* a_Triangle )
	{
		m_t2 = a_Triangle;
	}

	OpenGLLib::Skeleton::clsVertex*	clsEdge::GetVertex1( )
	{
		return m_v1;
	}

	OpenGLLib::Skeleton::clsVertex*	clsEdge::GetVertex2( )
	{
		return m_v2;
	}

	OpenGLLib::Skeleton::clsVertex*	clsEdge::GetVertexOppositeOf( OpenGLLib::Skeleton::clsVertex* a_Vertex )
	{
		if (m_v1 == a_Vertex)
		{
			return m_v2;
		}
		else if (m_v2 == a_Vertex)
		{
			return m_v1;
		}
		else
		{
			assert( "Edge does not contain input vertex." == 0 );
			return nullptr;
		}
	}

	clsTriangle* clsEdge::GetTriangle1( )
	{
		return m_t1;
	}

	clsTriangle* clsEdge::GetTriangle2( )
	{
		return m_t2;
	}

	clsLIE* clsEdge::GetLIE1( )
	{
		return m_LIE1;
	}

	clsLIE* clsEdge::GetLIE2( )
	{
		return m_LIE2;
	}

	clsLIE* clsEdge::GetLIEOppositeOf( clsLIE* a_LIE )
	{
		if (m_LIE1 == a_LIE)
		{
			return m_LIE2;
		}
		else if (m_LIE2 == a_LIE)
		{
			return m_LIE1;
		}
		else
		{
			assert( "Edge does not contain input LIE." == 0 );
			return nullptr;
		}
	}
}
