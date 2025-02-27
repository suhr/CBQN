#include "../core.h"

static Arr* m_f64slice(Arr* p, f64* ptr, usz ia) {
  F64Slice* r = m_arr(sizeof(F64Slice), t_f64slice, ia);
  r->p = p;
  r->a = ptr;
  return (Arr*)r;
}
static Arr* f64arr_slice  (B x, usz s, usz ia) { return m_f64slice(a(x), c(F64Arr,x)->a+s, ia); }
static Arr* f64slice_slice(B x, usz s, usz ia) { Arr* p=c(Slice,x)->p; ptr_inc(p); Arr* r = m_f64slice(p, c(F64Slice,x)->a+s, ia); dec(x); return r; }

static B f64arr_get  (B x, usz n) { VTY(x,t_f64arr  ); return m_f64(c(F64Arr  ,x)->a[n]); }
static B f64slice_get(B x, usz n) { VTY(x,t_f64slice); return m_f64(c(F64Slice,x)->a[n]); }
static bool f64arr_canStore(B x) { return q_f64(x); }

void f64arr_init() {
  TIi(t_f64arr,get)   = f64arr_get;   TIi(t_f64slice,get)   = f64slice_get;
  TIi(t_f64arr,getU)  = f64arr_get;   TIi(t_f64slice,getU)  = f64slice_get;
  TIi(t_f64arr,slice) = f64arr_slice; TIi(t_f64slice,slice) = f64slice_slice;
  TIi(t_f64arr,freeO) = tyarr_freeO;  TIi(t_f64slice,freeO) =    slice_freeO;
  TIi(t_f64arr,freeF) = tyarr_freeF;  TIi(t_f64slice,freeF) =    slice_freeF;
  TIi(t_f64arr,visit) = noop_visit;   TIi(t_f64slice,visit) =    slice_visit;
  TIi(t_f64arr,print) =    arr_print; TIi(t_f64slice,print) = arr_print;
  TIi(t_f64arr,isArr) = true;         TIi(t_f64slice,isArr) = true;
  TIi(t_f64arr,arrD1) = true;         TIi(t_f64slice,arrD1) = true;
  TIi(t_f64arr,elType) = el_f64;      TIi(t_f64slice,elType) = el_f64;
  TIi(t_f64arr,canStore) = f64arr_canStore;
}

NOINLINE B m_caf64(usz sz, f64* a) {
  f64* rp; B r = m_f64arrv(&rp, sz);
  for (usz i = 0; i < sz; i++) rp[i] = a[i];
  return r;
}
