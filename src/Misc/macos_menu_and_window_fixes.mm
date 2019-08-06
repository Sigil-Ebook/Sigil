/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario Canada
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

/* ============================================================
* Portions from QupZilla - Qt web browser
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
* Copyright (C) 2017 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
*
* GPL 3
* ============================================================ */

/*******************************************************************************

Portions from OpenCor
Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

*******************************************************************************/

#import <AppKit/AppKit.h>
#import <AppKit/NSWindow.h>
#import <Availability.h>

// code taken from: https://www.mail-archive.com/interest@qt-project.org/msg23593.html
// Disables auto window tabbing where supported, otherwise a no-op.

void disableWindowTabbing()
{
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
    if ([NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)]) {
        NSWindow.allowsAutomaticWindowTabbing = NO;
    }
#endif
}

void removeMacosSpecificMenuItems()
{
    // Remove (disable) the "Start Dictation..." and "Emoji & Symbols" menu
    // items from the "Edit" menu

    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledCharacterPaletteMenuItem"];

    // Remove (don't have) the "Enter Full Screen" menu item from the "View"
    // menu

    [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSFullScreenMenuItemEverywhere"];
}

