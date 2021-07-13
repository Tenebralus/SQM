#pragma once
#include "stdafx.h"

#include "../OpenGLLib/clsVertex.h"

namespace GenerationLib
{
	class clsTriangle
	{
	public:
		clsTriangle( );
		clsTriangle( OpenGLLib::Skeleton::clsVertex* p1, OpenGLLib::Skeleton::clsVertex* p2, OpenGLLib::Skeleton::clsVertex* p3, clsEdge* e1, clsEdge* e2, clsEdge* e3 );
		~clsTriangle( );

		void					 Init( OpenGLLib::Skeleton::clsVertex* p1, OpenGLLib::Skeleton::clsVertex* p2, OpenGLLib::Skeleton::clsVertex* p3, clsEdge* e1, clsEdge* e2, clsEdge* e3 );
		void					 Destroy( );
		void					 SetTriangle( OpenGLLib::Skeleton::clsVertex* p1, OpenGLLib::Skeleton::clsVertex* p2, OpenGLLib::Skeleton::clsVertex* p3, clsEdge* e1, clsEdge* e2, clsEdge* e3 );
		void					 PointEdgesToTriangle( );
		void					 SetNormal( );
		void					 SetNormal( glm::vec3 Normal );
		void					 SetBNPObject( clsBNPObject* BNPObject );


		std::vector<OpenGLLib::Skeleton::clsVertex*>* GetVertices( );
		std::vector<clsEdge*>*						  GetEdges( );
		clsBNPObject*								  GetBNPObject( );
		clsEdge*									  GetEdgePointerOppositeToVertex( OpenGLLib::Skeleton::clsVertex Vertex );
		glm::vec3									  GetCenter( );
		glm::vec3									  GetNormal( );
		std::vector<OpenGLLib::Skeleton::clsVertex*>  GetVerticesOppositeToVertexInWindingOrder( glm::vec3 Vertex );
		OpenGLLib::Skeleton::clsVertex*				  GetVertexOppositeToEdge( clsEdge* Edge );

		bool operator==( clsTriangle OtherTriangle )
		{
			return m_Vertices[ 0 ]->GetPosition( ) == OtherTriangle.GetVertices( )->at( 0 )->GetPosition( ) &&
				m_Vertices[ 1 ]->GetPosition( ) == OtherTriangle.GetVertices( )->at( 1 )->GetPosition( ) &&
				m_Vertices[ 2 ]->GetPosition( ) == OtherTriangle.GetVertices( )->at( 2 )->GetPosition( );
		}

	private:
		std::vector<clsEdge*>							m_Edges;
		std::vector<OpenGLLib::Skeleton::clsVertex*> m_Vertices;

		glm::vec3						m_Normal;
		clsBNPObject*					m_BNPObject;
		OpenGLLib::Skeleton::clsVertex*	m_VertexAtSplit;
	};
}
