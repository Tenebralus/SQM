#pragma once
#include "stdafx.h"
#include "clsSQMGeneration.h"

#if ENABLE_SQM
#include "clsSkeleton.h"
#include "clsBNPObject.h"
#include "clsEdge.h"
#include "clsLIE.h"
#include "clsBNPObject.h"
#include "clsSQMDebugData.h"
#include "clsLIESegment.h"

#include "../OpenGLLib/clsOpenGLDrawHelper.h"
#include "../OpenGLLib/clsOpenGL.h"
#include "../OpenGLLib/clsRenderPass.h"
#include "../OpenGLLib/clsBone.h"
#include "../OpenGLLib/clsJoint.h"
#include "../OpenGLLib/clsCircleVertices.h"

#include "../OpenGLLib/clsRenderPass.h"

namespace GenerationLib
{
	clsSQMGeneration::clsSQMGeneration( )
	{
		m_RenderPass = new OpenGLLib::clsRenderPass( );
	}

	clsSQMGeneration::~clsSQMGeneration( )
	{

	}

	void clsSQMGeneration::Destroy( )
	{
		for (size_t i = 0; i < m_BNPObjects.size( ); ++i)
		{
			m_BNPObjects[ i ] = nullptr;
			delete m_BNPObjects[ i ];
		}
		m_BNPObjects.clear( );
	}

	void clsSQMGeneration::Init( Skeleton::clsSkeleton* a_Skeleton )
	{
		SetFirstJointOfSkeleton( a_Skeleton->m_Joints[ 0 ] );
		StraightenSkeleton( a_Skeleton );
		SetBranchJoints( a_Skeleton->m_Joints[ 0 ] );
		GenerateUnrefinedBNPs( );
		ConnectPathVerticesInSkeleton( m_FirstJoint );
		SetRequiredSplitAmountPerLIE( );
		SplitLIEs( );
		AlignLIEsOfLinkedPathVertices( );

		SetBNPCircleVertices( a_Skeleton->m_Joints[ 0 ], a_Skeleton->m_Joints[ 0 ]->m_Position );
		SetCircleVerticesBetweenBNPs( a_Skeleton->m_Joints[ 0 ], nullptr );

		RevertSkeletonStraightening( a_Skeleton );

#if SQM_DEBUGMODE
		SetDebugData( );
		SQMDebugData.SetSkeleton( a_Skeleton->m_Joints );
#endif
	}

	void clsSQMGeneration::SetFirstJointOfSkeleton( OpenGLLib::Skeleton::clsJoint* a_FirstJoint )
	{
		m_FirstJoint = a_FirstJoint;
	}

	void clsSQMGeneration::StraightenSkeleton( Skeleton::clsSkeleton* a_Skeleton )
	{
		a_Skeleton->Straighten( a_Skeleton->m_Joints[ 0 ] );
	}

	void clsSQMGeneration::SetBranchJoints( OpenGLLib::Skeleton::clsJoint* a_FirstJoint )
	{
		FindNextBranchJointInSkeletonRecursive( a_FirstJoint );
	}

	void clsSQMGeneration::FindNextBranchJointInSkeletonRecursive( OpenGLLib::Skeleton::clsJoint* a_NextJoint )
	{
		if (a_NextJoint->m_Bones.size( ) > 1)
		{
			m_BranchJoints.push_back( a_NextJoint );
			for (size_t i = 0; i < a_NextJoint->m_Bones.size( ); ++i)
			{
				FindNextBranchJointInSkeletonRecursive( a_NextJoint->m_Bones[ i ]->GetEndJoint( ) );
			}
		}
		else if (a_NextJoint->m_Bones.size( ) == 1)
		{
			FindNextBranchJointInSkeletonRecursive( a_NextJoint->m_Bones[ 0 ]->GetEndJoint( ) );
		}
	}

