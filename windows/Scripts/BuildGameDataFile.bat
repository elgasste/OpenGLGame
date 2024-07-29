@echo off

echo ==================================================
echo               GAME DATA BUILDER 5000
echo ==================================================
echo:

:: asset compilation
:: TODO: maybe try to build these projects in here automatically before using them?

echo Compiling assets...
echo:

if exist "..\Tools\AssetCompiler\x64\Release\AssetCompiler.exe" (
	echo Using release version of asset compiler.
	pushd "..\Tools\AssetCompiler\x64\Release\"
) else if exist "..\Tools\AssetCompiler\x64\Debug\AssetCompiler.exe" (
	echo Using debug version of asset compiler.
	pushd "..\Tools\AssetCompiler\x64\Debug\"
) else (
	echo ERROR: Asset compiler not found! Make sure the AssetCompiler project has been built before running this script.
	echo:
	goto :end
)

AssetCompiler.exe ..\..\..\..\..\assets\
if ERRORLEVEL 1 goto :cleanup

echo Success!
echo:

:cleanup

popd

:end

set /p=Press the Enter key to exit
