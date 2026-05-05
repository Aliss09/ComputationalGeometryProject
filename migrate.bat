@echo off
REM ────────────────────────────────────────────────────────────────────
REM migrate.bat — Migrate flat structure → prof's template structure
REM Run this in MSYS2 UCRT64 from D:\0gomath\gjk_project
REM ────────────────────────────────────────────────────────────────────

echo Creating new folder structure...

REM ── Make new folders
mkdir cgproject 2>nul
cd cgproject
mkdir report
mkdir src\algorithms
mkdir src\data_structures
mkdir experiments\results
mkdir data\synthetic
mkdir data\real
mkdir visualization
mkdir demo\screenshots
mkdir tests
cd ..

echo Copying source files to new locations...

REM ── Copy headers from old structure
copy gjk_project\include\math2d.h        cgproject\src\data_structures\
copy gjk_project\include\convex_hull.h   cgproject\src\algorithms\
copy gjk_project\include\gjk.h           cgproject\src\algorithms\
copy gjk_project\include\bvh.h           cgproject\src\algorithms\
copy gjk_project\include\object.h        cgproject\src\data_structures\

REM ── Copy main.cpp
copy gjk_project\src\main.cpp            cgproject\src\

echo Done!
echo.
echo Next steps:
echo   1. Place Final_Report.pdf in cgproject\report\final_report.pdf
echo   2. Place YouTube link in cgproject\demo\video_link.txt
echo   3. Test build: cd cgproject ^&^& mkdir build ^&^& cd build ^&^& cmake .. -G "MinGW Makefiles" ^&^& mingw32-make
echo   4. Zip cgproject\ folder for submission
pause
