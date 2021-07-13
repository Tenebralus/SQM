#pragma once
#include "stdafx.h"

namespace GenerationLib
{
	class clsGeometryVertex
	{
	public:
		clsGeometryVertex( );
		clsGeometryVertex( glm::vec3* Position );
		~clsGeometryVertex( );

		void SetPosition( glm::vec3* Position );
		void SetNormal( glm::vec3* Normal );
		void AddDuplicateVertex( glm::vec3* Vertex );
		void AddConnectedGeomVertex( clsGeometryVertex* GeomVertex );

		glm::vec3* GetPosition( );
		glm::vec3* GetNormal( );
		std::vector<glm::vec3*> GetDuplicateVertices( );
		std::vector<clsGeometryVertex*> GetConnectedGeomVertices( );

	private:
		glm::vec3* m_Position;
		glm::vec3* m_Normal;
		std::vector<glm::vec3*> m_DuplicateVertices;
		std::vector<clsGeometryVertex*> m_ConnectedGeomVertices;
	};
}