	void clsSQMGeneration::GenerateUnrefinedBNPs( )
	{
		for (unsigned int i = 0; i < m_BranchJoints.size( ); ++i)
		{
			clsBNPObject* l_BNPObject = new clsBNPObject( );
			l_BNPObject->GenerateBNP( m_BranchJoints[ i ] );
			m_BNPObjects.push_back( l_BNPObject );
		}
	}

	void clsSQMGeneration::ConnectPathVerticesInSkeleton( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint, std::vector<clsPathVertex*> a_PathVertices )
	{
		if (a_CurrentJoint->m_Bones.size( ) > 1)
		{
			// branch joint found
			std::vector<clsPathVertex*> l_PathVertices;
			for (unsigned int i = 0; i < m_BNPObjects.size( ); ++i)
			{
				if (a_CurrentJoint->m_Position == m_BNPObjects[ i ]->GetStraightSkeletonJointPos( ))
				{
					if (a_PathVertices.size( ) > 0)
					{
						m_BNPObjects[ i ]->ConnectCrossingPathVertices( a_PathVertices );
					}

					for (unsigned int j = 0; j < m_BNPObjects[ i ]->GetPathVertices( ).size( ); ++j)
					{
						l_PathVertices.push_back( m_BNPObjects[ i ]->GetPathVertices( )[ j ] );
					}

					for (unsigned int j = 0; j < a_CurrentJoint->m_Bones.size( ); ++j)
					{
						ConnectPathVerticesInSkeleton( a_CurrentJoint->m_Bones[ j ]->GetEndJoint( ), l_PathVertices );
					}
					break;
				}
			}
		}
		else
		{
			if (a_CurrentJoint->m_Bones.size( ) > 0)
			{
				ConnectPathVerticesInSkeleton( a_CurrentJoint->m_Bones[ 0 ]->GetEndJoint( ), a_PathVertices );
			}
		}
	}

	void clsSQMGeneration::SetRequiredSplitAmountPerLIE( )
	{
		for (size_t i = 0; i < m_BNPObjects.size( ); ++i)
		{
			for (size_t j = 0; j < m_BNPObjects[ i ]->GetPathVertices( ).size( ); ++j)
			{
				if (m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetConnectedPathVertex( ) != nullptr)
				{
					clsLIE* l_LIE = m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( );
					clsLIE* l_OtherLIE = m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetConnectedPathVertex( )->GetLIE( );
					int l_SplitCount = (int) ( l_OtherLIE->GetEdges( )->size( ) - l_LIE->GetEdges( )->size( ) );
					l_LIE->m_RequiredAmountOfSplits = glm::max( 0, l_SplitCount );
				}
			}
		}
	}

	void clsSQMGeneration::SplitLIEs( )
	{
		for (size_t i = 0; i < m_BNPObjects.size( ); ++i)
		{
			for (size_t j = 0; j < m_BNPObjects[ i ]->GetPathVertices( ).size( ); ++j)
			{
				clsPathVertex* l_PathVertex = m_BNPObjects[ i ]->GetPathVertices( )[ j ];
				clsLIE* l_LIE = l_PathVertex->GetLIE( );

				while (l_LIE->m_RequiredAmountOfSplits > 0)
				{
					clsLIESegment* l_BestSegment = nullptr;
					unsigned int l_BestNeed;
					size_t l_SegmentIndex = -1;
					for (size_t k = 0; k < l_LIE->GetLIESegments( ).size( ); ++k)
					{
						if (j == 0 && i != 0) // ignore child LIE
						{
							continue;
						}

						clsLIESegment* l_LIESegment = l_LIE->GetLIESegments( )[ k ];
						clsPathVertex* l_OtherPathVertexOfLIESegment = l_LIESegment->GetOtherPathVertex( l_PathVertex );
						unsigned int l_Need = l_OtherPathVertexOfLIESegment->GetLIE( )->m_RequiredAmountOfSplits;

						if (( l_SegmentIndex == -1 ) || ( l_Need != 0 && l_BestNeed < l_Need ) || ( l_BestNeed == 0 && l_OtherPathVertexOfLIESegment->GetConnectedPathVertex( ) == nullptr ) || ( l_BestNeed == l_Need && l_LIESegment->m_DoneAmountOfSplits < l_BestSegment->m_DoneAmountOfSplits ))
						{
							l_BestNeed = l_Need;
							l_BestSegment = l_LIESegment;
							l_SegmentIndex = j;
						}
					}
					m_BNPObjects[ i ]->Split( l_BestSegment );

				}
			}
		}
	}

