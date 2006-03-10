#
#    makefile for build everything in this folder by GCC.
#    Copyright (c) Sergey Shishmintzev, Kiev 2005-2006
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    Contributor(s):
#      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>

CXX = g++ -O3 -mno-cygwin -Wl,-s
CC = gcc -O3 -mno-cygwin -Wl,-s

mju-sed.exe: mju-sed.cpp
	$(CXX) -o $@ mju-sed.cpp 

lemon.exe: lemon.c
	$(CC) -o $@ lemon.c


clean:
	-$(RM) mju-sed.exe
	-$(RM) lemon.exe
