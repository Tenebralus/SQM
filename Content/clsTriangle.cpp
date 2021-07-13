#pragma once
#include "stdafx.h"
#include "clsTriangle.h"
#include "clsEdge.h"
#include "clsBNPObject.h"

#include "../OpenGLLib/clsVertex.h"

#include "clsSQMDebugData.h"

namespace GenerationLib
{
	clsTriangle::clsTriangle( )
	{

	}

	clsTriangle::clsTriangle( OpenGLLib::Skeleton::clsVertex* a_p1, OpenGLLib::Skeleton::clsVertex* a_p2, OpenGLLib::Skeleton::clsVertex* a_p3, clsEdge* a_e1, clsEdge* a_e2, clsEdge* a_e3 )
	{
		Init( a_p1, a_p2, a_p3, a_e1, a_e2, a_e3 );
	}

	clsTriangle::~clsTriangle( )
	{

	}

	void clsTriangle::Init( OpenGLLib::Skeleton::clsVertex* a_p1, OpenGLLib::Skeleton::clsVertex* a_p2, OpenGLLib::Skeleton::clsVertex* a_p3, clsEdge* a_e1, clsEdge* a_e2, clsEdge* a_e3 )
	{
		SetTriangle( a_p1, a_p2, a_p3, a_e1, a_e2, a_e3 );
	}

	void clsTriangle::Destroy( )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			delete m_Edges[ i ];
		}
		m_Edges.clear( );
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			delete m_Vertices[ i ];
		}
		m_Vertices.clear( );
	}

	void clsTriangle::SetTriangle( OpenGLLib::Skeleton::clsVertex* a_p1, OpenGLLib::Skeleton::clsVertex* a_p2, OpenGLLib::Skeleton::clsVertex* a_p3, clsEdge* a_e1, clsEdge* a_e2, clsEdge* a_e3 )
	{
		m_Vertices.push_back( a_p1 );
		m_Vertices.push_back( a_p2 );
		m_Vertices.push_back( a_p3 );
		m_Edges.push_back( a_e1 );
		m_Edges.push_back( a_e2 );
		m_Edges.push_back( a_e3 );
	}

	void clsTriangle::PointEdgesToTriangle( )
	{
		for (int i = 0; i < m_Edges.size( ); ++i)
		{
			m_Edges[ i ]->PointToTriangle( this );
		}
	}

	void clsTriangle::SetNormal( )
	{
		m_Normal = glm::normalize( glm::cross( m_Vertices[ 1 ]->GetPosition( ) - m_Vertices[ 0 ]->GetPosition( ), m_Vertices[ 2 ]->GetPosition( ) - m_Vertices[ 1 ]->GetPosition( ) ) );
	}

	void clsTriangle::SetNormal( glm::vec3 a_Normal )
	{
		m_Normal = a_Normal;
	}

	void clsTriangle::SetBNPObject( clsBNPObject* a_BNPObject )
	{
		m_BNPObject = a_BNPObject;
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*>* clsTriangle::GetVertices( )
	{
		return &m_Vertices;
	}

	std::vector<clsEdge*>* clsTriangle::GetEdges( )
	{
		return &m_Edges;
	}

	clsBNPObject* clsTriangle::GetBNPObject( )
	{
		return m_BNPObject;
	}

	clsEdge* clsTriangle::GetEdgePointerOppositeToVertex( OpenGLLib::Skeleton::clsVertex a_Vertex )
	{
		for (int i = 0; i < m_Edges.size( ); ++i)
		{
			if (*m_Edges[ i ]->GetVertex1( ) != a_Vertex && *m_Edges[ i ]->GetVertex2( ) != a_Vertex)
			{
				return m_Edges[ i ];
			}
		}
		assert( "Vertex was not found in triangle" == 0 );
		return nullptr;
	}

	glm::vec3 clsTriangle::GetCenter( )
	{
		return glm::vec3( ( m_Vertices[ 0 ]->GetPosition( ).x + m_Vertices[ 1 ]->GetPosition( ).x + m_Vertices[ 2 ]->GetPosition( ).x ) / 3.0f, ( m_Vertices[ 0 ]->GetPosition( ).y + m_Vertices[ 1 ]->GetPosition( ).y + m_Vertices[ 2 ]->GetPosition( ).y ) / 3.0f, ( m_Vertices[ 0 ]->GetPosition( ).z + m_Vertices[ 1 ]->GetPosition( ).z + m_Vertices[ 2 ]->GetPosition( ).z ) / 3.0f );
	}

	glm::vec3 clsTriangle::GetNormal( )
	{
		return m_Normal;
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*> clsTriangle::GetVerticesOppositeToVertexInWindingOrder( glm::vec3 a_Vertex )
	{
		std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices;
		if (m_Vertices[ 0 ]->GetPosition( ) == a_Vertex)
		{
			l_Vertices.push_back( m_Vertices[ 1 ] );
			l_Vertices.push_back( m_Vertices[ 2 ] );
		}
		else if (m_Vertices[ 1 ]->GetPosition( ) == a_Vertex)
		{
			l_Vertices.push_back( m_Vertices[ 2 ] );
			l_Vertices.push_back( m_Vertices[ 0 ] );
		}
		else if (m_Vertices[ 2 ]->GetPosition( ) == a_Vertex)
		{
			l_Vertices.push_back( m_Vertices[ 0 ] );
			l_Vertices.push_back( m_Vertices[ 1 ] );
		}
		else
		{
			assert( "Triangle does not contain input vertex." == 0 );
		}
		return l_Vertices;
	}

	OpenGLLib::Skeleton::clsVertex* clsTriangle::GetVertexOppositeToEdge( clsEdge* a_Edge )
	{
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			if (m_Vertices[ i ] != a_Edge->GetVertex1( ) && m_Vertices[ i ] != a_Edge->GetVertex2( ))
			{
				return m_Vertices[ i ];
			}
		}

		assert( "m_Vertices does not contain data from input." == 0 );
		return nullptr;
	}
}
