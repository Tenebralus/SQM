#pragma once
#include "stdafx.h"
#include "clsGeometryVertex.h"

#if ENABLE_SQM
namespace GenerationLib
{
	class clsSQMGeneration
	{
	public:
		clsSQMGeneration( );
		~clsSQMGeneration( );
		void Destroy( );
		void Init( Skeleton::clsSkeleton* Skeleton );

		void SetFirstJointOfSkeleton( OpenGLLib::Skeleton::clsJoint* FirstJoint );
		void StraightenSkeleton( Skeleton::clsSkeleton* Skeleton );
		void SetBranchJoints( OpenGLLib::Skeleton::clsJoint* FirstJoint );
		void FindNextBranchJointInSkeletonRecursive( OpenGLLib::Skeleton::clsJoint* NextJoint );

		void GenerateUnrefinedBNPs( );
		void ConnectPathVerticesInSkeleton( OpenGLLib::Skeleton::clsJoint* CurrentJoint, std::vector<clsPathVertex*> PathVertices = std::vector<clsPathVertex*>( ) );
		void SetRequiredSplitAmountPerLIE( );
		void SplitLIEs( );
		void AlignLIEsOfLinkedPathVertices( );

		void SetBNPCircleVertices( OpenGLLib::Skeleton::clsJoint* CurrentJoint, glm::vec3 PreviousBNPPosition );
		void SetCircleVerticesBetweenBNPs( OpenGLLib::Skeleton::clsJoint* CurrentJoint, OpenGLLib::Skeleton::clsCircleVertices* CircleVertices );
		void ConnectToNormalJoint( OpenGLLib::Skeleton::clsCircleVertices* CircleVertices, OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		void ConnectToBranchingJoint( OpenGLLib::Skeleton::clsCircleVertices* CircleVertices, OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		void ConnectToLeafJoint( OpenGLLib::Skeleton::clsCircleVertices* CircleVertices, OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		void AddAnnularRegionToBuffer( OpenGLLib::Skeleton::clsCircleVertices* Circle1, OpenGLLib::Skeleton::clsCircleVertices* Circle2 );
		void AddPolarRegionToBuffer( OpenGLLib::Skeleton::clsCircleVertices* Circle1, OpenGLLib::Skeleton::clsCircleVertices* Circle2 );
		void AddQuadToBuffer( glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, glm::vec2 uv4 );

		void RevertSkeletonStraightening( Skeleton::clsSkeleton* Skeleton );

		void InitModelData( OpenGLLib::clsRenderPass* RenderPass );
		void GenerateModelPolygons( OpenGLLib::clsRenderPass* RenderPass );

		void SetDebugData( );

		std::vector<clsBNPObject*>* GetBNPObjects( );
		OpenGLLib::Skeleton::clsJoint* FindLastJointWithCircleVerticesData( OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		OpenGLLib::clsRenderPass* m_RenderPass;

	private:
		std::vector<clsBNPObject*> m_BNPObjects;
		std::vector<OpenGLLib::Skeleton::clsJoint*> m_BranchJoints;
		OpenGLLib::Skeleton::clsJoint* m_FirstJoint;

		std::vector<glm::vec3> m_VertexPositionBuffer;
		std::vector<glm::vec2> m_VertexUVBuffer;
		std::vector<glm::vec3> m_VertexNormalBuffer;
		clsLinkedList<clsGeometryVertex> m_GeomVerticesLinkedList;
		std::vector<clsGeometryVertex*> m_GeomVertices;
	};
}
#endif
