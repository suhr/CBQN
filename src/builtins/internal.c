#include "../core.h"
#include "../utils/mut.h"
#include "../utils/builtins.h"

B itype_c1(B t, B x) {
  B r;
  if(isVal(x)) {
    r = m_str8l(format_type(v(x)->type));
  } else {
    if      (isF64(x)) r = m_str32(U"tagged f64");
    else if (isC32(x)) r = m_str32(U"tagged c32");
    else if (isTag(x)) r = m_str32(U"tagged tag");
    else if (isVar(x)) r = m_str32(U"tagged var");
    else if (isExt(x)) r = m_str32(U"tagged extvar");
    else               r = m_str32(U"tagged unknown");
  }
  dec(x);
  return r;
}
B refc_c1(B t, B x) {
  B r = isVal(x)? m_i32(v(x)->refc) : m_str32(U"(not heap-allocated)");
  dec(x);
  return r;
}
B squeeze_c1(B t, B x) {
  if (!isArr(x)) return x;
  return bqn_squeeze(x);
}
B isPure_c1(B t, B x) {
  B r = m_f64(isPureFn(x));
  dec(x);
  return r;
}

B info_c2(B t, B w, B x) {
  B s = emptyCVec();
  i32 m = o2i(w);
  if (isVal(x)) {
    if (m) AFMT("%xl: ", x.u);
    else AFMT("%xl: ", x.u>>48);
    Value* xv = v(x);
    AFMT("refc:%i ", xv->refc);
    if (m) {
      AFMT("mmInfo:%i ", xv->mmInfo);
      AFMT("flags:%i ", xv->flags);
      AFMT("extra:%i ", xv->extra);
    }
    AFMT("type:%i=%S ", xv->type, format_type(xv->type));
    AFMT("alloc:%l", mm_size(xv));
    dec(x);
  } else {
    AFMT("%xl: ", x.u);
    A8("not heap-allocated");
  }
  return s;
}
B info_c1(B t, B x) {
  return info_c2(t, m_i32(0), x);
}

#define FOR_VARIATION(F) F(Ai32) F(Si32) F(Ai32Inc) F(Si32Inc) \
                         F(Af64) F(Sf64) F(Af64Inc) F(Sf64Inc) \
                         F(Ac32) F(Sc32) F(Ac32Inc) F(Sc32Inc) \
                         F(Ah)   F(Sh)   F(AhInc)   F(ShInc) \
                         F(Af)   F(Sf)   F(AfInc)   F(SfInc)

#define F(X) static B v_##X;
FOR_VARIATION(F)
#undef F
static B listVariations_def;

