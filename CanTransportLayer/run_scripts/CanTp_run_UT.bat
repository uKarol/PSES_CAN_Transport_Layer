cd ..
gcc UT_CanTp.c -o bins/UT_CanTp.exe
cd bins
UT_CanTp.exe
cd ..
gcc -fprofile-arcs -ftest-coverage -g UT_CanTp.c -o bins/UT_CanTp.exe
cd bins 
UT_CanTp.exe
cd ..
gcov UT_CanTp.c
pause