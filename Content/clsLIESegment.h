#pragma once
#include "stdafx.h"

#if ENABLE_SQM
namespace GenerationLib
{
	class clsLIESegment
	{
	public:
		struct VertexCount
		{
			OpenGLLib::Skeleton::clsVertex* Vertex;
			unsigned int Count = 0;
		};

		clsLIESegment( );
		~clsLIESegment( );

		void AddEdge( clsEdge* Edge );
		void AddEdgeAt( clsEdge* Edge, size_t Index );
		void SetQuaternion( glm::vec4 Quaternion );
		void SetPathVertices( clsPathVertex* PathVertex1, clsPathVertex* PathVertex2 );
		void SetIsCompleted( bool IsHandled );
		void SortAsChain( glm::vec3 PathVerticesCenter );
		void InitQuaternion( OpenGLLib::Skeleton::clsVertex* FirstVertex, OpenGLLib::Skeleton::clsVertex* LastVertex, glm::vec3 PathVerticesCenter );
		void Smooth( glm::vec3 PathVerticesCenter, float JointRadius, glm::vec3 JointPosition );
		void Smooth2( glm::vec3 PathVerticesCenter, float JointRadius, glm::vec3 JointPosition, clsBNPObject* BNPObject );
		void Smooth3( glm::vec3 PathVerticesCenter, float JointRadius, glm::vec3 JointPosition );
		void RemoveEdge( clsEdge* Edge );

		std::vector<clsEdge*> GetEdges( );
		glm::vec4 GetQuaternion( );
		clsPathVertex* GetPathVertex1( );
		clsPathVertex* GetPathVertex2( );
		clsPathVertex* GetOtherPathVertex( clsPathVertex* PathVertex );
		bool IsPathVertexConnectedToLIESegment( clsPathVertex* PathVertex1, clsPathVertex* PathVertex2 );
		bool IsCompleted( );
		OpenGLLib::Skeleton::clsVertex* FindNextVertex( OpenGLLib::Skeleton::clsVertex* Vertex, clsEdge* LastEdge );
		clsEdge* FindNextEdge( OpenGLLib::Skeleton::clsVertex* Vertex, clsEdge* LastEdge );
		void FindOuterVertices( OpenGLLib::Skeleton::clsVertex** r_FirstVertex, OpenGLLib::Skeleton::clsVertex** r_LastVertex );
		void FillVertexCounter( std::vector<VertexCount>& VertexCountVector );
		clsEdge* GetEdgeToSplit( );

		unsigned int m_DoneAmountOfSplits;
	private:
		std::vector<clsEdge*> m_Edges;
		glm::vec4 m_Quaternion;
		clsPathVertex* m_PathVertex1;
		clsPathVertex* m_PathVertex2;
		bool m_IsCompleted;
	};
}
#endif