B listVariations_c2(B t, B w, B x) {
  if (!isArr(x)) thrM("•internal.ListVariations: 𝕩 must be an array");
  
  if (!isArr(w) || rnk(w)!=1) thrM("•internal.ListVariations: 𝕨 must be a list");
  usz wia = a(w)->ia;
  BS2B wgetU = TI(w,getU);
  bool c_incr=false, c_rmFill=false;
  for (usz i = 0; i < wia; i++) {
    u32 c = o2c(wgetU(w, i));
    if (c=='i') c_incr=true;
    else if (c=='f') c_rmFill=true;
    else thrF("internal.ListVariations: Unknown option '%c' in 𝕨", c);
  }
  dec(w);
  
  u8 xe = TI(x,elType);
  B xf = getFillQ(x);
  bool ah = c_rmFill || noFill(xf);
  bool ai32=false, af64=false, ac32=false;
  usz xia = a(x)->ia;
  BS2B xgetU = TI(x,getU);
  if (isNum(xf)) {
    ai32=af64=true;
    if (xe!=el_i32) {
      if (xe==el_f64) { f64* xp = f64any_ptr(x);
        for (usz i = 0; i < xia; i++) if (xp[i] != (i32)xp[i]) { ai32=false; break; }
      } else {
        for (usz i = 0; i < xia; i++) {
          B c = xgetU(x, i);
          if (!isNum(c)) { ai32=af64=false; break; }
          if (!q_i32(c)) ai32=false;
        }
      }
    }
  } else if (isC32(xf)) {
    ac32=true;
    if (xe!=el_c32) {
      for (usz i = 0; i < xia; i++) {
        B c = xgetU(x, i);
        if (!isC32(c)) { ac32=false; break; }
      }
    }
  } else ai32=af64=false;
  B r = emptyHVec();
  if(ai32) { r=vec_add(r,inc(v_Ai32)); r=vec_add(r,inc(v_Si32)); if(c_incr) {r=vec_add(r,inc(v_Ai32Inc)); r=vec_add(r,inc(v_Si32Inc)); } }
  if(af64) { r=vec_add(r,inc(v_Af64)); r=vec_add(r,inc(v_Sf64)); if(c_incr) {r=vec_add(r,inc(v_Af64Inc)); r=vec_add(r,inc(v_Sf64Inc)); } }
  if(ac32) { r=vec_add(r,inc(v_Ac32)); r=vec_add(r,inc(v_Sc32)); if(c_incr) {r=vec_add(r,inc(v_Ac32Inc)); r=vec_add(r,inc(v_Sc32Inc)); } }
  if(ah)   { r=vec_add(r,inc(v_Ah  )); r=vec_add(r,inc(v_Sh  )); if(c_incr) {r=vec_add(r,inc(v_AhInc  )); r=vec_add(r,inc(v_ShInc  )); } }
  {          r=vec_add(r,inc(v_Af  )); r=vec_add(r,inc(v_Sf  )); if(c_incr) {r=vec_add(r,inc(v_AfInc  )); r=vec_add(r,inc(v_SfInc  )); } }
  dec(x);
  dec(xf);
  return r;
}
B listVariations_c1(B t, B x) {
  return listVariations_c2(t, inc(listVariations_def), x);
}
static bool u32_get(u32** cv, u32* cE, u32* x) {
  u32* c = *cv;
  while (true) {
    if (!*x) {
      *cv = c;
      return true;
    }
    if (c==cE || *c!=*x) return false;
    c++; x++;
  }
  
}

static B variation_refs;
static void variation_root() {
  mm_visit(variation_refs);
  mm_visit(listVariations_def);
  #define F(X) mm_visit(v_##X);
  FOR_VARIATION(F)
  #undef F
}

B variation_c2(B t, B w, B x) {
  if (!isArr(x)) thrM("•internal.Variation: Non-array 𝕩");
  usz xia = a(x)->ia;
  u8 xe = TI(x,elType);
  BS2B xget = TI(x,get);
  BS2B xgetU = TI(x,getU);
  C32Arr* wc = toC32Arr(w);
  u32* wp = wc->a;
  u32* wpE = wp+wc->ia;
  if (wc->ia==0) thrM("•internal.Variation: Zero-length 𝕨");
  B res;
  if (*wp == 'A' || *wp == 'S') {
    bool slice = *wp == 'S';
    wp++;
    if (u32_get(&wp, wpE, U"i32")) {
      i32* tp; res = m_i32arrc(&tp, x);
      if (xe==el_i32) { i32* xp=i32any_ptr(x); for (usz i = 0; i < xia; i++) tp[i] = xp[i]; }
      else for (usz i = 0; i < xia; i++) tp[i] = o2i(xgetU(x,i));
    } else if (u32_get(&wp, wpE, U"f64")) {
      f64* tp; res = m_f64arrc(&tp, x);
      if      (xe==el_i32) { i32* xp=i32any_ptr(x); for (usz i = 0; i < xia; i++) tp[i] = xp[i]; }
      else if (xe==el_f64) { f64* xp=f64any_ptr(x); for (usz i = 0; i < xia; i++) tp[i] = xp[i]; }
      else for (usz i = 0; i < xia; i++) tp[i] = o2f(xgetU(x,i));
    } else if (u32_get(&wp, wpE, U"c32")) {
      u32* tp; res = m_c32arrc(&tp, x);
      if (xe==el_c32) { u32* xp=c32any_ptr(x); for (usz i = 0; i < xia; i++) tp[i] = xp[i]; }
      else for (usz i = 0; i < xia; i++) tp[i] = o2c(xgetU(x,i));
    } else if (u32_get(&wp, wpE, U"h")) {
      HArr_p t = m_harrUc(x);
      if      (xe==el_i32) { i32* xp=i32any_ptr(x); for (usz i = 0; i < xia; i++) t.a[i] = m_f64(xp[i]); }
      else if (xe==el_f64) { f64* xp=f64any_ptr(x); for (usz i = 0; i < xia; i++) t.a[i] = m_f64(xp[i]); }
      else for (usz i = 0; i < xia; i++) t.a[i] = xget(x,i);
      res = t.b;
    } else if (u32_get(&wp, wpE, U"f")) {
      Arr* t = m_fillarrp(xia);
      res = taga(t);
      fillarr_setFill(t, getFillQ(x));
      arr_shCopy(t, x);
      B* rp = fillarr_ptr(t);
      if      (xe==el_i32) { i32* xp=i32any_ptr(x); for (usz i = 0; i < xia; i++) rp[i] = m_f64(xp[i]); }
      else if (xe==el_f64) { f64* xp=f64any_ptr(x); for (usz i = 0; i < xia; i++) rp[i] = m_f64(xp[i]); }
      else for (usz i = 0; i < xia; i++) rp[i] = xget(x,i);
    } else thrF("•internal.Variation: Bad type \"%R\"", taga(wc));
    if (slice) {
      Arr* slice = TI(res,slice)(res, 0, a(res)->ia);
      arr_shCopy(slice, res);
      res = taga(slice);
    }
    if (u32_get(&wp, wpE, U"Inc")) {
      if (!variation_refs.u) {
        variation_refs = emptyHVec();
      }
      variation_refs = vec_add(variation_refs, inc(res));
    }
    if (wp!=wpE) thrM("•internal.Variation: Bad 𝕨");
  } else thrM("•internal.Variation: Bad start of 𝕨");
  dec(x);
  ptr_dec(wc);
  return res;
}

