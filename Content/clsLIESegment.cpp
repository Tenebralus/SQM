#pragma once
#include "stdafx.h"
#include "clsLIESegment.h"
#include "clsEdge.h"
#include "clsBNPObject.h"
#include "clsSQMDebugData.h"

#if ENABLE_SQM
namespace GenerationLib
{
	clsLIESegment::clsLIESegment( )
	{
		m_PathVertex1 = nullptr;
		m_PathVertex2 = nullptr;
		m_IsCompleted = false;
	}

	clsLIESegment::~clsLIESegment( )
	{

	}

	void clsLIESegment::AddEdge( clsEdge* a_Edge )
	{
		m_Edges.push_back( a_Edge );
	}

	void clsLIESegment::AddEdgeAt( clsEdge* a_Edge, size_t a_Index )
	{
		m_Edges.insert( m_Edges.begin( ) + a_Index, a_Edge );
	}

	void clsLIESegment::SetQuaternion( glm::vec4 a_Quaternion )
	{
		m_Quaternion = a_Quaternion;
	}

	void clsLIESegment::SetPathVertices( clsPathVertex* a_PathVertex1, clsPathVertex* a_PathVertex2 )
	{
		m_PathVertex1 = a_PathVertex1;
		m_PathVertex2 = a_PathVertex2;
	}

	void clsLIESegment::SetIsCompleted( bool a_IsCompleted )
	{
		m_IsCompleted = a_IsCompleted;
	}

	void clsLIESegment::SortAsChain( glm::vec3 a_PathVerticesCenter )
	{
		OpenGLLib::Skeleton::clsVertex* l_FirstVertex = nullptr;
		OpenGLLib::Skeleton::clsVertex* l_LastVertex = nullptr;
		FindOuterVertices( &l_FirstVertex, &l_LastVertex );

		OpenGLLib::Skeleton::clsVertex* l_CurrVertex = l_FirstVertex;
		clsEdge* l_LastEdge = nullptr;
		std::vector<clsEdge*> l_SortedEdges;

		while (l_SortedEdges.size( ) != m_Edges.size( ))
		{
			clsEdge* l_Edge = FindNextEdge( l_CurrVertex, l_LastEdge );
			l_SortedEdges.push_back( l_Edge );
			l_CurrVertex = l_Edge->GetVertexOppositeOf( l_CurrVertex );
			l_LastEdge = l_Edge;
		}
		m_Edges = l_SortedEdges;

		InitQuaternion( l_FirstVertex, l_LastVertex, a_PathVerticesCenter );
	}

	void clsLIESegment::InitQuaternion( OpenGLLib::Skeleton::clsVertex* a_FirstVertex, OpenGLLib::Skeleton::clsVertex* a_LastVertex, glm::vec3 a_PathVerticesCenter )
	{
		glm::vec3 l_FirstVertexPos = a_FirstVertex->GetPosition( ) - a_PathVerticesCenter;
		glm::vec3 l_LastVertexPos = a_LastVertex->GetPosition( ) - a_PathVerticesCenter;
		glm::vec3 l_Axis = glm::normalize( glm::cross( l_FirstVertexPos, l_LastVertexPos ) );
		m_Quaternion = BrawlerEngineLib::clsMath::This( ).SQMQuaternionBetweenVectorsWithAxis( l_FirstVertexPos, l_LastVertexPos, l_Axis );
	}

