#pragma once
#include "stdafx.h"

#if ENABLE_SQM
namespace GenerationLib
{
	class clsLIE
	{
	public:
		clsLIE( );
		~clsLIE( );

		void AddEdge( clsEdge* Edge );
		void SetEdges( std::vector<clsEdge*> Edges );
		void SetBNPObject( clsBNPObject* BNPObject );
		void SortAsChain( );
		void SortVertices( );
		void SetRotation( glm::mat3 RotationMatrix );
		void AddLIESegment( clsLIESegment* Segment );
		void SetPathVertex( clsPathVertex* PathVertex );
		void SetVertexOrder( std::vector<OpenGLLib::Skeleton::clsVertex*> Vertices );
		void RemoveEdge( clsEdge* Edge );
		void Reverse( );

		std::vector<clsEdge*>* GetEdges( );
		clsBNPObject* GetBNPObject( );
		std::vector<OpenGLLib::Skeleton::clsVertex*>* GetVertices( );
		clsEdge* FindEdge( OpenGLLib::Skeleton::clsVertex* Vertex1, OpenGLLib::Skeleton::clsVertex* Vertex2 );
		std::vector<clsEdge*> FindEdges( OpenGLLib::Skeleton::clsVertex* Vertex );
		glm::mat3 GetRotation( );
		std::vector<clsLIESegment*> GetLIESegments( );
		bool DoesLIESegmentExist( clsLIESegment* LIESegment );
		clsPathVertex* GetPathVertex( );
		std::vector<OpenGLLib::Skeleton::clsVertex*> GetFirstTwoVertices( );
		OpenGLLib::Skeleton::clsVertex* GetNextVertex( OpenGLLib::Skeleton::clsVertex* Vertex, clsEdge* LastEdge ); // only works with sorted LIE
		OpenGLLib::Skeleton::clsVertex* GetCommonVertex( clsEdge* Edge1, clsEdge* Edge2 ); // only works with sorted LIE
		OpenGLLib::Skeleton::clsVertex* GetFirstVertex( ); // only works with sorted LIE
		OpenGLLib::Skeleton::clsVertex* GetVertexAt( size_t Index ); // only works with sorted LIE

		unsigned int m_RequiredAmountOfSplits;
	private:
		std::vector<clsEdge*> m_Edges;
		std::vector<OpenGLLib::Skeleton::clsVertex*> m_Vertices;
		clsBNPObject* m_BNPObject;
		glm::mat3 m_RotationMatrix;
		std::vector<clsLIESegment*> m_LIESegments;
		clsPathVertex* m_PathVertex;
		std::vector<OpenGLLib::Skeleton::clsVertex*> m_VertexOrder; // 2 vertices defining direction in LIE
	};
}
#endif
