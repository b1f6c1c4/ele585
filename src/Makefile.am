AM_CFLAGS = $(MORE_CFLAGS) $(MPI_CFLAGS)
AM_CXXFLAGS = $(MORE_CXXFLAGS) $(MPI_CXXFLAGS)
AM_LDFLAGS = $(MORE_LDFLAGS) $(MPI_LDFLAGS)

bin_PROGRAMS = sn-gen sn-mpi

sn_gen_SOURCES  = sn-gen.cpp generator.cpp
sn_gen_CFLAGS   = $(AM_CFLAGS)
sn_gen_CXXFLAGS = $(AM_CXXFLAGS)
sn_gen_LDFLAGS  = $(AM_LDFLAGS)

sn_mpi_SOURCES  = main.h main.cpp
sn_mpi_CFLAGS   = $(AM_CFLAGS)
sn_mpi_CXXFLAGS = $(AM_CXXFLAGS)
sn_mpi_LDFLAGS  = $(AM_LDFLAGS)
