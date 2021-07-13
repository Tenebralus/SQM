#pragma once
#include "stdafx.h"
#include "clsLIE.h"
#include "clsEdge.h"
#include "clsBNPObject.h"

#if ENABLE_SQM
namespace GenerationLib
{
	clsLIE::clsLIE( )
	{
		m_RequiredAmountOfSplits = 0;
	}

	clsLIE::~clsLIE( )
	{

	}

	void clsLIE::AddEdge( clsEdge* a_Edge )
	{
		m_Edges.push_back( a_Edge );
		m_Edges[ m_Edges.size( ) - 1 ]->SetLIE( this );
	}

	void clsLIE::SetEdges( std::vector<clsEdge*> a_Edges )
	{
		m_Edges = a_Edges;
	}

	void clsLIE::SetBNPObject( clsBNPObject* a_BNPObject )
	{
		m_BNPObject = a_BNPObject;
	}

	void clsLIE::SortAsChain( )
	{
		OpenGLLib::Skeleton::clsVertex* l_CurrVertex = m_VertexOrder[ 1 ];
		clsEdge* l_FirstEdge = FindEdge( m_VertexOrder[ 0 ], m_VertexOrder[ 1 ] );
		clsEdge* l_LastEdge = l_FirstEdge;
		std::vector<clsEdge*> l_SortedEdges;
		l_SortedEdges.push_back( l_FirstEdge );

		while (l_SortedEdges.size( ) != m_Edges.size( ))
		{
			for (size_t i = 0; i < m_Edges.size( ); ++i)
			{
				glm::vec3 l_cv1 = l_CurrVertex->GetPosition( );
				glm::vec3 l_iv1 = m_Edges[ i ]->GetVertex1( )->GetPosition( );
				glm::vec3 l_iv2 = m_Edges[ i ]->GetVertex2( )->GetPosition( );
				if (l_cv1 == l_iv1 || l_cv1 == l_iv2)
				{
					if (m_Edges[ i ] != l_LastEdge)
					{
						l_SortedEdges.push_back( m_Edges[ i ] );
						l_LastEdge = m_Edges[ i ];
						l_CurrVertex = m_Edges[ i ]->GetVertexOppositeOf( l_CurrVertex );
						break;
					}
				}
			}
		}

		m_Edges = l_SortedEdges;

		SortVertices( );
	}

