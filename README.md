ls-config
=========

Simple program to use libconfig9 configuration files in bash scripts

You can user libcongig( files directly invoking ls-config, or in bash script
by sourcing lslib-core and then usinig simpe cfg_* functions. In this case 
You must set LS_EXEC to 1 before sourcing lslib-core

Caution! in curentrly version buid-in doc are in Polish translatnion. 
To get working englich, You must manualy make and install english 
locales (neeted *.po translations are included. Call make, and then copy 
builded *.mo files do destination depending on You system).

We need some help to build packaging for direff systesm.