	void clsSQMGeneration::AlignLIEsOfLinkedPathVertices( )
	{
		for (size_t i = 0; i < m_BNPObjects.size( ); ++i)
		{
			m_BNPObjects[ i ]->LinkLIEs( );
		}
	}

	void clsSQMGeneration::SetBNPCircleVertices( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint, glm::vec3 a_PreviousBNPPosition )
	{
		if (a_CurrentJoint->m_Bones.size( ) > 1)
		{
			assert( a_CurrentJoint->GetBNPObject( ) != nullptr );
			for (size_t i = 0; i < a_CurrentJoint->GetBNPObject( )->GetPathVertices( ).size( ); ++i)
			{
				glm::vec3 l_Normal = glm::normalize( a_CurrentJoint->GetBNPObject( )->GetPathVertices( )[ i ]->GetPosition( ) - a_CurrentJoint->m_Position );
				a_CurrentJoint->AddCircleVertices( a_CurrentJoint->GetBNPObject( )->GetPathVertices( )[ i ]->GetLIE( )->GetVertices( ), l_Normal, &a_CurrentJoint->m_Position );
			}

			for (size_t i = 0; i < a_CurrentJoint->m_Bones.size( ); ++i)
			{
				a_CurrentJoint->SetType( 1 );
				SetBNPCircleVertices( a_CurrentJoint->m_Bones[ i ]->GetEndJoint( ), a_CurrentJoint->m_Position );
			}
		}
		else if (a_CurrentJoint->m_Bones.size( ) == 1)
		{
			a_CurrentJoint->SetType( 0 );
			SetBNPCircleVertices( a_CurrentJoint->m_Bones[ 0 ]->GetEndJoint( ), a_PreviousBNPPosition );
		}
		else if (a_CurrentJoint->m_Bones.size( ) == 0)
		{
			a_CurrentJoint->SetType( 2 );
		}
		else
		{
			assert( "current bone size is smaller than 0" == 0 );
		}
	}


	void clsSQMGeneration::SetCircleVerticesBetweenBNPs( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint, OpenGLLib::Skeleton::clsCircleVertices* a_CircleVertices )
	{
		if (a_CurrentJoint->m_Bones.size( ) > 1)
		{
			assert( a_CurrentJoint->GetBNPObject( ) != nullptr );
			int l_Index = 0;
			if (a_CurrentJoint->m_ConnectedBone != nullptr)
			{
				++l_Index;
			}
			ConnectToBranchingJoint( a_CircleVertices, a_CurrentJoint );
			for (int i = 0; i < a_CurrentJoint->m_Bones.size( ); ++i)
			{
				SetCircleVerticesBetweenBNPs( a_CurrentJoint->m_Bones[ i ]->GetEndJoint( ), a_CurrentJoint->GetCircleVerticesVectorPointerAt( l_Index + i ) );
			}
		}
		else if (a_CurrentJoint->m_Bones.size( ) == 1)
		{
			ConnectToNormalJoint( a_CircleVertices, a_CurrentJoint );
			SetCircleVerticesBetweenBNPs( a_CurrentJoint->m_Bones[ 0 ]->GetEndJoint( ), a_CurrentJoint->GetCircleVerticesVectorPointerAt( 0 ) );
		}
		else if (a_CurrentJoint->m_Bones.size( ) == 0)
		{
			ConnectToLeafJoint( a_CircleVertices, a_CurrentJoint );
		}
	}