	void clsLIESegment::Smooth( glm::vec3 a_PathVerticesCenter, float a_JointRadius, glm::vec3 a_JointPosition )
	{
		OpenGLLib::Skeleton::clsVertex* l_FirstVertex = nullptr;
		OpenGLLib::Skeleton::clsVertex* l_LastVertex = nullptr;
		FindOuterVertices( &l_FirstVertex, &l_LastVertex );

		float l_Div = (float) m_Edges.size( );
		float l_Alpha = glm::acos( m_Quaternion.w ) * 2.0f / l_Div;
		float l_PartAlpha = l_Alpha;
		glm::vec3 l_Axis = glm::normalize( glm::vec3( m_Quaternion ) );
		glm::vec3 l_Offset = a_PathVerticesCenter;
		glm::vec3 l_v = l_FirstVertex->GetPosition( ) - l_Offset;
		OpenGLLib::Skeleton::clsVertex* l_CurrVertex = l_FirstVertex;
		clsEdge* l_LastEdge = nullptr;
		for (size_t i = 0; i < m_Edges.size( ) - 1; ++i)
		{
			glm::vec4 l_q = BrawlerEngineLib::clsMath::This( ).QuaternionFromAngleAxis( l_Alpha, l_Axis );
			glm::vec3 l_u = BrawlerEngineLib::clsMath::This( ).QuaternionRotateVector( l_q, l_v );
			l_u += l_Offset;
			glm::vec3 l_p = glm::vec3( l_u.x, l_u.y, l_u.z );
			glm::vec3 l_RayOrigin = a_PathVerticesCenter;
			glm::vec3 l_RayDir = glm::normalize( l_p - a_PathVerticesCenter );
			glm::vec3 l_RayEnd = l_RayOrigin + l_RayDir * a_JointRadius * 10.0f;
			float l_t = 0.0f;
			float l_Radius2 = a_JointRadius * a_JointRadius;
			bool l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_RayOrigin, l_RayEnd - l_RayOrigin, a_JointPosition, l_Radius2, &l_t );
			assert( l_Intersected );
			l_t = glm::distance( l_RayOrigin, l_RayEnd ) * l_t;
			l_p = l_RayOrigin + ( l_t *  l_RayDir );

			l_CurrVertex = FindNextVertex( l_CurrVertex, l_LastEdge );
			l_LastEdge = m_Edges[ i ];
			l_CurrVertex->SetPosition( l_p );

			l_Alpha += l_PartAlpha;
		}
	}

	void clsLIESegment::Smooth2( glm::vec3 a_PathVerticesCenter, float a_JointRadius, glm::vec3 a_JointPosition, clsBNPObject* a_BNPObject )
	{
		OpenGLLib::Skeleton::clsVertex* l_FirstVertex = nullptr;
		OpenGLLib::Skeleton::clsVertex* l_LastVertex = nullptr;
		FindOuterVertices( &l_FirstVertex, &l_LastVertex );

		OpenGLLib::Skeleton::clsVertex* l_CurrVertex = l_LastVertex;
		l_CurrVertex = FindNextVertex( l_CurrVertex, nullptr );
		for (size_t i = m_Edges.size( ) - 1; i >= 1; --i)
		{
			std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices = a_BNPObject->GetVerticesConnectedToVertex( l_CurrVertex );
			glm::vec3 l_p( 0, 0, 0 );
			for (size_t j = 0; j < l_Vertices.size( ); ++j)
			{
				l_p += l_Vertices[ j ]->GetPosition( );
			}
			l_p /= (float) l_Vertices.size( );

			/*l_CurrVertex->SetPosition( l_p );
			l_CurrVertex = FindNextVertex( l_CurrVertex, m_Edges[ i ] );*/

			glm::vec3 l_Normal = glm::normalize( l_p - a_PathVerticesCenter );
			glm::vec3 l_FromPosition = l_p;
			glm::vec3 l_ToPosition = l_p + l_Normal * a_JointRadius * 10.0f;
			float l_DistanceNormalized = 0.0f;
			bool l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_JointPosition, a_JointRadius * a_JointRadius, &l_DistanceNormalized );
			if (l_Intersected)
			{
				float l_Distance = glm::distance( l_FromPosition, l_ToPosition ) * l_DistanceNormalized;
				l_CurrVertex->SetPosition( l_p + l_Normal * l_Distance );
				l_CurrVertex = FindNextVertex( l_CurrVertex, m_Edges[ i ] );
			}
			else
			{
				assert( "Could not find ray-sphere intersection." == 0 );
			}
		}
	}

	void clsLIESegment::Smooth3( glm::vec3 a_PathVerticesCenter, float a_JointRadius, glm::vec3 a_JointPosition )
	{
		OpenGLLib::Skeleton::clsVertex* l_FirstVertex = nullptr;
		OpenGLLib::Skeleton::clsVertex* l_LastVertex = nullptr;
		FindOuterVertices( &l_FirstVertex, &l_LastVertex );

		OpenGLLib::Skeleton::clsVertex* l_CurrVertex = l_FirstVertex;
		l_CurrVertex = FindNextVertex( l_CurrVertex, nullptr );
		for (size_t i = 0; i < m_Edges.size( ) - 1; ++i)
		{
			glm::vec3 l_FirstToLastNormal = glm::normalize( l_LastVertex->GetPosition( ) - l_FirstVertex->GetPosition( ) );
			float l_FirstToLastDistance = glm::distance( l_FirstVertex->GetPosition( ), l_LastVertex->GetPosition( ) );
			float l_CurrVertPercent = (float) ( i + 1 ) / (float) m_Edges.size( );
			glm::vec3 l_p = l_FirstVertex->GetPosition( ) + l_FirstToLastNormal * l_FirstToLastDistance * l_CurrVertPercent;

			/*l_CurrVertex->SetPosition( l_p );
			l_CurrVertex = FindNextVertex( l_CurrVertex, m_Edges[ i ] );*/

			glm::vec3 l_Normal = glm::normalize( l_p - a_PathVerticesCenter );
			glm::vec3 l_FromPosition = l_p;
			glm::vec3 l_ToPosition = l_p + l_Normal * a_JointRadius * 10.0f;
			float l_DistanceNormalized = 0.0f;
			bool l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_JointPosition, a_JointRadius * a_JointRadius, &l_DistanceNormalized );
			if (l_Intersected)
			{
				float l_Distance = glm::distance( l_FromPosition, l_ToPosition ) * l_DistanceNormalized;
				l_CurrVertex->SetPosition( l_p + l_Normal * l_Distance );
				l_CurrVertex = FindNextVertex( l_CurrVertex, m_Edges[ i ] );
			}
			else
			{
				assert( "Could not find ray-sphere intersection." == 0 );
			}
		}
	}

	void clsLIESegment::RemoveEdge( clsEdge* a_Edge )
	{
		if (std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) == m_Edges.end( ))
		{
			assert( "m_Edges does not contain input." == 0 );
		}
		m_Edges.erase( std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) );
	}

	std::vector<clsEdge*> clsLIESegment::GetEdges( )
	{
		return m_Edges;
	}

	glm::vec4 clsLIESegment::GetQuaternion( )
	{
		return m_Quaternion;
	}

	clsPathVertex* clsLIESegment::GetPathVertex1( )
	{
		return m_PathVertex1;
	}

	clsPathVertex* clsLIESegment::GetPathVertex2( )
	{
		return m_PathVertex2;
	}

	clsPathVertex* clsLIESegment::GetOtherPathVertex( clsPathVertex* a_PathVertex )
	{
		if (m_PathVertex1 == a_PathVertex)
		{
			return m_PathVertex2;
		}
		else if (m_PathVertex2 == a_PathVertex)
		{
			return m_PathVertex1;
		}
		else
		{
			assert( "Segment does not contain input." == 0 );
			return nullptr;
		}
	}

	bool clsLIESegment::IsPathVertexConnectedToLIESegment( clsPathVertex* a_PathVertex1, clsPathVertex* a_PathVertex2 )
	{
		assert( m_PathVertex1 != nullptr && m_PathVertex2 != nullptr && a_PathVertex1 != nullptr && a_PathVertex2 != nullptr );

		if (m_PathVertex1 == a_PathVertex1 && m_PathVertex2 == a_PathVertex2 ||
			 m_PathVertex1 == a_PathVertex2 && m_PathVertex2 == a_PathVertex1)
		{
			return true;
		}
		return false;
	}

	bool clsLIESegment::IsCompleted( )
	{
		return m_IsCompleted;
	}

	OpenGLLib::Skeleton::clsVertex* clsLIESegment::FindNextVertex( OpenGLLib::Skeleton::clsVertex* a_Vertex, clsEdge* a_LastEdge )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			if (m_Edges[ i ]->GetVertex1( ) == a_Vertex && m_Edges[ i ] != a_LastEdge)
			{
				return m_Edges[ i ]->GetVertex2( );
			}
			else if (m_Edges[ i ]->GetVertex2( ) == a_Vertex && m_Edges[ i ] != a_LastEdge)
			{
				return m_Edges[ i ]->GetVertex1( );
			}
		}

		assert( "Next vertex not found." == 0 );
		return nullptr;
	}

	clsEdge* clsLIESegment::FindNextEdge( OpenGLLib::Skeleton::clsVertex* a_Vertex, clsEdge* a_LastEdge )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			if (m_Edges[ i ]->GetVertex1( ) == a_Vertex && m_Edges[ i ] != a_LastEdge ||
				 m_Edges[ i ]->GetVertex2( ) == a_Vertex && m_Edges[ i ] != a_LastEdge)
			{
				return m_Edges[ i ];
			}
		}

		assert( "Next Edge not found." == 0 );
		return nullptr;
	}

	void clsLIESegment::FindOuterVertices( OpenGLLib::Skeleton::clsVertex** r_FirstVertex, OpenGLLib::Skeleton::clsVertex** r_LastVertex )
	{
		std::vector<VertexCount> l_VertexCountVector;
		FillVertexCounter( l_VertexCountVector );

		bool l_IsFirstVertexSet = false;
		bool l_IsLastVertexSet = false;
		for (size_t i = 0; i < l_VertexCountVector.size( ); ++i)
		{
			if (l_VertexCountVector[ i ].Count == 1)
			{
				if (!l_IsFirstVertexSet)
				{
					*r_FirstVertex = l_VertexCountVector[ i ].Vertex;
					l_IsFirstVertexSet = true;
				}
				else if (!l_IsLastVertexSet)
				{
					*r_LastVertex = l_VertexCountVector[ i ].Vertex;
					l_IsLastVertexSet = true;
				}
			}
		}
		assert( l_IsFirstVertexSet && l_IsLastVertexSet );
	}

	void clsLIESegment::FillVertexCounter( std::vector<VertexCount>& a_VertexCountVector )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			OpenGLLib::Skeleton::clsVertex* l_v1 = m_Edges[ i ]->GetVertex1( );
			OpenGLLib::Skeleton::clsVertex* l_v2 = m_Edges[ i ]->GetVertex2( );
			bool l_IsVertex1InVector = false;
			bool l_IsVertex2InVector = false;
			for (size_t j = 0; j < a_VertexCountVector.size( ); ++j)
			{
				if (a_VertexCountVector[ j ].Vertex == l_v1)
				{
					++a_VertexCountVector[ j ].Count;
					l_IsVertex1InVector = true;
				}
				else if (a_VertexCountVector[ j ].Vertex == l_v2)
				{
					++a_VertexCountVector[ j ].Count;
					l_IsVertex2InVector = true;
				}
			}
			if (!l_IsVertex1InVector)
			{
				VertexCount l_VertexCount;
				l_VertexCount.Vertex = l_v1;
				++l_VertexCount.Count;
				a_VertexCountVector.push_back( l_VertexCount );
			}
			if (!l_IsVertex2InVector)
			{
				VertexCount l_VertexCount;
				l_VertexCount.Vertex = l_v2;
				++l_VertexCount.Count;
				a_VertexCountVector.push_back( l_VertexCount );
			}
		}
	}

	clsEdge* clsLIESegment::GetEdgeToSplit( )
	{
		return m_Edges[ 0 ];
	}
}
#endif
