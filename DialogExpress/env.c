/*
    Environment for dialog resource compiler.
    Copyright (C) 2006 Sergey Shishmintzev

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contributor(s):
      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
*/

#include <assert.h>
#include <farcolor.hpp>
#include "dialogresInt.h"

struct EnvVar {
  Token sName;
  unsigned nValue;
};
#include "env.h"
#include "envcol.h"

void dialogresInitEnvironment(Parse *pParse)
{
  const struct EnvVar *pBind;
  assert(pParse);
  for (pBind = aEnvVars; pBind->sName.z; pBind++)
    AddIntBind(pParse, pBind->sName, pBind->nValue);
}

int dialogresLookupColorId(Parse *pParse, Token sId)
{
  const struct EnvVar *pBind;
  assert(sId.z);
  assert(sId.n);
  for (pBind = aEnvColors; pBind->sName.z; pBind++)
    if (pBind->sName.n == sId.n && strncmp(pBind->sName.z, sId.z, sId.n)==0 )
      return pBind->nValue;
  pParse->rc = dialogres_UnknownIdentifier;
  pParse->sErrToken = sId;
  return 0;
}