B clearRefs_c1(B t, B x) {
  dec(x);
  if (!isArr(variation_refs)) return m_f64(0);
  usz res = a(variation_refs)->ia;
  dec(variation_refs);
  variation_refs = m_f64(0);
  return m_f64(res);
}

static B unshare(B x) {
  if (!isArr(x)) return x;
  usz xia = a(x)->ia;
  switch (v(x)->type) {
    case t_i32arr: {
      i32* rp; B r = m_i32arrc(&rp, x);
      memcpy(rp, i32arr_ptr(x), xia*4);
      return r;
    }
    case t_c32arr: {
      u32* rp; B r = m_c32arrc(&rp, x);
      memcpy(rp, c32arr_ptr(x), xia*4);
      return r;
    }
    case t_f64arr: {
      f64* rp; B r = m_f64arrc(&rp, x);
      memcpy(rp, f64arr_ptr(x), xia*8);
      return r;
    }
    case t_harr: {
      HArr_p r = m_harrUc(x);
      B* xp = harr_ptr(x);
      for (usz i = 0; i < xia; i++) r.a[i] = unshare(xp[i]);
      return r.b;
    }
    case t_fillarr: {
      Arr* r = m_fillarrp(xia); arr_shCopy(r, x);
      fillarr_setFill(r, unshare(c(FillArr,x)->fill));
      B* rp = fillarr_ptr(r); B* xp = fillarr_ptr(a(x));
      for (usz i = 0; i < xia; i++) rp[i] = unshare(xp[i]);
      return taga(r);
    }
    default: thrF("•internal.Unshare: Cannot unshare array with type %i=%S", v(x)->type, format_type((v(x)->type)));
  }
}

B unshare_c1(B t, B x) {
  if (!isArr(x)) thrM("•internal.Unshare: Argument must be an array");
  B r = unshare(x);
  dec(x);
  return r;
}
static B internalNS;
B getInternalNS() {
  if (internalNS.u == 0) {
    #define F(X) v_##X = m_str8l(#X);
    FOR_VARIATION(F)
    #undef F
    listVariations_def = m_str32(U"if");
    gc_addFn(variation_root);
    #define F(X) inc(bi_##X),
    B fn = bqn_exec(m_str32(U"{⟨ Type,  Refc,  Squeeze,  IsPure,  Info,  ListVariations,  Variation,  ClearRefs,  Unshare⟩⇐𝕩}"), emptyCVec(), emptySVec());
    B arg =    m_caB(9, (B[]){F(itype)F(refc)F(squeeze)F(isPure)F(info)F(listVariations)F(variation)F(clearRefs)F(unshare)});
    #undef F
    internalNS = c1(fn,arg);
    gc_add(internalNS);
    dec(fn);
  }
  return inc(internalNS);
}
