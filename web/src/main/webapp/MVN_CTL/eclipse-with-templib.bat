@echo off
setlocal
cd /D %0\..\..
call mvn qq-thirdparty:maven-eclipse-plugin:clean
call mvn qq-thirdparty:maven-eclipse-plugin:eclipse
call mvn qq-central:maven-eclipse-classpath-plugin:add_templib
pause
