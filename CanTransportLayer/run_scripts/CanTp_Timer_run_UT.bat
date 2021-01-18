cd ..
gcc UT_CanTp_Timer.c -o bins/UT_CanTp_Timer.exe
cd bins
UT_CanTp_Timer.exe
cd ..
gcc -fprofile-arcs -ftest-coverage -g UT_CanTp_Timer.c -o bins/UT_CanTp_Timer.exe
cd bins 
UT_CanTp_Timer.exe
cd ..
gcov UT_CanTp_Timer.c
python -m gcovr -r . --html --html-details -o coverage.html
pause