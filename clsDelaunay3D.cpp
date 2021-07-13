#pragma once
#include "stdafx_CGAL.h"
#include "stdafx.h"
#include "clsDelaunay3D.h"

#if ENABLE_SQM
namespace GenerationLib
{
	clsDelaunay3D::clsDelaunay3D( )
	{

	}

	clsDelaunay3D::~clsDelaunay3D( )
	{

	}

	void clsDelaunay3D::Init( )
	{

	}

	void clsDelaunay3D::AddPoint( glm::vec3 a_Position )
	{
		Vertex_handle vertex = m_Triangulation.insert( Point( a_Position.x, a_Position.y, a_Position.z ) );
	}

	void clsDelaunay3D::Triangulate( std::vector<glm::vec3> a_Points )
	{
		std::vector<Point> l_DelPoints;
		for (unsigned int i = 0; i < a_Points.size( ); ++i)
		{
			l_DelPoints.push_back( Point( a_Points[ i ].x, a_Points[ i ].y, a_Points[ i ].z ) );
		}
		m_Triangulation.insert( l_DelPoints.begin( ), l_DelPoints.end( ) );
	}

	Delaunay::Cell_handle clsDelaunay3D::GetInfiniteCell( )
	{
		return m_Triangulation.infinite_cell( );
	}

	Delaunay::All_vertices_iterator clsDelaunay3D::GetAllVerticesIteratorBegin( )
	{
		return m_Triangulation.all_vertices_begin( );
	}

	Delaunay::All_vertices_iterator clsDelaunay3D::GetAllVerticesIteratorEnd( )
	{
		return m_Triangulation.all_vertices_end( );
	}

	Delaunay::Finite_vertices_iterator clsDelaunay3D::GetInnerVerticesIteratorBegin( )
	{
		return m_Triangulation.finite_vertices_begin( );
	}

	Delaunay::Finite_vertices_iterator clsDelaunay3D::GetInnerVerticesIteratorEnd( )
	{
		return m_Triangulation.finite_vertices_end( );
	}

	Delaunay::All_edges_iterator clsDelaunay3D::GetAllEdgesIteratorBegin( )
	{
		return m_Triangulation.all_edges_begin( );
	}

	Delaunay::All_edges_iterator clsDelaunay3D::GetAllEdgesIteratorEnd( )
	{
		return m_Triangulation.all_edges_end( );
	}

	Delaunay::Finite_edges_iterator clsDelaunay3D::GetInnerEdgesIteratorBegin( )
	{
		return m_Triangulation.finite_edges_begin( );
	}

	Delaunay::Finite_edges_iterator clsDelaunay3D::GetInnerEdgesIteratorEnd( )
	{
		return m_Triangulation.finite_edges_end( );
	}

	Delaunay::All_facets_iterator clsDelaunay3D::GetAllFacetsIteratorBegin( )
	{
		return m_Triangulation.all_facets_begin( );
	}

	Delaunay::All_facets_iterator clsDelaunay3D::GetAllFacetsIteratorEnd( )
	{
		return m_Triangulation.all_facets_end( );
	}

	Delaunay::Finite_facets_iterator clsDelaunay3D::GetInnerFacetsIteratorBegin( )
	{
		return m_Triangulation.finite_facets_begin( );
	}

	Delaunay::Finite_facets_iterator clsDelaunay3D::GetInnerFacetsIteratorEnd( )
	{
		return m_Triangulation.finite_facets_end( );
	}

	Delaunay::All_cells_iterator clsDelaunay3D::GetAllCellsIteratorBegin( )
	{
		return m_Triangulation.all_cells_begin( );
	}

	Delaunay::All_cells_iterator clsDelaunay3D::GetAllCellsIteratorEnd( )
	{
		return m_Triangulation.all_cells_end( );
	}

	Delaunay::Finite_cells_iterator clsDelaunay3D::GetInnerCellsIteratorBegin( )
	{
		return m_Triangulation.finite_cells_begin( );
	}

	Delaunay::Finite_cells_iterator clsDelaunay3D::GetInnerCellsIteratorEnd( )
	{
		return m_Triangulation.finite_cells_end( );
	}

	void clsDelaunay3D::Destroy( )
	{

	}
}
#endif