	void clsSQMGeneration::ConnectToNormalJoint( OpenGLLib::Skeleton::clsCircleVertices* a_CircleVertices, OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		glm::vec3 l_JointNormal = glm::normalize( a_CurrentJoint->m_Bones[ 0 ]->GetEndPosition( ) - a_CurrentJoint->m_Position );

		OpenGLLib::Skeleton::clsCircleVertices l_NewCircleVertices;
		l_NewCircleVertices.SetJointPos( &a_CurrentJoint->m_Position );
		for (int i = 0; i < a_CircleVertices->GetVertices( )->size( ); ++i)
		{
			float l_Distance = 0;
			bool l_Intersected = BrawlerEngineLib::clsMath::This( ).IntersectPlane( l_JointNormal, a_CurrentJoint->m_Position, a_CircleVertices->GetVertices( )->at( i )->GetPosition( ), l_JointNormal, l_Distance );
			if (!l_Intersected)
			{
				l_Intersected = BrawlerEngineLib::clsMath::This( ).IntersectPlane( -l_JointNormal, a_CurrentJoint->m_Position, a_CircleVertices->GetVertices( )->at( i )->GetPosition( ), -l_JointNormal, l_Distance );
			}
			assert( l_Intersected );
			l_Distance = glm::abs( l_Distance );

			glm::vec3 l_NewVertexPosBeforeThickness = a_CircleVertices->GetVertices( )->at( i )->GetPosition( ) + l_JointNormal * l_Distance;
			glm::vec3 l_JointToNewVertNormal = glm::normalize( l_NewVertexPosBeforeThickness - a_CurrentJoint->m_Position );

			OpenGLLib::Skeleton::clsVertex l_NewVertex( a_CurrentJoint->m_Position + l_JointToNewVertNormal * a_CurrentJoint->m_Thickness );
			OpenGLLib::Skeleton::clsVertex* l_NewVertexPointer = l_NewCircleVertices.AddVertex( l_NewVertex );
			a_CircleVertices->GetVertices( )->at( i )->SetConnectedVertex( l_NewVertexPointer );
		}

		l_NewCircleVertices.SetNormal( l_JointNormal );
		l_NewCircleVertices.Smooth( a_CurrentJoint->m_Thickness );
		OpenGLLib::Skeleton::clsCircleVertices* l_NewCircleVerticesP = a_CurrentJoint->AddCircleVertices( l_NewCircleVertices );
		AddAnnularRegionToBuffer( a_CircleVertices, l_NewCircleVerticesP );
	}

	void clsSQMGeneration::ConnectToBranchingJoint( OpenGLLib::Skeleton::clsCircleVertices* a_CircleVertices, OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		if (a_CircleVertices != nullptr)
		{
			OpenGLLib::Skeleton::clsCircleVertices* l_CorrectCircleVertices = nullptr;
			float l_BestDistance = FLT_MAX;
			int l_PathVertexIndex = -1;
			for (size_t i = 0; i < a_CurrentJoint->GetBNPObject( )->GetPathVertices( ).size( ); ++i)
			{
				clsPathVertex* l_PathVertex = a_CurrentJoint->GetBNPObject( )->GetPathVertices( )[ i ];
				if (l_PathVertex->GetLIE( )->GetVertices( )->size( ) == a_CircleVertices->GetVertices( )->size( ))
				{
					float l_CurrDistance = 0.0f;
					for (size_t j = 0; j < a_CircleVertices->GetVertices( )->size( ); ++j)
					{
						l_CurrDistance += glm::distance( l_PathVertex->GetLIE( )->GetVertices( )->at( j )->GetPosition( ), a_CircleVertices->GetVertices( )->at( j )->GetPosition( ) );
					}
					if (l_CurrDistance < l_BestDistance)
					{
						l_BestDistance = l_CurrDistance;
						l_PathVertexIndex = (int) i;
					}
				}
			}
			assert( l_PathVertexIndex != -1 );

			l_CorrectCircleVertices = a_CurrentJoint->GetCircleVerticesVectorPointerAt( l_PathVertexIndex );
			assert( l_CorrectCircleVertices != nullptr );

			AddAnnularRegionToBuffer( a_CircleVertices, l_CorrectCircleVertices );
		}
	}

