# - Find PostgreSQL
# Find the native PostgreSQL includes and library
#
#  POSTGRESQL_INCLUDE_DIR - where to find libpq-fe.h, etc.
#  POSTGRESQL_LIBRARIES   - List of libraries when using PostgreSQL.
#  POSTGRESQL_FOUND       - True if PostgreSQL found.

if (POSTGRESQL_INCLUDE_DIR)
  set(POSTGRESQL_FIND_QUIETLY TRUE)
endif (POSTGRESQL_INCLUDE_DIR)

find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h
  /usr/local/include/postgresql
  /usr/include/postgresql
  /usr/include
  /opt/homebrew/include
  /opt/local/include
)

find_library(POSTGRESQL_LIBRARY
  NAMES pq
  PATHS
  /usr/local/lib
  /usr/lib
  /opt/homebrew/lib
  /opt/local/lib
)

if (POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARY)
  set(POSTGRESQL_FOUND TRUE)
  set(POSTGRESQL_LIBRARIES ${POSTGRESQL_LIBRARY})
else (POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARY)
  set(POSTGRESQL_FOUND FALSE)
  set(POSTGRESQL_LIBRARIES)
endif (POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARY)

if (POSTGRESQL_FOUND)
  if (NOT POSTGRESQL_FIND_QUIETLY)
    message(STATUS "Found PostgreSQL: ${POSTGRESQL_LIBRARY}")
  endif (NOT POSTGRESQL_FIND_QUIETLY)
else (POSTGRESQL_FOUND)
  if (POSTGRESQL_FIND_REQUIRED)
    message(STATUS "Looked for PostgreSQL libraries named pq.")
    message(FATAL_ERROR "Could NOT find PostgreSQL library")
  endif (POSTGRESQL_FIND_REQUIRED)
endif (POSTGRESQL_FOUND)

mark_as_advanced(
  POSTGRESQL_LIBRARY
  POSTGRESQL_INCLUDE_DIR
)