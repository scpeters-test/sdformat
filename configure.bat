@set build_type=Release
@if not "%1"=="" set build_type=%1
@echo Configuring for build type %build_type%

REM looking for ruby executable. Don't look at me, the for loop is the to get 
REM command output in windows 
for /f %%i in ('where ruby') do set RUBY_PATH=%%i

cmake -G "NMake Makefiles" -DBOOST_ROOT:STRING="..\boost_1_56_0" -DBOOST_LIBRARYDIR:STRING="..\boost_1_56_0\lib64-msvc-12.0" -DRUBY_EXECUTABLE=%RUBY_PATH% -DCMAKE_INSTALL_PREFIX="install/%build_type%" -DCMAKE_BUILD_TYPE="%build_type%" .."
