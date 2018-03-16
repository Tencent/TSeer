@echo off
setlocal
cd /D %0\..\..
call mvn org.apache.maven.plugins:maven-dependency-plugin:2.1:copy-dependencies -DoutputDirectory=WEB-INF/lib/  -DexcludeScope=provided
pause
