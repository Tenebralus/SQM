#pragma once
#include "stdafx.h"
#include "clsTriangle.h"
#include "clsPathVertex.h"

#if ENABLE_SQM
namespace GenerationLib
{
	class clsBNPObject
	{
	public:
		clsBNPObject( );
		~clsBNPObject( );

		std::vector<clsTriangle> GenerateBNP( OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		void GenerateUnrefinedBNP( OpenGLLib::Skeleton::clsJoint* CurrentJoint );
		void Triangulate( std::vector<glm::vec3> Vertices );
		void Triangulate2D( std::vector<glm::vec3> Vertices );
		void Triangulate3D( std::vector<glm::vec3> Points );
		bool ArePoints3D( std::vector<glm::vec3> Points );
		void Set3DSurfaceTriangulation( std::vector<glm::vec3> Points );
		clsTriangle* SetupTriangle( glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n = glm::vec3( 0, 0, 0 ) );
		void SetupWindingOrder( glm::vec3& r_v1, glm::vec3& r_v2, glm::vec3& r_v3 );
		void SetupPathVertices( OpenGLLib::Skeleton::clsJoint* Joint );
		void SubdivideBNP( );
		void Project( glm::vec3 JointPosition, float JointRadius, std::vector<glm::vec3> PathVertices );
		void SetupLIE( float JointRadius );
		void RemoveEdges( std::vector<clsEdge*> Edges );
		void SubdivideTriangle( clsTriangle* Triangle, int Index );
		void NewSubdividedTriangleVertices( glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3& r_e1m, glm::vec3& r_e2m, glm::vec3& r_e3m, glm::vec3& r_c );
		void SetVertexNormalsForFlatBNP( );
		bool IsVertexOnOuterEdgeOfFlatBNP( glm::vec3 Vertex );
		glm::vec3 GetAverageNormalFromEdges( std::vector<clsEdge*> Edges );
		glm::vec3 GetCommonVertexFromEdges( std::vector<clsEdge*> Edges );
		std::vector<clsEdge*> GetUniqueEdgesWithVertex( glm::vec3 Vertex );
		void MakeFlatBNP3D( glm::vec3 SphereCenter, float SphereRadius );
		std::vector<glm::vec3> ProjectFlatBNPVerticesToSphere( glm::vec3 Normal, glm::vec3 SphereCenter, float SphereRadius );
		void ProjectToSphereWithVertexNormals( glm::vec3 SphereCenter, float SphereRadius );
		void SetVertexNormals( );
		void SetFaceNormals( );
		std::vector<glm::vec3> GetEdgesFromConnectedVertexAsNormals( glm::vec3 Vertex );
		bool IsVertexAPathVertex( glm::vec3 Vertex, clsPathVertex** r_PathVertex );
		glm::vec3 GetVertexNormalBasedOnSurroundingFaces( glm::vec3 Vertex );
		std::vector<clsTriangle*> GetTrianglesAroundVertex( glm::vec3 Vertex );
		void SetLIE( clsPathVertex* r_PathVertex );
		void SetLIESegments( );
		clsLIESegment* AddNewLIESegment( clsPathVertex* PathVertex1, clsPathVertex* PathVertex2 );
		clsLIESegment* FindLIESegmentWithPathVertex( clsPathVertex* PathVertex1, clsPathVertex* PathVertex2 );
		void SmoothLIESegments( float JointRadius );
		void Split( clsLIESegment* Segment );
		void RemoveOldLIEEdge_Split( std::vector<clsLIE*> LIEs, clsLIESegment* Segment, clsEdge* OldEdge );
		void AddNewLIEEdges_Split( std::vector<clsLIE*> LIEs, clsLIESegment* Segment, std::vector<clsEdge*> NewEdges );
		void LinkLIEs( );
		int CompareLIEsByDistance( clsLIE* LIE1, clsLIE* LIE2 );
		void ReorderLIE( clsLIE* LIE, int Index );
		void LinkLIEVertices( clsLIE* LIE1, clsLIE* LIE2 );
		clsTriangle* AddTriangle( clsTriangle Triangle );
		void ReplaceSubdividedTrianglesInVectorTo( int Index );
		void RemoveEdge( clsEdge* Edge );
		void RemoveTriangle( clsTriangle* Triangle );
		void ConvertTrianglesToDelaunay( glm::vec3 JointPos );

		void ConnectCrossingPathVertices( std::vector<clsPathVertex*> PathVertices );
		void SetLocalVertexPositions( std::vector<glm::mat4> RotationMatrix, std::vector<glm::vec3> NewPos );
		void SetRotation( glm::mat3 Rotation );
		void ResetRotation( );

		void GenerateBNPPolygons( OpenGLLib::clsRenderPass* RenderPass );

		OpenGLLib::Skeleton::clsVertex* FindVertexIfExists( OpenGLLib::Skeleton::clsVertex Vertex );
		clsEdge* FindEdgeIfExists( clsEdge Edge );
		std::vector<clsTriangle*>* GetTriangles( );
		std::vector<clsPathVertex*> GetPathVertices( );
		glm::vec3 GetStraightSkeletonJointPos( );
		glm::vec3* GetJointPos( );
		float GetJointThickness( );
		glm::mat3 GetRotation( );
		glm::vec3 GetCenter( );
		glm::vec3 GetPathVerticesCenter( );
		std::vector<OpenGLLib::Skeleton::clsVertex*> GetVerticesWithPosition( glm::vec3 Position );
		std::vector<OpenGLLib::Skeleton::clsVertex*> GetVerticesConnectedToVertex( OpenGLLib::Skeleton::clsVertex* Vertex );
		std::vector<clsTriangle*> GetTrianglesWithEdge( clsEdge* Edge );
		std::vector<clsLIE*> GetLIEsWithEdge( clsEdge* Edge );
		std::vector<clsEdge*> GetNewEdgesWithValues( glm::vec3 e1v1, glm::vec3 CommonVertex, glm::vec3 e2v2 );
		clsEdge* GetEdgeWithVertices( glm::vec3 v1, glm::vec3 v2 );

		OpenGLLib::clsRenderPass* m_RenderPass;

	private:
		clsLinkedList<OpenGLLib::Skeleton::clsVertex> m_VerticesLinkedList;
		std::vector<OpenGLLib::Skeleton::clsVertex*> m_Vertices;
		clsLinkedList<clsEdge> m_EdgesLinkedList;
		std::vector<clsEdge*> m_Edges;
		clsLinkedList<clsTriangle> m_TrianglesLinkedList;
		std::vector<clsTriangle*> m_Triangles;
		clsLinkedList<clsPathVertex> m_PathVerticesLinkedList;
		std::vector<clsPathVertex*> m_PathVertices;
		clsLinkedList<clsLIE> m_LIEsLinkedList;
		std::vector<clsLIE*> m_LIEs;
		clsDelaunay3D* m_Del3D;
		unsigned int m_AmountOfVertsInTriangle;
		glm::vec3 m_StraightSkeletonJointPos;
		glm::vec3* m_JointPos;
		float m_JointThickness;
		glm::mat3 m_RotationMatrix;
		BrawlerEngineLib::Delaunay::clsDelaunay* m_Delaunay;
		clsLinkedList<clsLIESegment> m_LIESegmentsLinkedList;
		std::vector<clsLIESegment*> m_LIESegments;
		unsigned int m_SubdivideAmount;
		bool m_IsRoot;
	};
}
#endif
