#pragma once
#include "stdafx.h"
#include "clsGeometryVertex.h"

namespace GenerationLib
{
	clsGeometryVertex::clsGeometryVertex( )
	{

	}

	clsGeometryVertex::clsGeometryVertex( glm::vec3* a_Position )
	{
		SetPosition( a_Position );
	}

	clsGeometryVertex::~clsGeometryVertex( )
	{

	}

	void clsGeometryVertex::SetPosition( glm::vec3* a_Position )
	{
		m_Position = a_Position;
	}

	void clsGeometryVertex::SetNormal( glm::vec3* a_Normal )
	{
		m_Normal = a_Normal;
	}

	void clsGeometryVertex::AddDuplicateVertex( glm::vec3* a_GeomVertex )
	{
		m_DuplicateVertices.push_back( a_GeomVertex );
	}

	void clsGeometryVertex::AddConnectedGeomVertex( clsGeometryVertex* a_GeomVertex )
	{
		m_ConnectedGeomVertices.push_back( a_GeomVertex );
	}

	glm::vec3* clsGeometryVertex::GetPosition( )
	{
		return m_Position;
	}

	glm::vec3* clsGeometryVertex::GetNormal( )
	{
		return m_Normal;
	}

	std::vector<glm::vec3*> clsGeometryVertex::GetDuplicateVertices( )
	{
		return m_DuplicateVertices;
	}

	std::vector<clsGeometryVertex*> clsGeometryVertex::GetConnectedGeomVertices( )
	{
		return m_ConnectedGeomVertices;
	}
}
