macro(configure_pthreads)
    message ("Dependency check for pthreads multi-threading support...")

    if(UNIX)
        find_package(Threads REQUIRED)
        if(Threads_FOUND)
            message(STATUS "Found threading library")
            if(CMAKE_USE_PTHREADS_INIT)
                message(STATUS "Found pthreads " ${CMAKE_THREAD_LIBS_INIT})
            else()
                message(STATUS "Pthreads not found")
            endif()
        else()
            message(STATUS "Threading library not found")
        endif()
    else()
       message(STATUS "Not required on windows")
    endif()
endmacro(configure_pthreads)

macro(configure_argp)

   find_package(Argp)
   message(STATUS "Argp libs:" ${ARGP_LIBRARIES} )
   message(STATUS "Argp includes:" ${ARGP_INCLUDE_PATH} )
   add_definitions(-DARGP_FOUND)
endmacro(configure_argp)

macro(configure_mpi)
  message ("Dependency check for mpi...")

  find_package(MPI)
  if(${MPI_C_FOUND})
    message(STATUS "MPI found")
  else()
    message(STATUS "MPI not found")
  endif()
   
endmacro(configure_mpi)

macro(configure_openmp)
  message ("Dependency check for OpenMP")

  find_package(OpenMP)
  if(${OPENMP_FOUND})
      message(STATUS "OpenMP found")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OpenMP_CXX_FLAGS}")
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OpenMP_C_FLAGS}")
  else()
      message(STATUS "OpenMP not found")
  endif()
   
endmacro(configure_openmp)


macro(configure_blas)
  message ("Dependency check for blas...")
  set(BLA_VENDOR "Generic")
  find_package( BLAS REQUIRED )
  message(STATUS BLAS: ${BLAS_LIBRARIES} )

  find_path(BLAS_INCLUDE_DIRS cblas.h
  /usr/include
  /usr/local/include
  $ENV{BLAS_HOME}/include)
  message(STATUS ${BLAS_INCLUDE_DIRS})

endmacro(configure_blas)

macro(configure_lapack)
  message ("Dependency check for lapack...")
  set(BLA_VENDOR "Generic")
  find_package(LAPACK REQUIRED)
  message(STATUS LAPACK: ${LAPACK_LIBRARIES})

endmacro(configure_lapack)

macro(configure_openblas)
  message ("Dependency check for openblas...")
  
  if(MSVC)
  set(BLAS_LIBRARIES  $ENV{BLAS_LIBRARIES})
  set(BLAS_INCLUDES $ENV{BLAS_INCLUDES})
  set(BLAS_FOUND ON)
  else()
  set(BLA_VENDOR "OpenBLAS")
  find_package( BLAS REQUIRED )
  endif()
  
  message(STATUS BLAS: ${BLAS_LIBRARIES} )
 
endmacro(configure_openblas)

macro(configure_eigen)
  message ("Dependency check for eigen...")
  
  if(MSVC)
  SET(EIGEN3_INCLUDE_DIR $ENV{EIGEN3_INCLUDE_DIR})
  else()
  find_package(Eigen3 REQUIRED)
  endif()
  
  message(STATUS EIGEN3: ${EIGEN3_INCLUDE_DIR})

  add_definitions(-DEIGEN_DONT_PARALLELIZE)
endmacro(configure_eigen)

macro(configure_boost)
  message ("Dependency check for boost...")
  
  set (Boost_USE_STATIC_LIBS ON)
  set (Boost_USE_MULTITHREADED ON)

  set (BOOST_COMPONENTS system 
                        program_options)

  FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

  if(Boost_FOUND)
  message("Found Boost_VERSION: ${Boost_VERSION}")
  message("Found Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
  message("Found Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
  else()
  message("Boost library is not found")
  endif()

endmacro(configure_boost)


