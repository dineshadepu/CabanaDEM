#ifndef PARTICLES_H
#define PARTICLES_H

#include <memory>

#include "mpi.h"

#include <Kokkos_Core.hpp>

#include <Cabana_Core.hpp>
#include <Cabana_Grid.hpp>

#define DIM 3

namespace CabanaDEM
{
  template <class MemorySpace, int Dimension>
  class Particles
  {
  public:
    using memory_space = MemorySpace;
    using execution_space = typename memory_space::execution_space;
    static constexpr int dim = Dimension;

    using double_type = Cabana::MemberTypes<double>;
    using int_type = Cabana::MemberTypes<int>;
    using vec_double_type = Cabana::MemberTypes<double[dim]>;
    using vec_int_type = Cabana::MemberTypes<int[dim]>;

    // FIXME: add vector length.
    // FIXME: enable variable aosoa.
    using aosoa_double_type = Cabana::AoSoA<double_type, memory_space, 1>;
    using aosoa_int_type = Cabana::AoSoA<int_type, memory_space, 1>;
    using aosoa_vec_double_type = Cabana::AoSoA<vec_double_type, memory_space, 1>;
    using aosoa_vec_int_type = Cabana::AoSoA<vec_int_type, memory_space, 1>;

    // Constructor which initializes particles on regular grid.
    template <class ExecSpace>
    Particles( const ExecSpace& exec_space, std::size_t no_of_particles )
    {
      _no_of_particles = no_of_particles;
      resize( _no_of_particles );
      createParticles( exec_space );
    }

    // Constructor which initializes particles on regular grid.
    template <class ExecSpace>
    void createParticles( const ExecSpace& exec_space )
    {
      auto x = slicePosition();

      auto create_particles_func = KOKKOS_LAMBDA( const int i )
	{
	  for (int j=0; j < DIM; j++){
	    // x( i, j ) = DIM * i + j;
	  }
	};
      Kokkos::RangePolicy<ExecSpace> policy( 0, x.size() );
      Kokkos::parallel_for( "create_particles_lambda", policy,
			    create_particles_func );
    }

    template <class ExecSpace, class FunctorType>
    void updateParticles( const ExecSpace, const FunctorType init_functor )
    {
      Kokkos::RangePolicy<ExecSpace> policy( 0, _no_of_particles );
      Kokkos::parallel_for(
			   "CabanaPD::Particles::update_particles", policy,
			   KOKKOS_LAMBDA( const int pid ) { init_functor( pid ); } );
    }

    auto slicePosition()
    {
      return Cabana::slice<0>( _x, "positions" );
    }
    auto slicePosition() const
    {
      return Cabana::slice<0>( _x, "positions" );
    }
    auto sliceVelocity()
    {
      return Cabana::slice<0>( _u, "velocities" );
    }
    auto sliceVelocity() const
    {
      return Cabana::slice<0>( _u, "velocities" );
    }
    auto sliceAcceleration()
    {
      return Cabana::slice<0>( _au, "accelerations" );
    }
    auto sliceAcceleration() const
    {
      return Cabana::slice<0>( _au, "accelerations" );
    }
    auto sliceMass() {
      return Cabana::slice<0>( _m, "mass" );
    }
    auto sliceMass() const
    {
      return Cabana::slice<0>( _m, "mass" );
    }
    auto sliceDensity() {
      return Cabana::slice<0>( _rho, "density" );
    }
    auto sliceDensity() const
    {
      return Cabana::slice<0>( _rho, "density" );
    }
    auto sliceRadius() {
      return Cabana::slice<0>( _rad, "radius" );
    }
    auto sliceRadius() const
    {
      return Cabana::slice<0>( _rad, "radius" );
    }

    void resize(const std::size_t n)
    {
      _x.resize( n );
      _u.resize( n );
      _au.resize( n );
      _m.resize( n );
      _rho.resize( n );
      _rad.resize( n );
    }

    void output(  const int output_step,
                  const double output_time,
                  const bool use_reference = true )
    {
      // _output_timer.start();

#ifdef Cabana_ENABLE_HDF5
      Cabana::Experimental::HDF5ParticleOutput::writeTimeStep(
							      h5_config,
							      "particles",
							      MPI_COMM_WORLD,
							      output_step,
							      output_time,
							      _no_of_particles,
							      slicePosition(),
							      sliceVelocity(),
							      sliceAcceleration(),
							      sliceMass(),
							      sliceDensity(),
							      sliceRadius());
      // #else
      // #ifdef Cabana_ENABLE_SILO
      //       Cabana::Grid::Experimental::SiloParticleOutput::
      // 	writePartialRangeTimeStep(
      // 				  "particles", local_grid->globalGrid(), output_step, output_time,
      // 				  0, n_local, base_type::getPosition( use_reference ),
      // 				  base_type::sliceStrainEnergy(), base_type::sliceForce(),
      // 				  base_type::sliceDisplacement(), base_type::sliceVelocity(),
      // 				  base_type::sliceDamage(), sliceWeightedVolume(),
      // 				  sliceDilatation() );
#else
      std::cout << "No particle output enabled.";
      // log( std::cout, "No particle output enabled." );
      // #endif
#endif

      // _output_timer.stop();
    }

  private:
    int _no_of_particles;
    aosoa_vec_double_type _x;
    aosoa_vec_double_type _u;
    aosoa_vec_double_type _au;
    aosoa_double_type _m;
    aosoa_double_type _rho;
    aosoa_double_type _rad;

#ifdef Cabana_ENABLE_HDF5
    Cabana::Experimental::HDF5ParticleOutput::HDF5Config h5_config;
#endif

    // Cabana::Experimental::HDF5ParticleOutput::HDF5Config h5_config;
  };

} // namespace CabanaDEM

#endif
