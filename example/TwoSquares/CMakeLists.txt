

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

IF(HAVE_MOERTEL_MPI)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  TwoSquares
  NAME TwoSquares
  SOURCES TwoSquares.cpp
  ARGS
  COMM mpi
  NUM_MPI_PROCS 1
  PASS_REGULAR_EXPRESSION "Test passed!"
  XHOST trilinos-test.sandia.gov trilinos-test2.sandia.gov zan.sandia.gov
  )

ELSE()

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  TwoSquares
  NAME TwoSquares
  SOURCES TwoSquares.cpp
  ARGS 
  COMM serial
  PASS_REGULAR_EXPRESSION "Test passed!"
  )

ENDIF()


TRIBITS_COPY_FILES_TO_BINARY_DIR(
   TwoSquaresCopyFiles
   DEST_FILES TwoSquares.grid
   SOURCE_DIR ${PACKAGE_SOURCE_DIR}/example/TwoSquares
   EXEDEPS TwoSquares
   )
