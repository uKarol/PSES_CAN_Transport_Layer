cd ..
gcc UT_CanTp.c CanTp_Timer.c -o bins/UT_CanTp.exe
cd bins
UT_CanTp.exe
cd ..
gcc -fprofile-arcs -ftest-coverage -g UT_CanTp.c CanTp_Timer.c -o bins/UT_CanTp.exe
cd bins 
UT_CanTp.exe
cd ..
gcov UT_CanTp.c
python -m gcovr -r . --html --html-details -o coverage.html
pause