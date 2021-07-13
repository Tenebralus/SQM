#pragma once
#include "stdafx.h"

#include "../OpenGLLib/clsVertex.h"

namespace GenerationLib
{
	class clsEdge
	{
	public:
		clsEdge( );
		clsEdge( OpenGLLib::Skeleton::clsVertex* v1, OpenGLLib::Skeleton::clsVertex* v2 );
		~clsEdge( );

		void Init( OpenGLLib::Skeleton::clsVertex* v1, OpenGLLib::Skeleton::clsVertex* v2 );
		void InitData( );
		void PointToTriangle( clsTriangle* Triangle );
		void SetLIE( clsLIE* LIE );
		void SetVertex1( OpenGLLib::Skeleton::clsVertex* Vertex );
		void SetVertex2( OpenGLLib::Skeleton::clsVertex* Vertex );
		void SetTriangle1( clsTriangle* Triangle );
		void SetTriangle2( clsTriangle* Triangle );

		OpenGLLib::Skeleton::clsVertex*	GetVertex1( );
		OpenGLLib::Skeleton::clsVertex*	GetVertex2( );
		OpenGLLib::Skeleton::clsVertex*	GetVertexOppositeOf( OpenGLLib::Skeleton::clsVertex* Vertex );
		clsTriangle*					GetTriangle1( );
		clsTriangle*					GetTriangle2( );
		clsLIE*							GetLIE1( );
		clsLIE*							GetLIE2( );
		clsLIE*							GetLIEOppositeOf( clsLIE* LIE );

		bool operator==( const clsEdge OtherEdge ) const
		{
			return m_v1 == OtherEdge.m_v1 && m_v2 == OtherEdge.m_v2 ||
				m_v2 == OtherEdge.m_v1 && m_v1 == OtherEdge.m_v2;
		}

	private:
		OpenGLLib::Skeleton::clsVertex*	 m_v1;
		OpenGLLib::Skeleton::clsVertex*	 m_v2;
		clsTriangle*					 m_t1;
		clsTriangle*					 m_t2;
		clsLIE*							 m_LIE1;
		clsLIE*							 m_LIE2;
		bool							 m_IsReverseInLIE;
	};
}