	void clsLIE::SortVertices( )
	{
		m_Vertices.clear( );
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			m_Vertices.push_back( GetVertexAt( i ) );
		}
	}

	void clsLIE::SetRotation( glm::mat3 a_RotationMatrix )
	{
		m_RotationMatrix = a_RotationMatrix;
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			m_Edges[ i ]->GetVertex1( )->SetLocalPosition( m_Edges[ i ]->GetVertex1( )->GetPosition( ) - *m_BNPObject->GetJointPos( ) );
			m_Edges[ i ]->GetVertex2( )->SetLocalPosition( m_Edges[ i ]->GetVertex2( )->GetPosition( ) - *m_BNPObject->GetJointPos( ) );

			m_Edges[ i ]->GetVertex1( )->SetPosition( m_RotationMatrix * m_Edges[ i ]->GetVertex1( )->GetLocalPosition( ) + *m_BNPObject->GetJointPos( ) );
			m_Edges[ i ]->GetVertex2( )->SetPosition( m_RotationMatrix * m_Edges[ i ]->GetVertex2( )->GetLocalPosition( ) + *m_BNPObject->GetJointPos( ) );
		}
	}

	void clsLIE::AddLIESegment( clsLIESegment* a_Segment )
	{
		m_LIESegments.push_back( a_Segment );
	}

	void clsLIE::SetPathVertex( clsPathVertex* a_PathVertex )
	{
		m_PathVertex = a_PathVertex;
	}

	void clsLIE::SetVertexOrder( std::vector<OpenGLLib::Skeleton::clsVertex*> a_Vertices )
	{
		m_VertexOrder = a_Vertices;
	}

	void clsLIE::RemoveEdge( clsEdge* a_Edge )
	{
		if (std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) == m_Edges.end( ))
		{
			assert( "m_Edges does not contain input." == 0 );
		}
		m_Edges.erase( std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) );
	}

	void clsLIE::Reverse( )
	{
		std::reverse( m_Edges.begin( ), m_Edges.end( ) );
		std::reverse( m_Vertices.begin( ), m_Vertices.end( ) );
	}

	std::vector<clsEdge*>* clsLIE::GetEdges( )
	{
		return &m_Edges;
	}

	clsBNPObject* clsLIE::GetBNPObject( )
	{
		return m_BNPObject;
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*>* clsLIE::GetVertices( )
	{
		return &m_Vertices;
	}

	clsEdge* clsLIE::FindEdge( OpenGLLib::Skeleton::clsVertex* a_Vertex1, OpenGLLib::Skeleton::clsVertex* a_Vertex2 )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			OpenGLLib::Skeleton::clsVertex* l_FirstVertex = m_Edges[ i ]->GetVertex1( );
			OpenGLLib::Skeleton::clsVertex* l_LastVertex = m_Edges[ i ]->GetVertex2( );
			if (l_FirstVertex == a_Vertex1 && l_LastVertex == a_Vertex2 ||
				 l_FirstVertex == a_Vertex2 && l_LastVertex == a_Vertex1)
			{
				return m_Edges[ i ];
			}
		}

		return nullptr;
	}

	std::vector<clsEdge*> clsLIE::FindEdges( OpenGLLib::Skeleton::clsVertex* a_Vertex )
	{
		std::vector<clsEdge*> l_Edges;
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			if (*m_Edges[ i ]->GetVertex1( ) == *a_Vertex || *m_Edges[ i ]->GetVertex2( ) == *a_Vertex)
			{
				l_Edges.push_back( m_Edges[ i ] );
			}
			if (l_Edges.size( ) == 2)
			{
				break;
			}
		}
		assert( l_Edges.size( ) == 2 );
		return l_Edges;
	}

	glm::mat3 clsLIE::GetRotation( )
	{
		return m_RotationMatrix;
	}

	std::vector<clsLIESegment*> clsLIE::GetLIESegments( )
	{
		return m_LIESegments;
	}

	bool clsLIE::DoesLIESegmentExist( clsLIESegment* a_LIESegment )
	{
		for (size_t i = 0; i < m_LIESegments.size( ); ++i)
		{
			if (m_LIESegments[ i ] == a_LIESegment)
			{
				return true;
			}
		}
		return false;
	}

	clsPathVertex* clsLIE::GetPathVertex( )
	{
		return m_PathVertex;
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*> clsLIE::GetFirstTwoVertices( )
	{
		std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices;
		l_Vertices.push_back( m_VertexOrder[ 0 ] );
		l_Vertices.push_back( m_VertexOrder[ 1 ] );
		return l_Vertices;
	}

	OpenGLLib::Skeleton::clsVertex* clsLIE::GetNextVertex( OpenGLLib::Skeleton::clsVertex* a_Vertex, clsEdge* a_LastEdge )
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

	OpenGLLib::Skeleton::clsVertex* clsLIE::GetCommonVertex( clsEdge* a_Edge1, clsEdge* a_Edge2 )
	{
		if (a_Edge1->GetVertex1( ) == a_Edge2->GetVertex1( ))
		{
			return a_Edge1->GetVertex1( );
		}
		else if (a_Edge1->GetVertex1( ) == a_Edge2->GetVertex2( ))
		{
			return a_Edge1->GetVertex1( );
		}
		else if (a_Edge1->GetVertex2( ) == a_Edge2->GetVertex1( ))
		{
			return a_Edge1->GetVertex2( );
		}
		else if (a_Edge1->GetVertex2( ) == a_Edge2->GetVertex2( ))
		{
			return a_Edge1->GetVertex2( );
		}
		else
		{
			assert( "Edges do not share a common vertex." == 0 );
			return nullptr;
		}
	}

	OpenGLLib::Skeleton::clsVertex* clsLIE::GetFirstVertex( )
	{
		return GetCommonVertex( m_Edges[ 0 ], m_Edges[ m_Edges.size( ) - 1 ] );
	}

	OpenGLLib::Skeleton::clsVertex* clsLIE::GetVertexAt( size_t a_Index )
	{
		int l_PreviousIndex = (int) a_Index - 1;
		if (l_PreviousIndex < 0)
		{
			l_PreviousIndex = (int) m_Edges.size( ) - 1;
		}
		return GetCommonVertex( m_Edges[ l_PreviousIndex ], m_Edges[ a_Index ] );
	}
}
#endif
