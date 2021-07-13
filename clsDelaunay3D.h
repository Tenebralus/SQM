#pragma once
#include "stdafx_CGAL.h"
#include "stdafx.h"

#if ENABLE_SQM
namespace GenerationLib
{
	template<class GT, class Vb = CGAL::Triangulation_vertex_base_3<GT>>
	class My_vertex_base : public Vb
	{
	public:
		typedef typename Vb::Vertex_handle	Vertex_handle;
		typedef typename Vb::Point			Point;
		typedef typename Vb::Cell_handle	Cell_handle;

		template<class TDS2>
		struct Rebind_TDS
		{
			typedef typename Vb::template Rebind_TDS<TDS2>::Other	Vb2;
			typedef My_vertex_base<GT, Vb2>							Other;
		};

		My_vertex_base( )
		{
		}

		My_vertex_base( const Point&p ) : Vb( p )
		{
		}

		My_vertex_base( const Point&p, Cell_handle c ) : Vb( p, c )
		{
		}

		Vertex_handle vh;
		Cell_handle ch;
	};

	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef CGAL::Triangulation_data_structure_3<My_vertex_base<K>> Tds;
	typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
	typedef Delaunay::Vertex_handle Vertex_handle;
	typedef Delaunay::Point Point;

	class clsDelaunay3D
	{
	public:
		clsDelaunay3D( );
		~clsDelaunay3D( );
		void Init( );
		void AddPoint( glm::vec3 Position );
		void Triangulate( std::vector<glm::vec3> Points );

		Delaunay::Cell_handle GetInfiniteCell( );

		Delaunay::All_vertices_iterator GetAllVerticesIteratorBegin( );
		Delaunay::All_vertices_iterator GetAllVerticesIteratorEnd( );
		Delaunay::Finite_vertices_iterator GetInnerVerticesIteratorBegin( );
		Delaunay::Finite_vertices_iterator GetInnerVerticesIteratorEnd( );

		Delaunay::All_edges_iterator GetAllEdgesIteratorBegin( );
		Delaunay::All_edges_iterator GetAllEdgesIteratorEnd( );
		Delaunay::Finite_edges_iterator GetInnerEdgesIteratorBegin( );
		Delaunay::Finite_edges_iterator GetInnerEdgesIteratorEnd( );

		Delaunay::All_facets_iterator GetAllFacetsIteratorBegin( );
		Delaunay::All_facets_iterator GetAllFacetsIteratorEnd( );
		Delaunay::Finite_facets_iterator GetInnerFacetsIteratorBegin( );
		Delaunay::Finite_facets_iterator GetInnerFacetsIteratorEnd( );

		Delaunay::All_cells_iterator GetAllCellsIteratorBegin( );
		Delaunay::All_cells_iterator GetAllCellsIteratorEnd( );
		Delaunay::Finite_cells_iterator GetInnerCellsIteratorBegin( );
		Delaunay::Finite_cells_iterator GetInnerCellsIteratorEnd( );
		void Destroy( );
		Delaunay m_Triangulation;
	private:

	};
}
#endif
