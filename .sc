__PROJECT_FILE=true
project_name="sc"
project_source_dirs=( "src" )
project_common_flags=( "-fsanitize=address" "-g" "-Isrc" "-pedantic-errors" "-std=gnu99" "-Wall" "-Werror" "-Wextra" "-Wpedantic" "-Wno-pointer-arith" )
project_linker_libs=( "-lgmp" )
