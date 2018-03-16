@echo off
setlocal
cd /D %0\..\..
cls
chdir
call mvn qq-central:maven-taf-plugin:1.0.0-SNAPSHOT:jce2java -f jce.pom.xml
pause
