#pragma once
#include "stdafx.h"

#include "../OpenGLLib/clsVertex.h"

namespace GenerationLib
{
	class clsPathVertex : public OpenGLLib::Skeleton::clsVertex
	{
	public:

		clsPathVertex( );
		~clsPathVertex( );

		void SetConnectedPathVertex( clsPathVertex* PathVertex );
		void SetLIE( clsLIE* LIE );
		void SetBNPPointer( clsBNPObject* BNPObject );

		clsPathVertex* GetConnectedPathVertex( );
		clsLIE* GetLIE( );

	private:

		clsLIE * m_LIE;
		clsPathVertex*	m_ConnectedPathVertex;
		clsBNPObject*	m_BNPObject;
	};
}
