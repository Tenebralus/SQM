#pragma once
#include "stdafx.h"
#include "clsBNPObject.h"

#if ENABLE_SQM
#include "clsTriangle.h"
#include "clsPathVertex.h"
#include "clsEdge.h"
#include "clsLIE.h"
#include "clsSQMDebugData.h"
#include "clsGeneratedObject.h"
#include "clsLIESegment.h"

#include "../OpenGLLib/clsObject.h"
#include "../OpenGLLib/clsModel.h"
#include "../OpenGLLib/clsRenderPass.h"
#include "../OpenGLLib/clsOpenGL.h"
#include "../OpenGLLib/clsJoint.h"
#include "../OpenGLLib/clsBone.h"

#include "../GenerationLib/clsDelaunay3D.h"

#include "../BrawlerEngineLib/Delaunay/delaunay.h"
#include "../BrawlerEngineLib/Delaunay/triangle.h"
#include "../BrawlerEngineLib/Delaunay/clsDelaunay.h"

// for testing lines
#include "../BrawlerEngineLib/Delaunay/clsTriangle.h"
#include "../BrawlerEngineLib/Delaunay/clsEdge.h"
#include "../BrawlerEngineLib/Delaunay/clsVertex.h"

namespace GenerationLib
{
	clsBNPObject::clsBNPObject( )
	{
		ResetRotation( );
		m_Del3D = new clsDelaunay3D( );
		m_RenderPass = new OpenGLLib::clsRenderPass( );
		m_Delaunay = new BrawlerEngineLib::Delaunay::clsDelaunay( );
		m_SubdivideAmount = 6;
		m_IsRoot = false;
	}

	clsBNPObject::~clsBNPObject( )
	{
		delete m_Del3D;
		m_Del3D = nullptr;
		delete m_RenderPass;
		m_RenderPass = nullptr;
		delete m_Delaunay;
		m_Delaunay = nullptr;
	}

	std::vector<clsTriangle> clsBNPObject::GenerateBNP( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		std::vector<clsTriangle> l_RetValue;
		m_AmountOfVertsInTriangle = 3;
		m_StraightSkeletonJointPos = a_CurrentJoint->m_Position;
		m_JointPos = &a_CurrentJoint->m_Position;
		m_JointThickness = a_CurrentJoint->m_Thickness;

		GenerateUnrefinedBNP( a_CurrentJoint );
		a_CurrentJoint->SetBNPObject( this );

		return l_RetValue;
	}

	void clsBNPObject::GenerateUnrefinedBNP( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		// There should be at least one bone
		assert( a_CurrentJoint->m_Bones.size( ) > 0 );
		SetupPathVertices( a_CurrentJoint );
		std::vector<glm::vec3> l_PathVertCoords;
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			l_PathVertCoords.push_back( m_PathVertices[ i ]->GetPosition( ) );
		}

		// branch points branch out in at least 3 directions
		assert( m_PathVertices.size( ) >= 3 );