	void clsSQMGeneration::ConnectToLeafJoint( OpenGLLib::Skeleton::clsCircleVertices* a_CircleVertices, OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		glm::vec3 l_JointNormal = glm::normalize( a_CurrentJoint->m_Position - a_CurrentJoint->m_ConnectedBone->GetStartPosition( ) );

		OpenGLLib::Skeleton::clsCircleVertices l_NewCircleVertices;
		l_NewCircleVertices.SetJointPos( &a_CurrentJoint->m_Position );
		OpenGLLib::Skeleton::clsVertex l_NewVertex( a_CurrentJoint->m_Position );
		OpenGLLib::Skeleton::clsVertex* l_NewVertexPointer = l_NewCircleVertices.AddVertex( l_NewVertex );
		for (int i = 0; i < a_CircleVertices->GetVertices( )->size( ); ++i)
		{
			a_CircleVertices->GetVertices( )->at( i )->SetConnectedVertex( l_NewVertexPointer );
		}

		OpenGLLib::Skeleton::clsVertex* l_NewLastVertexPointer = l_NewCircleVertices.AddVertex( OpenGLLib::Skeleton::clsVertex( a_CurrentJoint->m_Position ) );
		l_NewLastVertexPointer->SetNormal( l_JointNormal );
		l_NewLastVertexPointer->SetPosition( a_CurrentJoint->m_Position + l_JointNormal * a_CurrentJoint->m_Thickness );
		for (int i = 0; i < l_NewCircleVertices.GetVertices( )->size( ); ++i)
		{
			l_NewCircleVertices.GetVertices( )->at( i )->SetConnectedVertex( l_NewLastVertexPointer );
		}

		l_NewCircleVertices.SetNormal( l_JointNormal );
		OpenGLLib::Skeleton::clsCircleVertices* l_NewCircleVerticesP = a_CurrentJoint->AddCircleVertices( l_NewCircleVertices );
		AddPolarRegionToBuffer( a_CircleVertices, l_NewCircleVerticesP );
	}

	void clsSQMGeneration::AddAnnularRegionToBuffer( OpenGLLib::Skeleton::clsCircleVertices* a_Circle1, OpenGLLib::Skeleton::clsCircleVertices* a_Circle2 )
	{
		assert( a_Circle1->GetVertices( )->size( ) == a_Circle2->GetVertices( )->size( ) );
		int l_NextIndex = 0;
		int l_NextUVIndex = 0;
		int l_Size = (int) a_Circle1->GetVertices( )->size( );
		for (int i = 0; i < l_Size; ++i)
		{
			l_NextIndex = ( i + 1 ) % l_Size;

			glm::vec3 l_v1 = a_Circle1->GetVertices( )->at( i )->GetPosition( );
			glm::vec3 l_v2 = a_Circle2->GetVertices( )->at( i )->GetPosition( );
			glm::vec3 l_v3 = a_Circle1->GetVertices( )->at( l_NextIndex )->GetPosition( );
			glm::vec3 l_v4 = a_Circle2->GetVertices( )->at( l_NextIndex )->GetPosition( );

			l_NextUVIndex = i + 1;

			glm::vec2 l_uv1 = glm::vec2( 0, (float) i / l_Size );
			glm::vec2 l_uv2 = glm::vec2( 1, (float) i / l_Size );
			glm::vec2 l_uv3 = glm::vec2( 0, (float) l_NextUVIndex / l_Size );
			glm::vec2 l_uv4 = glm::vec2( 1, (float) l_NextUVIndex / l_Size );
			AddQuadToBuffer( l_v1, l_v2, l_v3, l_v4, l_uv1, l_uv2, l_uv3, l_uv4 );
		}
	}

