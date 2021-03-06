/*
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2004, 2007, 2008 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "InternalFunction.h"

#include "FunctionPrototype.h"
#include "JSString.h"

namespace JSC {

ASSERT_CLASS_FITS_IN_CELL(InternalFunction);

const ClassInfo InternalFunction::info = { "Function", 0, 0, 0 };

InternalFunction::InternalFunction(ExecState* exec)
    : JSObject(exec->globalData().nullProtoStructureID)
{
    putDirect(exec->propertyNames().name, jsString(exec, exec->propertyNames().nullIdentifier.ustring()), DontDelete | ReadOnly | DontEnum);
}

InternalFunction::InternalFunction(ExecState* exec, FunctionPrototype* prototype, const Identifier& name)
    : JSObject(prototype)
{
    putDirect(exec->propertyNames().name, jsString(exec, name.ustring()), DontDelete | ReadOnly | DontEnum);
}

const UString& InternalFunction::name(ExecState* exec)
{
    JSValue* v = getDirect(exec->propertyNames().name);
    ASSERT(v->isString());
    return static_cast<JSString*>(v)->value();
}

bool InternalFunction::implementsHasInstance() const
{
    return true;
}

} // namespace JSC
