/* latte_relocatable.cpp -- Support relocatable installations
	       
   Copyright 2008 Matthias Koeppe

   This file is part of LattE.
   
   LattE is free software; you can redistribute it and/or modify it
   under the terms of the version 2 of the GNU General Public License
   as published by the Free Software Foundation.

   LattE is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LattE; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include "config.h"
#include "latte_relocatable.h"
#include "gnulib/relocatable.h"
#include <cstdlib>

std::string relocated_pathname(const char *pathname)
{
  const char *relocated = relocate(pathname);
  std::string result = relocated;
  if (relocated != pathname) {
    // It was malloced, so get rid of it.
    free((char*) relocated);
  }
  return result;
}

std::string relocated_pathname(const std::string &pathname)
{
  return relocated_pathname(pathname.c_str());
}

  