	// to do: change to triangle fan, also change render type to triangle fan in renderpass
	void clsSQMGeneration::AddPolarRegionToBuffer( OpenGLLib::Skeleton::clsCircleVertices* a_Circle1, OpenGLLib::Skeleton::clsCircleVertices* a_Circle2 )
	{
		int l_NextIndex = 0;
		int l_NextUVIndex = 0;
		int l_Size = (int) a_Circle1->GetVertices( )->size( );
		for (int i = 0; i < l_Size; ++i)
		{
			l_NextIndex = ( i + 1 ) % l_Size;

			glm::vec3 l_v1 = a_Circle1->GetVertices( )->at( i )->GetPosition( );
			glm::vec3 l_v2 = a_Circle2->GetVertices( )->at( 0 )->GetPosition( );
			glm::vec3 l_v3 = a_Circle1->GetVertices( )->at( l_NextIndex )->GetPosition( );
			glm::vec3 l_v4 = l_v2;

			l_NextUVIndex = i + 1;

			glm::vec2 l_uv1 = glm::vec2( 0, (float) i / l_Size );
			glm::vec2 l_uv2 = glm::vec2( 1, (float) 0 / l_Size );
			glm::vec2 l_uv3 = glm::vec2( 0, (float) l_NextUVIndex / l_Size );
			glm::vec2 l_uv4 = l_uv2;
			AddQuadToBuffer( l_v1, l_v2, l_v3, l_v4, l_uv1, l_uv2, l_uv3, l_uv4 );
		}
	}

	void clsSQMGeneration::AddQuadToBuffer( glm::vec3 a_v1, glm::vec3 a_v2, glm::vec3 a_v3, glm::vec3 a_v4, glm::vec2 a_uv1, glm::vec2 a_uv2, glm::vec2 a_uv3, glm::vec2 a_uv4 )
	{
		std::vector<glm::vec3> l_NewVertices = { a_v1, a_v2, a_v3, a_v3, a_v2, a_v4 };
		m_VertexPositionBuffer.insert( m_VertexPositionBuffer.end( ), l_NewVertices.begin( ), l_NewVertices.end( ) );

		std::vector<glm::vec2> l_NewUVs = { a_uv1, a_uv2, a_uv3, a_uv3, a_uv2, a_uv4 };
		m_VertexUVBuffer.insert( m_VertexUVBuffer.end( ), l_NewUVs.begin( ), l_NewUVs.end( ) );

		glm::vec3 l_Normal = glm::normalize( glm::cross( a_v2 - a_v1, a_v3 - a_v2 ) );
		std::vector<glm::vec3> l_NewNormals = { l_Normal, l_Normal, l_Normal, l_Normal, l_Normal, l_Normal };
		m_VertexNormalBuffer.insert( m_VertexNormalBuffer.end( ), l_NewNormals.begin( ), l_NewNormals.end( ) );
	}

	void clsSQMGeneration::RevertSkeletonStraightening( Skeleton::clsSkeleton* a_Skeleton )
	{
		a_Skeleton->RevertStraighten( a_Skeleton->m_Joints[ 0 ] );
	}

	void clsSQMGeneration::InitModelData( OpenGLLib::clsRenderPass* a_RenderPass )
	{
		m_RenderPass = a_RenderPass;

		for (unsigned int i = 0; i < m_BNPObjects.size( ); ++i)
		{
			//m_BNPObjects[ i ]->GenerateBNPPolygons( m_RenderPass );
		}

		GenerateModelPolygons( m_RenderPass );
		m_RenderPass->Init( m_RenderPass->GetVertexBuffer( ), m_RenderPass->GetUVBuffer( ), m_RenderPass->GetNormalBuffer( ), OpenGLLib::clsOpenGL::This( ).LoadMaterial( "Box" ) );
	}

	void clsSQMGeneration::GenerateModelPolygons( OpenGLLib::clsRenderPass* a_RenderPass )
	{
		assert( m_VertexPositionBuffer.size( ) == m_VertexUVBuffer.size( ) && m_VertexUVBuffer.size( ) == m_VertexNormalBuffer.size( ) );
		for (size_t i = 0; i < m_VertexPositionBuffer.size( ); ++i)
		{
			m_RenderPass->AddVertex( m_VertexPositionBuffer[ i ] );
			m_RenderPass->AddUV( m_VertexUVBuffer[ i ] );
			m_RenderPass->AddNormal( m_VertexNormalBuffer[ i ] );
		}
	}

