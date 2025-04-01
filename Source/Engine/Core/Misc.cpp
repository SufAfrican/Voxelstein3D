#include "StdAfx.h"

const Matrix44 Matrix44::mIdentity(1,0,0,0,
                                   0,1,0,0,
                                   0,0,1,0,
                                   0,0,0,1);

const Matrix44 Matrix44::mZero(0,0,0,0,
                               0,0,0,0,
                               0,0,0,0,
                               0,0,0,0);

const Quaternion Quaternion::qIdentity(0,0,0,1);