// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corporation 1998, 2024. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

int n = ...;
range N = 1..n;

int    v_int = ...;
float  v_float = ...;
string v_string = ...;

int a_int[N]       = ...;
float a_float[N]   = ...;
string a_string[N] = ...;

{int}    s_int    = ...;
{float}  s_float  = ...;
{string} s_string = ...;

tuple T {
  int    i;
  float  f;
  string s;
}

T v_tuple    = ...;
T a_tuple[N] = ...;
{T} s_tuple  = ...;

tuple S {
  float f;
  int   i;
}
tuple Nested {
  string s;
  S      sub;
}
Nested v_nested    = ...;
Nested a_nested[N] = ...;
{Nested} s_nested  = ...;

minimize 0; subject to {}
