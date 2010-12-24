/************************************************************************
**
**  Copyright (C) 2010  Strahinja Markovic
**
**  This file is part of FlightCrew.
**
**  FlightCrew is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  FlightCrew is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with FlightCrew.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef FLIGHTCREW_H
#define FLIGHTCREW_H

#include <vector>
#include <string>
#include "DllExporting.h"
#include "Result.h"
#include "exception.h"

namespace FlightCrew
{

/**
 * Validates the provided epub file, running all the checks that FlightCrew 
 * can perform. This includes all the OPF, NCX, XHTML and CSS checks.
 *
 * @param filepath A UTF-8 encoded path to the epub file to validate.
 *                 The path can be either absolute or relative to the 
 *                 current working directory.
 * @return A vector of Results, sorted by internal file and then by line number.
 */
FC_WIN_DLL_API std::vector< Result > ValidateEpub(  const std::string &filepath );

/**
 * Validates the provided OPF file of an epub. The files that are listed in the
 * OPF are expected to exist.
 *
 * @param filepath A UTF-8 encoded path to the epub file to validate.
 *                 The path can be either absolute or relative to the 
 *                 current working directory.
 * @return A vector of Results, sorted by internal file and then by line number.
 */
FC_WIN_DLL_API std::vector< Result > ValidateOpf(   const std::string &filepath );

/**
 * Validates the provided NCX file of an epub. The files that are listed in the
 * NCX are expected to exist.
 *
 * @param filepath A UTF-8 encoded path to the epub file to validate.
 *                 The path can be either absolute or relative to the 
 *                 current working directory.
 * @return A vector of Results, sorted by internal file and then by line number.
 */
FC_WIN_DLL_API std::vector< Result > ValidateNcx(   const std::string &filepath );

/**
 * Validates the provided XHTML file of an epub. The resources linked from the file
 * are expected to exist.
 *
 * @param filepath A UTF-8 encoded path to the epub file to validate.
 *                 The path can be either absolute or relative to the 
 *                 current working directory.
 * @return A vector of Results, sorted by internal file and then by line number.
 */
FC_WIN_DLL_API std::vector< Result > ValidateXhtml( const std::string &filepath );

/**
 * Validates the provided CSS file of an epub. The resources linked from the file
 * are expected to exist. NOTE: this function is currently a stub. It doesn't really
 * do anything (yet).
 *
 * @param filepath A UTF-8 encoded path to the epub file to validate.
 *                 The path can be either absolute or relative to the 
 *                 current working directory.
 * @return A vector of Results, sorted by internal file and then by line number.
 */
FC_WIN_DLL_API std::vector< Result > ValidateCss(   const std::string &filepath );

} // namespace FlightCrew

#endif // FLIGHTCREW_H