		Triangulate( l_PathVertCoords );
		m_Triangles.reserve( m_Triangles.size( ) * 6 );
		SubdivideBNP( );
		Project( a_CurrentJoint->m_Position, a_CurrentJoint->m_Thickness, l_PathVertCoords );
		SetupLIE( a_CurrentJoint->m_Thickness );
	}

	void clsBNPObject::Triangulate( std::vector<glm::vec3> a_Vertices )
	{
		if (!ArePoints3D( a_Vertices ))
		{
			Triangulate2D( a_Vertices );
		}
		else
		{
			Triangulate3D( a_Vertices );
		}
	}

	void clsBNPObject::Triangulate2D( std::vector<glm::vec3> a_Vertices )
	{
		// FIX: should work with more than 3 vertices. Write own Delaunay? check CGAL?
		SetupTriangle( a_Vertices[ 0 ], a_Vertices[ 1 ], a_Vertices[ 2 ] );
	}

	void clsBNPObject::Triangulate3D( std::vector<glm::vec3> a_Points )
	{
		std::sort( a_Points.begin( ), a_Points.end( ), [ ] ( glm::vec3 p1, glm::vec3 p2 )
		{
			return p1.x < p2.x;
		} );

		Set3DSurfaceTriangulation( a_Points );
	}

	bool clsBNPObject::ArePoints3D( std::vector<glm::vec3> a_Points )
	{
		glm::vec3 l_Normal;
		for (unsigned int i = 0; i < a_Points.size( ); ++i)
		{
			glm::vec3 l_TrianglePoint1 = a_Points[ i + 0 ];
			glm::vec3 l_TrianglePoint2 = a_Points[ i + 1 ];
			glm::vec3 l_TrianglePoint3;

			if (a_Points.size( ) > i + 2)
			{
				l_TrianglePoint3 = a_Points[ i + 2 ];
			}
			else
			{
				return false;
			}

			glm::vec3 l_NormalToCompare;
			bool l_IsNormalReverse = false;
			if (i == 0)
			{
				l_Normal = glm::normalize( glm::cross( l_TrianglePoint2 - l_TrianglePoint1, l_TrianglePoint3 - l_TrianglePoint1 ) );
			}
			else if (i >= 1)
			{
				l_NormalToCompare = glm::normalize( glm::cross( l_TrianglePoint2 - l_TrianglePoint1, l_TrianglePoint3 - l_TrianglePoint1 ) );

				if (glm::distance( l_Normal, l_NormalToCompare * glm::vec3( -1.0f, -1.0f, -1.0f ) ) < 0.01f)
				{
					l_IsNormalReverse = true;
				}

				if (l_IsNormalReverse)
				{
					l_NormalToCompare *= glm::vec3( -1.0f, -1.0f, -1.0f );
				}

				if (glm::distance( l_Normal, l_NormalToCompare ) > 0.01f)
				{
					return true;
				}
			}
		}
		return false;
	}

	void clsBNPObject::Set3DSurfaceTriangulation( std::vector<glm::vec3> a_Points )
	{
		std::vector<clsTriangle> l_RetValue;

		m_Del3D->Triangulate( a_Points );

		for (auto it = m_Del3D->GetAllCellsIteratorBegin( ); it != m_Del3D->GetAllCellsIteratorEnd( ); ++it)
		{
			glm::vec3 l_Points[ 4 ];
			int l_Count = 0;
			for (int i = 0; i < 4; ++i)
			{
				glm::vec3 l_Point( it->vertex( i )->point( ).x( ), it->vertex( i )->point( ).y( ), it->vertex( i )->point( ).z( ) );
				if (std::find( a_Points.begin( ), a_Points.end( ), l_Point ) != a_Points.end( ))
				{
					l_Points[ l_Count ] = l_Point;
					l_Count++;
				}
			}

			if (l_Count == 3)
			{
				SetupTriangle( l_Points[ 0 ], l_Points[ 1 ], l_Points[ 2 ] );
			}
		}
	}

	clsTriangle* clsBNPObject::SetupTriangle( glm::vec3 a_v1, glm::vec3 a_v2, glm::vec3 a_v3, glm::vec3 a_n )
	{
		SetupWindingOrder( a_v1, a_v2, a_v3 );
		OpenGLLib::Skeleton::clsVertex l_Vertex1( a_v1.x, a_v1.y, a_v1.z );
		OpenGLLib::Skeleton::clsVertex l_Vertex2( a_v2.x, a_v2.y, a_v2.z );
		OpenGLLib::Skeleton::clsVertex l_Vertex3( a_v3.x, a_v3.y, a_v3.z );
		OpenGLLib::Skeleton::clsVertex* l_Vertex1P = FindVertexIfExists( l_Vertex1 );
		OpenGLLib::Skeleton::clsVertex* l_Vertex2P = FindVertexIfExists( l_Vertex2 );
		OpenGLLib::Skeleton::clsVertex* l_Vertex3P = FindVertexIfExists( l_Vertex3 );
		clsEdge* l_Edge1P = FindEdgeIfExists( clsEdge( l_Vertex1P, l_Vertex2P ) );
		clsEdge* l_Edge2P = FindEdgeIfExists( clsEdge( l_Vertex2P, l_Vertex3P ) );
		clsEdge* l_Edge3P = FindEdgeIfExists( clsEdge( l_Vertex3P, l_Vertex1P ) );

		clsTriangle l_Triangle( l_Vertex1P, l_Vertex2P, l_Vertex3P, l_Edge1P, l_Edge2P, l_Edge3P );
		l_Triangle.SetBNPObject( this );

		glm::vec3 l_Normal;
		if (a_n == glm::vec3( 0, 0, 0 ))
		{
			l_Normal = glm::normalize( glm::cross( a_v2 - a_v1, a_v3 - a_v2 ) );
		}
		else
		{
			l_Normal = a_n;
		}
		l_Triangle.SetNormal( l_Normal );
		clsTriangle* l_TriangleP = AddTriangle( l_Triangle );
		l_TriangleP->PointEdgesToTriangle( );

		return l_TriangleP;
	}

	void clsBNPObject::SetupWindingOrder( glm::vec3& r_v1, glm::vec3& r_v2, glm::vec3& r_v3 )
	{
		glm::vec3 l_CurrentNormal = glm::normalize( glm::cross( r_v2 - r_v1, r_v3 - r_v2 ) );
		glm::vec3 l_ReversedNormal = glm::normalize( glm::cross( r_v2 - r_v3, r_v1 - r_v2 ) );
		glm::vec3 l_TriangleCenter = glm::vec3( ( r_v1.x + r_v2.x + r_v3.x ) / 3.0f, ( r_v1.y + r_v2.y + r_v3.y ) / 3.0f, ( r_v1.z + r_v2.z + r_v3.z ) / 3.0f );
		glm::vec3 l_PathVerticesCenter = GetPathVerticesCenter( );
		glm::vec3 l_NormalToSphereCenter = glm::normalize( l_PathVerticesCenter - l_TriangleCenter );
		float l_DistToCurrentNormal = glm::distance( l_CurrentNormal, l_NormalToSphereCenter );
		float l_DistToReversedNormal = glm::distance( l_ReversedNormal, l_NormalToSphereCenter );
		if (l_DistToCurrentNormal < l_DistToReversedNormal)
		{
			glm::vec3 l_Temp = r_v1;
			r_v1 = r_v3;
			r_v3 = l_Temp;
		}
	}

	void clsBNPObject::SetupPathVertices( OpenGLLib::Skeleton::clsJoint* a_Joint )
	{
		if (a_Joint->m_ConnectedBone != nullptr)
		{
			glm::vec3 l_Normal = glm::normalize( a_Joint->m_ConnectedBone->GetStartPosition( ) - a_Joint->m_Position );
			glm::vec3 l_PathVertCoord = a_Joint->m_Position + glm::normalize( a_Joint->m_ConnectedBone->GetStartPosition( ) - a_Joint->m_Position ) * a_Joint->m_Thickness;
			clsPathVertex l_PathVertex;
			l_PathVertex.SetPosition( l_PathVertCoord );
			l_PathVertex.SetNormal( l_Normal );
			l_PathVertex.SetBNPPointer( this );
			clsPathVertex* l_PathVertexP = m_PathVerticesLinkedList.Add( l_PathVertex );
			m_PathVertices.push_back( l_PathVertexP );
		}
		else
		{
			m_IsRoot = true;
		}

		for (unsigned int i = 0; i < a_Joint->m_Bones.size( ); ++i)
		{
			glm::vec3 l_Normal = glm::normalize( a_Joint->m_Bones[ i ]->GetEndPosition( ) - a_Joint->m_Position );
			glm::vec3 l_PathVertCoord = a_Joint->m_Position + l_Normal * a_Joint->m_Thickness;
			clsPathVertex l_PathVertex;
			l_PathVertex.SetPosition( l_PathVertCoord );
			l_PathVertex.SetNormal( l_Normal );
			l_PathVertex.SetBNPPointer( this );
			clsPathVertex* l_PathVertexP = m_PathVerticesLinkedList.Add( l_PathVertex );
			m_PathVertices.push_back( l_PathVertexP );
		}
	}

	void clsBNPObject::SubdivideBNP( )
	{
		std::vector<clsTriangle> l_SubdividedTriangle;
		std::vector<clsEdge*> l_OldEdges;
		for (unsigned int i = 0; i < m_Triangles.size( ); i += m_SubdivideAmount)
		{
			l_OldEdges.push_back( m_Triangles[ i ]->GetEdges( )->at( 0 ) );
			l_OldEdges.push_back( m_Triangles[ i ]->GetEdges( )->at( 1 ) );
			l_OldEdges.push_back( m_Triangles[ i ]->GetEdges( )->at( 2 ) );

			SubdivideTriangle( m_Triangles[ i ], i );
		}

		// Remove edges from initial triangles
		RemoveEdges( l_OldEdges );
	}

	void clsBNPObject::Project( glm::vec3 a_JointPosition, float a_JointRadius, std::vector<glm::vec3> a_PathVertices )
	{
		if (!ArePoints3D( a_PathVertices ))
		{
			MakeFlatBNP3D( a_JointPosition, a_JointRadius );
		}
		else
		{
			SetVertexNormals( );
			ProjectToSphereWithVertexNormals( a_JointPosition, a_JointRadius );
		}
	}

	void clsBNPObject::SetupLIE( float a_JointRadius )
	{
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			SetLIE( m_PathVertices[ i ] );
			m_PathVertices[ i ]->GetLIE( )->SortAsChain( );
		}

		SetLIESegments( );
		SmoothLIESegments( a_JointRadius );
	}

	void clsBNPObject::RemoveEdges( std::vector<clsEdge*> a_Edges )
	{
		for (size_t i = 0; i < a_Edges.size( ); ++i)
		{
			if (std::find( m_Edges.begin( ), m_Edges.end( ), a_Edges[ i ] ) != m_Edges.end( ))
			{
				m_Edges.erase( std::find( m_Edges.begin( ), m_Edges.end( ), a_Edges[ i ] ) );
			}
		}
	}

	void clsBNPObject::SubdivideTriangle( clsTriangle* a_Triangle, int a_Index )
	{
		unsigned int l_TrianglePoints = 3;
		glm::vec3 l_v1 = a_Triangle->GetVertices( )->at( 0 )->GetPosition( );
		glm::vec3 l_v2 = a_Triangle->GetVertices( )->at( 1 )->GetPosition( );
		glm::vec3 l_v3 = a_Triangle->GetVertices( )->at( 2 )->GetPosition( );
		glm::vec3 l_e1m;
		glm::vec3 l_e2m;
		glm::vec3 l_e3m;
		glm::vec3 l_c;

		NewSubdividedTriangleVertices( l_v1, l_v2, l_v3, l_e1m, l_e2m, l_e3m, l_c );

		glm::vec3 l_n = a_Triangle->GetNormal( );
		SetupTriangle( l_v1, l_e1m, l_c, l_n );
		SetupTriangle( l_e1m, l_v2, l_c, l_n );
		SetupTriangle( l_c, l_v2, l_e2m, l_n );
		SetupTriangle( l_c, l_e2m, l_v3, l_n );
		SetupTriangle( l_e3m, l_c, l_v3, l_n );
		SetupTriangle( l_v1, l_c, l_e3m, l_n );


		ReplaceSubdividedTrianglesInVectorTo( a_Index );
	}

	void clsBNPObject::NewSubdividedTriangleVertices( glm::vec3 a_v1, glm::vec3 a_v2, glm::vec3 a_v3, glm::vec3& r_e1m, glm::vec3& r_e2m, glm::vec3& r_e3m, glm::vec3& r_c )
	{
		float l_MidPointX = ( a_v2.x + a_v1.x ) / 2.0f;
		float l_MidPointY = ( a_v2.y + a_v1.y ) / 2.0f;
		float l_MidPointZ = ( a_v2.z + a_v1.z ) / 2.0f;
		r_e1m = glm::vec3( l_MidPointX, l_MidPointY, l_MidPointZ );

		l_MidPointX = ( a_v3.x + a_v2.x ) / 2.0f;
		l_MidPointY = ( a_v3.y + a_v2.y ) / 2.0f;
		l_MidPointZ = ( a_v3.z + a_v2.z ) / 2.0f;
		r_e2m = glm::vec3( l_MidPointX, l_MidPointY, l_MidPointZ );

		l_MidPointX = ( a_v1.x + a_v3.x ) / 2.0f;
		l_MidPointY = ( a_v1.y + a_v3.y ) / 2.0f;
		l_MidPointZ = ( a_v1.z + a_v3.z ) / 2.0f;
		r_e3m = glm::vec3( l_MidPointX, l_MidPointY, l_MidPointZ );

		float l_CenterX = ( a_v1.x + a_v2.x + a_v3.x ) / 3.0f;
		float l_CenterY = ( a_v1.y + a_v2.y + a_v3.y ) / 3.0f;
		float l_CenterZ = ( a_v1.z + a_v2.z + a_v3.z ) / 3.0f;
		r_c = glm::vec3( l_CenterX, l_CenterY, l_CenterZ );
	}

	void clsBNPObject::SetVertexNormalsForFlatBNP( )
	{
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			glm::vec3 l_CurrVertPos = m_Vertices[ i ]->GetPosition( );
			if (IsVertexOnOuterEdgeOfFlatBNP( l_CurrVertPos ))
			{
				clsPathVertex* l_PathVertex = nullptr;
				if (!IsVertexAPathVertex( l_CurrVertPos, &l_PathVertex ))
				{
					std::vector<clsEdge*> l_CommonEdges = GetUniqueEdgesWithVertex( l_CurrVertPos );
					glm::vec3 l_VertexNormal = GetAverageNormalFromEdges( l_CommonEdges );
					m_Vertices[ i ]->SetNormal( l_VertexNormal );
				}
				else
				{
					m_Vertices[ i ]->SetNormal( l_PathVertex->GetNormal( ) );
				}
			}
			else
			{
				m_Vertices[ i ]->SetNormal( GetVertexNormalBasedOnSurroundingFaces( m_Vertices[ i ]->GetPosition( ) ) );
			}
		}
	}

	bool clsBNPObject::IsVertexOnOuterEdgeOfFlatBNP( glm::vec3 a_Vertex )
	{
		std::vector<glm::vec3> l_Normals;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetEdges( )->size( ); ++j)
			{
				glm::vec3 l_EV1Pos = m_Triangles[ i ]->GetEdges( )->at( j )->GetVertex1( )->GetPosition( );
				glm::vec3 l_EV2Pos = m_Triangles[ i ]->GetEdges( )->at( j )->GetVertex2( )->GetPosition( );
				if (l_EV1Pos == a_Vertex)
				{
					glm::vec3 l_Normal = glm::normalize( l_EV2Pos - l_EV1Pos );
					l_Normals.push_back( l_Normal );
				}
				else if (l_EV2Pos == a_Vertex)
				{
					glm::vec3 l_Normal = glm::normalize( l_EV1Pos - l_EV2Pos );
					l_Normals.push_back( l_Normal );
				}
			}
		}

		assert( l_Normals.size( ) >= 2 );
		float l_TotalAngle = 0.0f;
		for (size_t i = 0; i < l_Normals.size( ) - 1; ++i)
		{
			l_TotalAngle += glm::acos( glm::dot( l_Normals[ i ], l_Normals[ i + 1 ] ) );
		}

		if (l_TotalAngle >= glm::pi<float>( ) * 2.0f)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 clsBNPObject::GetAverageNormalFromEdges( std::vector<clsEdge*> a_Edges )
	{
		assert( a_Edges.size( ) >= 2 );
		glm::vec3 l_CommonVertex = GetCommonVertexFromEdges( a_Edges );
		glm::vec3 l_TotalNormals;
		for (size_t i = 0; i < a_Edges.size( ); ++i)
		{
			glm::vec3 l_EV1Pos = a_Edges[ i ]->GetVertex1( )->GetPosition( );
			glm::vec3 l_EV2Pos = a_Edges[ i ]->GetVertex2( )->GetPosition( );
			if (l_EV1Pos == l_CommonVertex)
			{
				l_TotalNormals += glm::normalize( l_CommonVertex - l_EV2Pos );
			}
			else if (l_EV2Pos == l_CommonVertex)
			{
				l_TotalNormals += glm::normalize( l_CommonVertex - l_EV1Pos );
			}
			else
			{
				assert( "Edges do not share the common vertex" == 0 );
			}
		}
		glm::vec3 l_AverageNormal = l_TotalNormals / (float) a_Edges.size( );
		return glm::normalize( l_AverageNormal );
	}

	glm::vec3 clsBNPObject::GetCommonVertexFromEdges( std::vector<clsEdge*> a_Edges )
	{
		assert( a_Edges.size( ) >= 2 );
		glm::vec3 l_CommonVertex;
		glm::vec3 l_E1V1Pos = a_Edges[ 0 ]->GetVertex1( )->GetPosition( );
		glm::vec3 l_E1V2Pos = a_Edges[ 0 ]->GetVertex2( )->GetPosition( );
		glm::vec3 l_E2V1Pos = a_Edges[ 1 ]->GetVertex1( )->GetPosition( );
		glm::vec3 l_E2V2Pos = a_Edges[ 1 ]->GetVertex2( )->GetPosition( );

		std::vector<glm::vec3> l_Vertices = { l_E1V1Pos, l_E1V2Pos, l_E2V1Pos, l_E2V2Pos };

		for (size_t i = 0; i < l_Vertices.size( ); ++i)
		{
			for (size_t j = i + 1; j < l_Vertices.size( ); ++j) // i + 1 because we don't want to compare the exact same edge vertex (j = i)
			{
				if (l_Vertices[ i ] == l_Vertices[ j ])
				{
					return l_Vertices[ i ];
				}
			}
		}
		assert( "Edges do not share a common vertex" == 0 );
		return glm::vec3( );
	}

	std::vector<clsEdge*> clsBNPObject::GetUniqueEdgesWithVertex( glm::vec3 a_Vertex )
	{
		std::vector<clsEdge*> l_Edges;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetEdges( )->size( ); ++j)
			{
				glm::vec3 l_EV1Pos = m_Triangles[ i ]->GetEdges( )->at( j )->GetVertex1( )->GetPosition( );
				glm::vec3 l_EV2Pos = m_Triangles[ i ]->GetEdges( )->at( j )->GetVertex2( )->GetPosition( );
				if (l_EV1Pos == a_Vertex || l_EV2Pos == a_Vertex)
				{
					bool l_IsEdgeUnique = true;
					for (size_t k = 0; k < l_Edges.size( ); ++k)
					{
						if (l_Edges[ k ] == m_Triangles[ i ]->GetEdges( )->at( j ))
						{
							l_IsEdgeUnique = false;
							break;
						}
					}
					if (l_IsEdgeUnique)
					{
						l_Edges.push_back( m_Triangles[ i ]->GetEdges( )->at( j ) );
					}
				}
			}
		}

		assert( l_Edges.size( ) >= 2 );
		return l_Edges;
	}

	void clsBNPObject::MakeFlatBNP3D( glm::vec3 a_SphereCenter, float a_SphereRadius )
	{
		glm::vec3 l_Normal = m_Triangles[ 0 ]->GetNormal( ); // Flat BNP triangles all have same normal
		SetVertexNormalsForFlatBNP( );
		std::vector<glm::vec3> l_Vertices = ProjectFlatBNPVerticesToSphere( l_Normal, a_SphereCenter, a_SphereRadius );

		// Retriangulate
		m_Triangles.clear( );
		m_Edges.clear( );
		m_Vertices.clear( );
		Triangulate3D( l_Vertices );
	}

	std::vector<glm::vec3> clsBNPObject::ProjectFlatBNPVerticesToSphere( glm::vec3 a_Normal, glm::vec3 a_SphereCenter, float a_SphereRadius )
	{
		std::vector<glm::vec3> l_Vertices;
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			glm::vec3 l_Vertex = m_Vertices[ i ]->GetPosition( );
			if (!IsVertexOnOuterEdgeOfFlatBNP( l_Vertex ))
			{
				l_Vertices.push_back( l_Vertex + a_Normal * a_SphereRadius );
				l_Vertices.push_back( l_Vertex - a_Normal * a_SphereRadius );
			}
			else
			{
				clsPathVertex* l_PathVertex = nullptr;
				if (IsVertexAPathVertex( l_Vertex, &l_PathVertex ))
				{
					l_Vertices.push_back( l_Vertex );
				}
				else
				{
					glm::vec3 l_Vertex = m_Vertices[ i ]->GetPosition( );
					glm::vec3 l_Normal = m_Vertices[ i ]->GetNormal( );
					if (l_Vertex == a_SphereCenter)
					{
						l_Vertex = l_Vertex + l_Normal * 1.01f;
					}
					glm::vec3 l_FromPosition = l_Vertex;
					glm::vec3 l_ToPosition = l_Vertex + l_Normal * a_SphereRadius * 10.0f;
					float l_DistanceNormalized = 0.0f;
					bool l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_SphereCenter, a_SphereRadius * a_SphereRadius, &l_DistanceNormalized );
					if (!l_Intersected)
					{
						l_ToPosition = l_Vertex + -l_Normal * a_SphereRadius * 10.0f;
						l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_SphereCenter, a_SphereRadius * a_SphereRadius, &l_DistanceNormalized );
					}

					if (l_Intersected)
					{
						float l_Distance = glm::distance( l_FromPosition, l_ToPosition ) * l_DistanceNormalized;
						l_Vertices.push_back( l_Vertex + l_Normal * l_Distance );
					}
				}
			}
		}
		return l_Vertices;
	}

	void clsBNPObject::ProjectToSphereWithVertexNormals( glm::vec3 a_SphereCenter, float a_SphereRadius )
	{
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			glm::vec3 l_Vertex = m_Vertices[ i ]->GetPosition( );
			glm::vec3 l_Normal = m_Vertices[ i ]->GetNormal( );
			if (l_Vertex == a_SphereCenter)
			{
				l_Vertex = l_Vertex + l_Normal * 1.01f;
			}
			glm::vec3 l_FromPosition = l_Vertex;
			glm::vec3 l_ToPosition = l_Vertex + l_Normal * a_SphereRadius * 10.0f;
			float l_DistanceNormalized = 0.0f;
			bool l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_SphereCenter, a_SphereRadius * a_SphereRadius, &l_DistanceNormalized );
			if (!l_Intersected)
			{
				l_ToPosition = l_Vertex + -l_Normal * a_SphereRadius * 10.0f;
				l_Intersected = BrawlerEngineLib::clsMath::This( ).RaySphereIntersection( l_FromPosition, l_ToPosition - l_FromPosition, a_SphereCenter, a_SphereRadius * a_SphereRadius, &l_DistanceNormalized );
			}

			if (l_Intersected)
			{
				clsPathVertex* l_PathVertex = nullptr;
				if (!IsVertexAPathVertex( l_Vertex, &l_PathVertex ))
				{
					float l_Distance = glm::distance( l_FromPosition, l_ToPosition ) * l_DistanceNormalized;
					m_Vertices[ i ]->SetPosition( l_Vertex + l_Normal * l_Distance );
				}
			}
		}
	}

	void clsBNPObject::SetVertexNormals( )
	{
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetVertices( )->size( ); ++j)
			{
				glm::vec3 l_Vertex = m_Triangles[ i ]->GetVertices( )->at( (int) j )->GetPosition( );
				glm::vec3 l_Normal = GetVertexNormalBasedOnSurroundingFaces( l_Vertex );
				m_Triangles[ i ]->GetVertices( )->at( (int) j )->SetNormal( l_Normal, GetCenter( ) );
			}
		}
	}

	void clsBNPObject::SetFaceNormals( )
	{
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			m_Triangles[ i ]->SetNormal( );
		}
	}

	std::vector<glm::vec3> clsBNPObject::GetEdgesFromConnectedVertexAsNormals( glm::vec3 a_Vertex )
	{
		std::vector<glm::vec3> l_Normals;
		std::vector<clsEdge> l_Edges;
		for (unsigned int i = 0; i < m_Triangles.size( ); ++i)
		{
			clsTriangle l_t = *m_Triangles[ i ];
			for (int j = 0; j < l_t.GetEdges( )->size( ); ++j)
			{
				clsEdge* l_Edge = l_t.GetEdges( )->at( j );
				if (a_Vertex == l_Edge->GetVertex1( )->GetPosition( ) || a_Vertex == l_Edge->GetVertex2( )->GetPosition( ))
				{
					bool l_IsEdgeUnique = true;
					for (size_t k = 0; k < l_Edges.size( ); ++k)
					{
						if (l_Edges[ k ] == *l_Edge)
						{
							l_IsEdgeUnique = false;
							break;
						}
					}
					if (l_IsEdgeUnique)
					{
						l_Edges.push_back( *l_Edge );

						glm::vec3 l_Normal;
						if (l_Edge->GetVertex1( )->GetPosition( ) == a_Vertex)
						{
							l_Normal = glm::normalize( l_Edge->GetVertex2( )->GetPosition( ) - a_Vertex );
						}
						else if (l_Edge->GetVertex2( )->GetPosition( ) == a_Vertex)
						{
							l_Normal = glm::normalize( l_Edge->GetVertex1( )->GetPosition( ) - a_Vertex );
						}
						else
						{
							assert( "Vertex is not connected to edge." == 0 );
						}
						l_Normals.push_back( l_Normal );
					}
				}
			}
		}
		return l_Normals;
	}

	bool clsBNPObject::IsVertexAPathVertex( glm::vec3 a_Vertex, clsPathVertex** r_PathVertex )
	{
		bool l_IsPathVertex = false;
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			if (a_Vertex == m_PathVertices[ i ]->GetPosition( ))
			{
				l_IsPathVertex = true;
				*r_PathVertex = m_PathVertices[ i ];
				break;
			}
		}
		return l_IsPathVertex;
	}

	glm::vec3 clsBNPObject::GetVertexNormalBasedOnSurroundingFaces( glm::vec3 a_Vertex )
	{
		std::vector<clsTriangle*> l_TrianglesAroundVertex = GetTrianglesAroundVertex( a_Vertex );
		glm::vec3 l_NewFaceNormal;

		for (size_t i = 0; i < l_TrianglesAroundVertex.size( ); ++i)
		{
			clsTriangle* l_t = l_TrianglesAroundVertex[ i ];
			glm::vec3 l_FaceNormal = l_t->GetNormal( );
			l_NewFaceNormal += l_FaceNormal;
		}
		glm::vec3 l_VertexNormal = glm::normalize( l_NewFaceNormal );

		return l_VertexNormal;
	}

	std::vector<clsTriangle*> clsBNPObject::GetTrianglesAroundVertex( glm::vec3 a_Vertex )
	{
		std::vector<clsTriangle*> l_TrianglesAroundVertex;
		for (unsigned int i = 0; i < m_Triangles.size( ); ++i)
		{
			clsTriangle* l_t = m_Triangles[ i ];
			for (int j = 0; j < l_t->GetVertices( )->size( ); ++j)
			{
				if (a_Vertex == l_t->GetVertices( )->at( j )->GetPosition( ))
				{
					l_TrianglesAroundVertex.push_back( l_t );
				}
			}
		}
		assert( l_TrianglesAroundVertex.size( ) > 2 );

		return l_TrianglesAroundVertex;
	}

	void clsBNPObject::SetLIE( clsPathVertex* r_PathVertex )
	{
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			glm::vec3 l_Vert = m_Vertices[ i ]->GetPosition( );
			glm::vec3 l_VertRounded = glm::round( l_Vert * 100.0f ) / 100.0f;
			glm::vec3 l_PathVertRounded = glm::round( r_PathVertex->GetPosition( ) * 100.0f ) / 100.0f;
			if (l_VertRounded == l_PathVertRounded)
			{
				clsLIE* l_LIE = m_LIEsLinkedList.Add( clsLIE( ) );
				m_LIEs.push_back( l_LIE );
				std::vector<clsTriangle*> l_Triangles = GetTrianglesAroundVertex( r_PathVertex->GetPosition( ) );
				std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices = l_Triangles[ 0 ]->GetVerticesOppositeToVertexInWindingOrder( r_PathVertex->GetPosition( ) );
				l_LIE->SetVertexOrder( l_Vertices );
				l_LIE->SetBNPObject( this );
				l_LIE->SetPathVertex( r_PathVertex );
				for (unsigned int k = 0; k < l_Triangles.size( ); ++k)
				{
					clsEdge* l_Edge = l_Triangles[ k ]->GetEdgePointerOppositeToVertex( r_PathVertex->GetPosition( ) );
					l_LIE->AddEdge( l_Edge );
				}
				assert( l_LIE->GetEdges( )->size( ) > 0 );
				r_PathVertex->SetLIE( l_LIE );
			}
		}
	}

	void clsBNPObject::SetLIESegments( )
	{
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			clsPathVertex* l_CurrentPathVertex = m_PathVertices[ i ];
			clsLIE* l_LIE = l_CurrentPathVertex->GetLIE( );
			clsPathVertex* l_OppositePathVertex = l_LIE->GetEdges( )->at( 0 )->GetLIEOppositeOf( l_LIE )->GetPathVertex( );
			clsPathVertex* l_NewOppositePathVertex = nullptr;
			clsLIESegment* l_LIESegmentP = nullptr;

			// Go through each edge in LIE and set as segment accordingly
			for (size_t j = 0; j < l_LIE->GetEdges( )->size( ); ++j)
			{
				l_NewOppositePathVertex = l_LIE->GetEdges( )->at( j )->GetLIEOppositeOf( l_LIE )->GetPathVertex( );

				// Finds either completed or no LIE segment
				clsLIESegment* l_FoundLIESegment = FindLIESegmentWithPathVertex( l_CurrentPathVertex, l_NewOppositePathVertex );
				if (l_FoundLIESegment != nullptr)
				{
					l_LIESegmentP = l_FoundLIESegment;
				}
				else
				{
					l_LIESegmentP = AddNewLIESegment( l_CurrentPathVertex, l_NewOppositePathVertex );
				}

				if (!l_LIESegmentP->IsCompleted( ))
				{


					l_LIESegmentP->AddEdge( l_LIE->GetEdges( )->at( j ) );
					if (!l_LIE->DoesLIESegmentExist( l_LIESegmentP ))
					{
						l_LIE->AddLIESegment( l_LIESegmentP );
					}
				}
				l_OppositePathVertex = l_LIE->GetEdges( )->at( j )->GetLIEOppositeOf( l_LIE )->GetPathVertex( );
			}

			// We are done with current LIE segments by this moment
			for (size_t j = 0; j < m_LIESegments.size( ); ++j)
			{
				m_LIESegments[ j ]->SetIsCompleted( true );
			}
		}
	}

	clsLIESegment* clsBNPObject::AddNewLIESegment( clsPathVertex* a_PathVertex1, clsPathVertex* a_PathVertex2 )
	{
		clsLIESegment* l_LIESegmentP = m_LIESegmentsLinkedList.Add( clsLIESegment( ) );
		m_LIESegments.push_back( l_LIESegmentP );
		l_LIESegmentP->SetPathVertices( a_PathVertex1, a_PathVertex2 );
		return l_LIESegmentP;
	}

	clsLIESegment* clsBNPObject::FindLIESegmentWithPathVertex( clsPathVertex* a_PathVertex1, clsPathVertex* a_PathVertex2 )
	{
		for (size_t i = 0; i < m_LIESegments.size( ); ++i)
		{
			if (m_LIESegments[ i ]->IsPathVertexConnectedToLIESegment( a_PathVertex1, a_PathVertex2 ))
			{
				return m_LIESegments[ i ];
			}
		}
		return nullptr;
	}

	void clsBNPObject::SmoothLIESegments( float a_JointRadius )
	{
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			for (size_t j = 0; j < m_PathVertices[ i ]->GetLIE( )->GetLIESegments( ).size( ); ++j)
			{
				clsLIESegment* l_LIESegment = m_PathVertices[ i ]->GetLIE( )->GetLIESegments( )[ j ];
				glm::vec3 l_PathVerticesCenter = GetPathVerticesCenter( );
				l_LIESegment->SortAsChain( l_PathVerticesCenter );
				l_LIESegment->Smooth2( l_PathVerticesCenter, a_JointRadius, *GetJointPos( ), this ); // not sure if works properly
			}
		}
	}

	void clsBNPObject::Split( clsLIESegment* a_Segment )
	{
		clsEdge* l_OldEdge = a_Segment->GetEdgeToSplit( );
		std::vector<clsTriangle*> l_OldTriangles = GetTrianglesWithEdge( l_OldEdge );

		glm::vec3 l_OldEdgeNormal = glm::normalize( l_OldEdge->GetVertex2( )->GetPosition( ) - l_OldEdge->GetVertex1( )->GetPosition( ) );
		float l_OldEdgeLength = glm::distance( l_OldEdge->GetVertex1( )->GetPosition( ), l_OldEdge->GetVertex2( )->GetPosition( ) );
		glm::vec3 l_NewVertex = l_OldEdge->GetVertex1( )->GetPosition( ) + l_OldEdgeNormal * l_OldEdgeLength * 0.5f;

		glm::vec3 l_OldTriangle1OppositeVertex = l_OldTriangles[ 0 ]->GetVertexOppositeToEdge( l_OldEdge )->GetPosition( );
		glm::vec3 l_OldTriangle2OppositeVertex = l_OldTriangles[ 1 ]->GetVertexOppositeToEdge( l_OldEdge )->GetPosition( );

		glm::vec3 l_t1v1 = l_OldEdge->GetVertex1( )->GetPosition( );
		glm::vec3 l_t1v2 = l_OldTriangle1OppositeVertex;
		glm::vec3 l_t1v3 = l_NewVertex;

		glm::vec3 l_t2v1 = l_OldEdge->GetVertex2( )->GetPosition( );
		glm::vec3 l_t2v2 = l_OldTriangle1OppositeVertex;
		glm::vec3 l_t2v3 = l_NewVertex;

		glm::vec3 l_t3v1 = l_OldEdge->GetVertex1( )->GetPosition( );
		glm::vec3 l_t3v2 = l_OldTriangle2OppositeVertex;
		glm::vec3 l_t3v3 = l_NewVertex;

		glm::vec3 l_t4v1 = l_OldEdge->GetVertex2( )->GetPosition( );
		glm::vec3 l_t4v2 = l_OldTriangle2OppositeVertex;
		glm::vec3 l_t4v3 = l_NewVertex;

		std::vector<clsLIE*> l_LIEs = GetLIEsWithEdge( l_OldEdge );
		RemoveOldLIEEdge_Split( l_LIEs, a_Segment, l_OldEdge );

		RemoveTriangle( l_OldTriangles[ 0 ] );
		RemoveTriangle( l_OldTriangles[ 1 ] );
		RemoveEdge( l_OldEdge );

		SetupTriangle( l_t1v1, l_t1v2, l_t1v3 );
		SetupTriangle( l_t2v1, l_t2v2, l_t2v3 );
		SetupTriangle( l_t3v1, l_t3v2, l_t3v3 );
		SetupTriangle( l_t4v1, l_t4v2, l_t4v3 );

		std::vector<clsEdge*> l_NewEdges = GetNewEdgesWithValues( l_t1v1, l_NewVertex, l_t2v1 );

		AddNewLIEEdges_Split( l_LIEs, a_Segment, l_NewEdges );

		if (l_LIEs[ 0 ]->m_RequiredAmountOfSplits > 0)
		{
			--l_LIEs[ 0 ]->m_RequiredAmountOfSplits;
		}
		if (l_LIEs[ 1 ]->m_RequiredAmountOfSplits > 0)
		{
			--l_LIEs[ 1 ]->m_RequiredAmountOfSplits;
		}
		++a_Segment->m_DoneAmountOfSplits;
	}

	void clsBNPObject::RemoveOldLIEEdge_Split( std::vector<clsLIE*> a_LIEs, clsLIESegment* a_Segment, clsEdge* a_OldEdge )
	{
		a_LIEs[ 0 ]->RemoveEdge( a_OldEdge );
		a_LIEs[ 1 ]->RemoveEdge( a_OldEdge );
		a_Segment->RemoveEdge( a_OldEdge );
	}

	void clsBNPObject::AddNewLIEEdges_Split( std::vector<clsLIE*> a_LIEs, clsLIESegment* a_Segment, std::vector<clsEdge*> a_NewEdges )
	{
		a_LIEs[ 0 ]->AddEdge( a_NewEdges[ 0 ] );
		a_LIEs[ 0 ]->AddEdge( a_NewEdges[ 1 ] );
		a_LIEs[ 1 ]->AddEdge( a_NewEdges[ 0 ] );
		a_LIEs[ 1 ]->AddEdge( a_NewEdges[ 1 ] );
		a_Segment->AddEdge( a_NewEdges[ 0 ] );
		a_Segment->AddEdge( a_NewEdges[ 1 ] );

		std::vector<clsTriangle*> l_Triangles = GetTrianglesAroundVertex( a_LIEs[ 0 ]->GetPathVertex( )->GetPosition( ) );
		std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices = l_Triangles[ 0 ]->GetVerticesOppositeToVertexInWindingOrder( a_LIEs[ 0 ]->GetPathVertex( )->GetPosition( ) );
		a_LIEs[ 0 ]->SetVertexOrder( l_Vertices );
		a_LIEs[ 0 ]->SortAsChain( );
		l_Triangles = GetTrianglesAroundVertex( a_LIEs[ 1 ]->GetPathVertex( )->GetPosition( ) );
		l_Vertices = l_Triangles[ 0 ]->GetVerticesOppositeToVertexInWindingOrder( a_LIEs[ 1 ]->GetPathVertex( )->GetPosition( ) );
		a_LIEs[ 1 ]->SetVertexOrder( l_Vertices );
		a_LIEs[ 1 ]->SortAsChain( );
		glm::vec3 l_PathVerticesCenter = GetPathVerticesCenter( );
		a_Segment->SortAsChain( l_PathVerticesCenter );

		a_Segment->Smooth2( GetPathVerticesCenter( ), m_JointThickness, *m_JointPos, this );
	}

	void clsBNPObject::LinkLIEs( )
	{
		size_t l_Index = 0;
		if (!m_IsRoot)
		{
			++l_Index;
		}

		for (size_t i = l_Index; i < m_PathVertices.size( ); ++i)
		{
			if (m_PathVertices[ i ]->GetConnectedPathVertex( ) != nullptr)
			{
				clsLIE* l_LIE1 = m_PathVertices[ i ]->GetLIE( );
				clsLIE* l_LIE2 = m_PathVertices[ i ]->GetConnectedPathVertex( )->GetLIE( );
				l_LIE1->Reverse( );
				int l_LIEIndex = CompareLIEsByDistance( l_LIE1, l_LIE2 );
				ReorderLIE( l_LIE1, l_LIEIndex );
				LinkLIEVertices( l_LIE1, l_LIE2 );
			}
		}
	}

	int clsBNPObject::CompareLIEsByDistance( clsLIE* a_LIE1, clsLIE* a_LIE2 )
	{
		float minDist = FLT_MAX;
		int index = 0;

		for (int i = 0; i <= a_LIE1->GetEdges( )->size( ); ++i)
		{
			int k = i;
			float dist = 0;
			for (int j = 0; j < a_LIE2->GetEdges( )->size( ); ++j)
			{
				if (k >= a_LIE1->GetEdges( )->size( ))
				{
					k = 0;
				}

				glm::vec3 A = a_LIE2->GetVertexAt( j )->GetPosition( );
				glm::vec3 B = a_LIE1->GetVertexAt( k )->GetPosition( );
				dist += glm::distance( A, B );
				++k;
			}

			if (dist < minDist)
			{
				minDist = dist;
				index = i;
			}
		}

		return index;
	}

	void clsBNPObject::ReorderLIE( clsLIE* a_LIE, int a_Index )
	{
		std::vector<clsEdge*> l_Edges;
		for (int i = 0; i < a_LIE->GetEdges( )->size( ); ++i)
		{
			if (a_Index >= a_LIE->GetEdges( )->size( ))
			{
				a_Index = 0;
			}
			l_Edges.push_back( a_LIE->GetEdges( )->at( a_Index ) );
			++a_Index;
		}
		a_LIE->SetEdges( l_Edges );
		a_LIE->SortVertices( );
	}

	void clsBNPObject::LinkLIEVertices( clsLIE* a_LIE1, clsLIE* a_LIE2 )
	{
		assert( a_LIE1->GetEdges( )->size( ) == a_LIE2->GetEdges( )->size( ) );
		size_t l_Size = a_LIE1->GetEdges( )->size( );

		OpenGLLib::Skeleton::clsVertex* l_L1cv = nullptr;
		OpenGLLib::Skeleton::clsVertex* l_L2cv = nullptr;
		for (size_t i = 0; i < l_Size; ++i)
		{
			l_L1cv = a_LIE1->GetVertexAt( i );
			l_L2cv = a_LIE2->GetVertexAt( i );

			// temp
			//SQMDebugData.AddLine( l_L1cv->GetPosition( ), l_L2cv->GetPosition( ), glm::vec3( 0, 0, 0 ) );

			l_L1cv->AddConnectedVertex( l_L2cv );
			l_L2cv->AddConnectedVertex( l_L1cv );
		}

	}

	clsTriangle* clsBNPObject::AddTriangle( clsTriangle a_Triangle )
	{
		clsTriangle* l_Triangle = m_TrianglesLinkedList.Add( a_Triangle );
		l_Triangle->SetBNPObject( this );
		m_Triangles.push_back( l_Triangle );
		return l_Triangle;
	}

	void clsBNPObject::ReplaceSubdividedTrianglesInVectorTo( int a_Index )
	{
		std::vector<clsTriangle*> l_TrianglesToMove;
		l_TrianglesToMove.insert( l_TrianglesToMove.begin( ), m_Triangles.end( ) - m_SubdivideAmount, m_Triangles.end( ) );
		m_Triangles.erase( m_Triangles.end( ) - m_SubdivideAmount, m_Triangles.end( ) );
		m_Triangles.erase( m_Triangles.begin( ) + a_Index );
		m_Triangles.insert( m_Triangles.begin( ) + a_Index, l_TrianglesToMove.begin( ), l_TrianglesToMove.end( ) );
	}

	void clsBNPObject::RemoveEdge( clsEdge* a_Edge )
	{
		if (std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) == m_Edges.end( ))
		{
			assert( "m_Edges does not contain input." == 0 );
		}
		m_Edges.erase( std::find( m_Edges.begin( ), m_Edges.end( ), a_Edge ) );
	}

	void clsBNPObject::RemoveTriangle( clsTriangle* a_Triangle )
	{
		// set to nullptr before removing
		for (size_t i = 0; i < a_Triangle->GetEdges( )->size( ); ++i)
		{
			clsEdge* l_Edge = a_Triangle->GetEdges( )->at( i );
			if (l_Edge->GetTriangle1( ) == a_Triangle)
			{
				l_Edge->SetTriangle1( nullptr );
			}
			else if (l_Edge->GetTriangle2( ) == a_Triangle)
			{
				l_Edge->SetTriangle2( nullptr );
			}
		}

		if (std::find( m_Triangles.begin( ), m_Triangles.end( ), a_Triangle ) == m_Triangles.end( ))
		{
			assert( "m_Triangles does not contain input." == 0 );
		}
		m_Triangles.erase( std::find( m_Triangles.begin( ), m_Triangles.end( ), a_Triangle ) );
	}

	void clsBNPObject::ConvertTrianglesToDelaunay( glm::vec3 a_JointPos )
	{
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			m_Delaunay->SetupTriangle( m_Triangles[ i ]->GetVertices( )->at( 0 )->GetPosition( ), m_Triangles[ i ]->GetVertices( )->at( 1 )->GetPosition( ), m_Triangles[ i ]->GetVertices( )->at( 2 )->GetPosition( ) );
		}
		m_Delaunay->DelaunayOnSphereSurface_FlipEdges( m_Delaunay->GetTrianglePointers( ), a_JointPos );
	}

	void clsBNPObject::ConnectCrossingPathVertices( std::vector<clsPathVertex*> a_PathVertices )
	{
		clsPathVertex* l_ClosestThisPathVertex = 0;
		clsPathVertex* l_ClosestOtherPathVertex = 0;
		float l_ClosestDist = FLT_MAX;
		for (unsigned int i = 0; i < m_PathVertices.size( ); ++i)
		{
			for (unsigned int j = 0; j < a_PathVertices.size( ); ++j)
			{
				if (glm::distance( m_PathVertices[ i ]->GetPosition( ), a_PathVertices[ j ]->GetPosition( ) ) < l_ClosestDist)
				{
					l_ClosestDist = glm::distance( m_PathVertices[ i ]->GetPosition( ), a_PathVertices[ j ]->GetPosition( ) );
					l_ClosestThisPathVertex = m_PathVertices[ i ];
					l_ClosestOtherPathVertex = a_PathVertices[ j ];
				}
			}
		}
		if (l_ClosestThisPathVertex != 0 && l_ClosestOtherPathVertex != 0)
		{
			l_ClosestThisPathVertex->SetConnectedPathVertex( l_ClosestOtherPathVertex );
			l_ClosestOtherPathVertex->SetConnectedPathVertex( l_ClosestThisPathVertex );
		}
		else
		{
			assert( "No closest path vertices were found" == 0 );
		}
	}

	// Used in kinematics for positioning BNP
	void clsBNPObject::SetLocalVertexPositions( std::vector<glm::mat4> a_RotationMatrix, std::vector<glm::vec3> a_NewPos )
	{
		glm::vec3 l_JointPos = m_StraightSkeletonJointPos;
		glm::vec3 l_FirstPos = m_StraightSkeletonJointPos;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetVertices( )->size( ); ++j)
			{
				assert( a_RotationMatrix.size( ) == a_NewPos.size( ) );
				OpenGLLib::Skeleton::clsVertex* l_Vertex = m_Triangles[ i ]->GetVertices( )->at( (int) j );
				for (size_t k = 0; k < a_RotationMatrix.size( ); ++k)
				{
					l_Vertex->SetLocalPosition( l_Vertex->GetPosition( ) - l_JointPos );
					glm::vec4 l_NewLocalPos = a_RotationMatrix[ k ] * glm::vec4( l_Vertex->GetLocalPosition( ), 1 );
					l_Vertex->SetPosition( a_NewPos[ k ] + glm::vec3( l_NewLocalPos ) );
					l_JointPos = a_NewPos[ k ];
				}
				l_JointPos = l_FirstPos;
			}
		}
	}

	void clsBNPObject::SetRotation( glm::mat3 a_Rotation )
	{
		m_RotationMatrix *= a_Rotation;
	}

	void clsBNPObject::ResetRotation( )
	{
		m_RotationMatrix = { 1,0,0,0,1,0,0,0,1 };
	}

	void clsBNPObject::GenerateBNPPolygons( OpenGLLib::clsRenderPass* a_RenderPass )
	{
		m_RenderPass = a_RenderPass;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			glm::vec3 l_v1 = m_Triangles[ i ]->GetVertices( )->at( 0 )->GetPosition( );
			glm::vec3 l_v2 = m_Triangles[ i ]->GetVertices( )->at( 1 )->GetPosition( );
			glm::vec3 l_v3 = m_Triangles[ i ]->GetVertices( )->at( 2 )->GetPosition( );

			glm::vec2 l_uv1 = glm::vec2( 0, 0 );
			glm::vec2 l_uv2 = glm::vec2( 0, 1 );
			glm::vec2 l_uv3 = glm::vec2( 1, 1 );

			m_RenderPass->AddUV( l_uv1 );
			m_RenderPass->AddUV( l_uv2 );
			m_RenderPass->AddUV( l_uv3 );

			glm::vec3 l_Normal = glm::normalize( glm::cross( l_v2 - l_v1, l_v3 - l_v1 ) );
			for (size_t j = 0; j < m_Triangles[ i ]->GetVertices( )->size( ); ++j)
			{
				m_RenderPass->AddNormal( l_Normal );

#if SQM_DEBUGMODE
				int l_AmountOfVertices = (int) m_Triangles[ i ]->GetVertices( )->size( );
				glm::vec3 l_Pos = m_Triangles[ i ]->GetVertices( )->at( (int) j )->GetPosition( );
				glm::vec3 l_NextPos = m_Triangles[ i ]->GetVertices( )->at( (int) ( j + 1 ) % l_AmountOfVertices )->GetPosition( );
				//SQMDebugData.AddLine( l_Pos, l_Pos + l_Normal, glm::vec3( 0, 0, 0 ) );
				//SQMDebugData.AddPoint( l_Pos, glm::vec3( 0, 0, 0 ) );
				//SQMDebugData.AddLine( l_Pos, l_NextPos, glm::vec3( 0, 0, 0 ) );
#endif

				m_RenderPass->AddVertex( m_Triangles[ i ]->GetVertices( )->at( j )->GetPosition( ) );
			}
		}
	}

	OpenGLLib::Skeleton::clsVertex* clsBNPObject::FindVertexIfExists( OpenGLLib::Skeleton::clsVertex a_Vertex )
	{
		for (size_t i = 0; i < m_Vertices.size( ); ++i)
		{
			if (*m_Vertices[ i ] == a_Vertex)
			{
				return m_Vertices[ i ];
			}
		}
		OpenGLLib::Skeleton::clsVertex* l_VertexP = m_VerticesLinkedList.Add( a_Vertex );
		m_Vertices.push_back( l_VertexP );
		return l_VertexP;
	}

	clsEdge* clsBNPObject::FindEdgeIfExists( clsEdge a_Edge )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			if (*m_Edges[ i ] == a_Edge)
			{
				return m_Edges[ i ];
			}
		}
		clsEdge* l_EdgeP = m_EdgesLinkedList.Add( a_Edge );
		m_Edges.push_back( l_EdgeP );
		return l_EdgeP;
	}

	std::vector<clsTriangle*>* clsBNPObject::GetTriangles( )
	{
		return &m_Triangles;
	}

	std::vector<clsPathVertex*> clsBNPObject::GetPathVertices( )
	{
		return m_PathVertices;
	}

	glm::vec3 clsBNPObject::GetStraightSkeletonJointPos( )
	{
		return m_StraightSkeletonJointPos;
	}

	glm::vec3* clsBNPObject::GetJointPos( )
	{
		return m_JointPos;
	}

	float clsBNPObject::GetJointThickness( )
	{
		return m_JointThickness;
	}

	glm::mat3 clsBNPObject::GetRotation( )
	{
		return m_RotationMatrix;
	}

	glm::vec3 clsBNPObject::GetCenter( )
	{
		std::vector<glm::vec3> l_TriangleCenters;
		glm::vec3 l_TotalCenter;
		for (size_t i = 0; i < GetTriangles( )->size( ); ++i)
		{
			glm::vec3 l_TriangleCenter = GetTriangles( )->at( (int) i )->GetCenter( );
			l_TriangleCenters.push_back( l_TriangleCenter );
			l_TotalCenter += l_TriangleCenter;
		}


		glm::vec3 l_BNPAverageCenter = l_TotalCenter / (float) GetTriangles( )->size( );
		glm::vec3 l_BNPBBCenter = BrawlerEngineLib::clsMath::This( ).GetBoundingBoxCenter( l_TriangleCenters );
		return l_BNPAverageCenter;
	}

	glm::vec3 clsBNPObject::GetPathVerticesCenter( )
	{
		glm::vec3 l_Center;
		for (size_t i = 0; i < m_PathVertices.size( ); ++i)
		{
			l_Center += m_PathVertices[ i ]->GetPosition( );
		}

		return l_Center / (float) m_PathVertices.size( );
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*> clsBNPObject::GetVerticesWithPosition( glm::vec3 a_Position )
	{
		std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetVertices( )->size( ); ++j)
			{
				if (m_Triangles[ i ]->GetVertices( )->at( (int) j )->GetPosition( ) == a_Position)
				{
					l_Vertices.push_back( m_Triangles[ i ]->GetVertices( )->at( (int) j ) );
				}
			}
		}
		return l_Vertices;
	}

	std::vector<OpenGLLib::Skeleton::clsVertex*> clsBNPObject::GetVerticesConnectedToVertex( OpenGLLib::Skeleton::clsVertex* a_Vertex )
	{
		std::vector<OpenGLLib::Skeleton::clsVertex*> l_Vertices;
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			if (m_Edges[ i ]->GetVertex1( ) == a_Vertex)
			{
				l_Vertices.push_back( m_Edges[ i ]->GetVertex2( ) );
			}
			else if (m_Edges[ i ]->GetVertex2( ) == a_Vertex)
			{
				l_Vertices.push_back( m_Edges[ i ]->GetVertex1( ) );
			}
		}

		assert( l_Vertices.size( ) > 2 );
		return l_Vertices;
	}

	std::vector<clsTriangle*> clsBNPObject::GetTrianglesWithEdge( clsEdge* a_Edge )
	{
		std::vector<clsTriangle*> l_Triangles;
		for (size_t i = 0; i < m_Triangles.size( ); ++i)
		{
			for (size_t j = 0; j < m_Triangles[ i ]->GetEdges( )->size( ); ++j)
			{
				clsEdge* l_Edge = m_Triangles[ i ]->GetEdges( )->at( j );
				if (l_Edge == a_Edge)
				{
					l_Triangles.push_back( m_Triangles[ i ] );
					if (l_Triangles.size( ) == 2)
					{
						return l_Triangles;
					}
				}
			}
		}
		assert( "m_Triangles does not contain input." == 0 );
		return std::vector<clsTriangle*>( );
	}

	std::vector<clsLIE*> clsBNPObject::GetLIEsWithEdge( clsEdge* a_Edge )
	{
		std::vector<clsLIE*> l_LIEs;
		for (size_t i = 0; i < m_LIEs.size( ); ++i)
		{
			for (size_t j = 0; j < m_LIEs[ i ]->GetEdges( )->size( ); ++j)
			{
				clsEdge* l_LIEEdge = m_LIEs[ i ]->GetEdges( )->at( j );
				if (l_LIEEdge == a_Edge)
				{
					l_LIEs.push_back( m_LIEs[ i ] );
					if (l_LIEs.size( ) == 2)
					{
						return l_LIEs;
					}
				}
			}
		}

		assert( "m_LIEs does not contain input." == 0 );
		return std::vector<clsLIE*>( );
	}

	std::vector<clsEdge*> clsBNPObject::GetNewEdgesWithValues( glm::vec3 a_e1v1, glm::vec3 a_CommonVertex, glm::vec3 a_e2v2 )
	{
		std::vector<clsEdge*> l_Edges;
		clsEdge* l_e1 = GetEdgeWithVertices( a_e1v1, a_CommonVertex );
		clsEdge* l_e2 = GetEdgeWithVertices( a_CommonVertex, a_e2v2 );
		l_Edges.push_back( l_e1 );
		l_Edges.push_back( l_e2 );
		return l_Edges;
	}

	clsEdge* clsBNPObject::GetEdgeWithVertices( glm::vec3 a_v1, glm::vec3 a_v2 )
	{
		for (size_t i = 0; i < m_Edges.size( ); ++i)
		{
			glm::vec3 l_iv1 = m_Edges[ i ]->GetVertex1( )->GetPosition( );
			glm::vec3 l_iv2 = m_Edges[ i ]->GetVertex2( )->GetPosition( );
			if (a_v1 == l_iv1 && a_v2 == l_iv2 || a_v1 == l_iv2 && a_v2 == l_iv1)
			{
				return m_Edges[ i ];
			}
		}

		assert( "m_Edges does not contain input." == 0 );
		return nullptr;
	}
}
#endif