	void clsSQMGeneration::SetDebugData( )
	{
		for (unsigned int i = 0; i < m_BNPObjects.size( ); ++i)
		{
			for (unsigned int j = 0; j < m_BNPObjects[ i ]->GetTriangles( )->size( ); ++j)
			{
				clsTriangle l_t = *m_BNPObjects[ i ]->GetTriangles( )->at( j );

				SQMDebugData.SetFaceNormalData( l_t );
				for (int k = 0; k < l_t.GetVertices( )->size( ); ++k)
				{
					SQMDebugData.SetVertexNormalData( *l_t.GetVertices( )->at( k ) );
				}
			}



			SQMDebugData.SetLIEData( m_BNPObjects[ i ]->GetPathVertices( ) );
			SQMDebugData.SetBNPCenterData( m_BNPObjects[ i ] );
			SQMDebugData.AddJointPosData( m_BNPObjects[ i ]->GetStraightSkeletonJointPos( ) );

			for (unsigned int j = 0; j < m_BNPObjects[ i ]->GetPathVertices( ).size( ); ++j)
			{
				if (m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( ) != nullptr)
				{
					clsLIE* l_LIE = m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( );

					// LIE Segments
					for (size_t k = 0; k < l_LIE->GetLIESegments( ).size( ); ++k)
					{
						for (size_t l = 0; l < l_LIE->GetLIESegments( )[ k ]->GetEdges( ).size( ); ++l)
						{
							float l_CurrEdgeInLIESegmentPercent = (float) l / (float) ( l_LIE->GetLIESegments( )[ k ]->GetEdges( ).size( ) - 1 );
							glm::vec3 l_LineColor = glm::vec3( l_CurrEdgeInLIESegmentPercent, l_CurrEdgeInLIESegmentPercent, l_CurrEdgeInLIESegmentPercent );
							SQMDebugData.AddLine( l_LIE->GetLIESegments( )[ k ]->GetEdges( )[ l ]->GetVertex1( )->GetPosition( ), l_LIE->GetLIESegments( )[ k ]->GetEdges( )[ l ]->GetVertex2( )->GetPosition( ), l_LineColor );
						}
					}

					// LIE connections across BNPs
					for (unsigned int k = 0; k < m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( )->GetEdges( )->size( ); ++k)
					{
						for (size_t l = 0; l < m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( )->GetVertexAt( k )->GetConnectedVertices( ).size( ); ++l)
						{
							if (m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetConnectedPathVertex( ) != nullptr)
							{
								glm::vec3 l_Point1 = m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( )->GetVertexAt( k )->GetPosition( );
								glm::vec3 l_Point2 = m_BNPObjects[ i ]->GetPathVertices( )[ j ]->GetLIE( )->GetVertexAt( k )->GetConnectedVertices( )[ l ]->GetPosition( );
								SQMDebugData.AddVertexConnectionsInLIE( l_Point1, l_Point2 );
							}
						}
					}
				}
			}
		}
	}

	std::vector<clsBNPObject*>* clsSQMGeneration::GetBNPObjects( )
	{
		return &m_BNPObjects;
	}

	OpenGLLib::Skeleton::clsJoint* clsSQMGeneration::FindLastJointWithCircleVerticesData( OpenGLLib::Skeleton::clsJoint* a_CurrentJoint )
	{
		OpenGLLib::Skeleton::clsJoint* l_LastJoint = nullptr;
		if (a_CurrentJoint->GetCircleVerticesVectorSize( ) < 1)
		{
			l_LastJoint = FindLastJointWithCircleVerticesData( a_CurrentJoint->m_Bones[ 0 ]->GetEndJoint( ) );
		}
		else
		{
			return a_CurrentJoint;
		}
		return l_LastJoint;
	}
}
#endif
