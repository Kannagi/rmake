#rmake

#name executable
exe_name:rmake

#command execute
cmd_execute:./rmake

#file .c or .cpp or folder : main.c;test.c , src;core;
add_folder:src
add_file:main.c;test/test.c

#--------compiler option------------
libdirs: 
includes:

#-O1 -O2 -O3 -Wall -Wextra -pedantic
options:-O2 -Wall -Wextra

#-lnamelib 
libs:

cmd_precompiler:
cmd_compiler:$compiler $options $includes -c $file -o $object
cmd_linker:$linker -s $libdirs $object -o $exe_name $libs
cmd_postcompiler:

#--------compiler config------------
compilerc:gcc
compilercpp:g++
linker:g++