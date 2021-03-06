
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pub_core_basics.h"
#include "pub_core_libcbase.h"
#include "pub_core_libcassert.h"
#include "pub_core_libcprint.h"

// I need this to avoid some signedness warnings, not sure why
// #define Char char
// jrs 19 Aug 2005: m_oset.c relies on Char being a signed char.
// It appears that plain 'char' on ppc32 is unsigned and so the
// above #define screws up the AVL tree balancing logic and
// leads to segfaults.  Commenting it out and using the standard
// definition of Char from pub_core_basics.h seems a good solution
// as that has the same signedness on all platforms.

// Crudely redirect various VG_(foo)() functions to their libc equivalents.
#undef vg_assert
#define vg_assert(e)                   assert(e)
#undef vg_assert2
#define vg_assert2(e, fmt, args...)    assert(e)

#define vgPlain_printf                 printf
#define vgPlain_memset                 memset
#define vgPlain_memcpy                 memcpy

#include "coregrind/m_oset.c"

#define NN  1000       // Size of OSets being created


/* Consistent random number generator, so it produces the
   same results on all platforms. */

#define random error_do_not_use_libc_random

static UInt seed = 0;
static UInt myrandom( void )
{
  seed = (1103515245 * seed + 12345);
  return seed;
}



//---------------------------------------------------------------------------
// Word example
//---------------------------------------------------------------------------

// This example shows that an element can be a single value (in this
// case a Word), in which case the element is also the key.

__attribute__((unused))
static Char *wordToStr(void *p)
{
   static char buf[32];
   sprintf(buf, "%ld", *(Word*)p);
   return buf;
}

__attribute__((unused))
static Word wordCmp(void* vkey, void* velem)
{
   return *(Word*)vkey - *(Word*)velem;
}

void example1(void)
{
   Int  i, n;
   Word v, prev;
   Word* vs[NN];
   Word *pv;

   // Create a static OSet of Ints.  This one uses fast (built-in)
   // comparisons.
   OSet* oset = VG_(OSetGen_Create)(0,
                                  NULL,
                                  (void*)malloc, free);

   // Try some operations on an empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetGen_Contains)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Lookup)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Remove)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( 0 == VG_(OSetGen_Size)(oset) );

   // Create some elements, with gaps (they're all even) but no dups,
   // and shuffle them randomly.
   for (i = 0; i < NN; i++) {
      vs[i] = VG_(OSetGen_AllocNode)(oset, sizeof(Word));
      *(vs[i]) = 2*i;
   }
   seed = 0;
   for (i = 0; i < NN; i++) {
      Word r1  = myrandom() % NN;
      Word r2  = myrandom() % NN;
      Word* tmp= vs[r1];
      vs[r1]   = vs[r2];
      vs[r2]   = tmp;
   }

   // Insert the elements
   for (i = 0; i < NN; i++) {
      VG_(OSetGen_Insert)(oset, vs[i]);
   }

   // Check the size
   vg_assert( NN == VG_(OSetGen_Size)(oset) );

   // Check we can find all the elements we inserted
   for (i = 0; i < NN; i++) {
      assert( VG_(OSetGen_Contains)(oset, vs[i]) );
   }

   // Check we cannot find elements we did not insert, below, within (odd
   // numbers), and above the inserted elements.
   v = -1;
   assert( ! VG_(OSetGen_Contains)(oset, &v) );
   for (i = 0; i < NN; i++) {
      v = *(vs[i]) + 1;
      assert( ! VG_(OSetGen_Contains)(oset, &v) );
   }
   v = NN*2;
   assert( ! VG_(OSetGen_Contains)(oset, &v) );

   // Check we can find all the elements we inserted, and the right values
   // are returned.
   for (i = 0; i < NN; i++) {
      assert( vs[i] == VG_(OSetGen_Lookup)(oset, vs[i]) );
   }

   // Check that we can iterate over the OSet elements in sorted order, and
   // there is the right number of them.
   n = 0;
   pv = NULL;
   prev = -1;
   VG_(OSetGen_ResetIter)(oset);
   while ( (pv = VG_(OSetGen_Next)(oset)) ) {
      Word curr = *pv;
      assert(prev < curr); 
      prev = curr;
      n++;
   }
   assert(NN == n);
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );

   // Check that we can remove half of the elements, and that their values
   // are as expected.
   for (i = 0; i < NN; i += 2) {
      assert( pv = VG_(OSetGen_Remove)(oset, vs[i]) );
      assert( pv == vs[i] );
   }

   // Check the size
   vg_assert( NN/2 == VG_(OSetGen_Size)(oset) );

   // Check we can find the remaining elements (with the right values).
   for (i = 1; i < NN; i += 2) {
      assert( pv = VG_(OSetGen_LookupWithCmp)(oset, vs[i], NULL) );
      assert( pv == vs[i] );
   }

   // Check we cannot find any of the elements we removed.
   for (i = 0; i < NN; i += 2) {
      assert( ! VG_(OSetGen_Contains)(oset, vs[i]) );
   }

   // Check that we can remove the remaining half of the elements, and that
   // their values are as expected.
   for (i = 1; i < NN; i += 2) {
      assert( pv = VG_(OSetGen_Remove)(oset, vs[i]) );
      assert( pv == vs[i] );
   }

   // Try some more operations on the empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetGen_Contains)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Lookup)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Remove)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( 0 == VG_(OSetGen_Size)(oset) );

   // Free a few elements
   VG_(OSetGen_FreeNode)(oset, vs[0]);
   VG_(OSetGen_FreeNode)(oset, vs[1]);
   VG_(OSetGen_FreeNode)(oset, vs[2]);

   // Re-insert remaining elements, to give OSetGen_Destroy something to
   // work with.
   for (i = 3; i < NN; i++) {
      VG_(OSetGen_Insert)(oset, vs[i]);
   }

   // Print the list
   OSet_Print(oset, "oset1", wordToStr);

   // Destroy the OSet
   VG_(OSetGen_Destroy)(oset);
}


void example1b(void)
{
   Int  i, n;
   Word v = 0, prev;
   Word vs[NN];
   Word *pv;

   // Create a static OSet of Ints.  This one uses fast (built-in)
   // comparisons.
   OSet* oset = VG_(OSetWord_Create)( (void*)malloc, free);

   // Try some operations on an empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetWord_Contains)(oset, v) );
   vg_assert( ! VG_(OSetWord_Remove)(oset, v) );
   vg_assert( ! VG_(OSetWord_Next)(oset, &v) );
   vg_assert( 0 == VG_(OSetWord_Size)(oset) );

   // Create some elements, with gaps (they're all even) but no dups,
   // and shuffle them randomly.
   for (i = 0; i < NN; i++) {
      vs[i] = 2*i;
   }
   seed = 0;
   for (i = 0; i < NN; i++) {
      Word r1  = myrandom() % NN;
      Word r2  = myrandom() % NN;
      Word tmp = vs[r1];
      vs[r1]   = vs[r2];
      vs[r2]   = tmp;
   }

   // Insert the elements
   for (i = 0; i < NN; i++) {
      VG_(OSetWord_Insert)(oset, vs[i]);
   }

   // Check the size
   vg_assert( NN == VG_(OSetWord_Size)(oset) );

   // Check we can find all the elements we inserted
   for (i = 0; i < NN; i++) {
      assert( VG_(OSetWord_Contains)(oset, vs[i]) );
   }

   // Check we cannot find elements we did not insert, below, within (odd
   // numbers), and above the inserted elements.
   v = -1;
   assert( ! VG_(OSetWord_Contains)(oset, v) );
   for (i = 0; i < NN; i++) {
      v = vs[i] + 1;
      assert( ! VG_(OSetWord_Contains)(oset, v) );
   }
   v = NN*2;
   assert( ! VG_(OSetWord_Contains)(oset, v) );

   // Check we can find all the elements we inserted.
   for (i = 0; i < NN; i++) {
      assert( VG_(OSetWord_Contains)(oset, vs[i]) );
   }

   // Check that we can iterate over the OSet elements in sorted order, and
   // there is the right number of them.
   n = 0;
   prev = -1;
   VG_(OSetWord_ResetIter)(oset);
   while ( VG_(OSetWord_Next)(oset, &v) ) {
      Word curr = v;
      assert(prev < curr); 
      prev = curr;
      n++;
   }
   assert(NN == n);
   vg_assert( ! VG_(OSetWord_Next)(oset, &v) );
   vg_assert( ! VG_(OSetWord_Next)(oset, &v) );

   // Check that we can remove half of the elements.
   for (i = 0; i < NN; i += 2) {
      assert( VG_(OSetWord_Remove)(oset, vs[i]) );
   }

   // Check the size
   vg_assert( NN/2 == VG_(OSetWord_Size)(oset) );

   // Check we can find the remaining elements (with the right values).
   for (i = 1; i < NN; i += 2) {
      assert( VG_(OSetWord_Contains)(oset, vs[i]) );
   }

   // Check we cannot find any of the elements we removed.
   for (i = 0; i < NN; i += 2) {
      assert( ! VG_(OSetWord_Contains)(oset, vs[i]) );
   }

   // Check that we can remove the remaining half of the elements.
   for (i = 1; i < NN; i += 2) {
      assert( VG_(OSetWord_Remove)(oset, vs[i]) );
   }

   // Try some more operations on the empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetWord_Contains)(oset, v) );
   vg_assert( ! VG_(OSetWord_Remove)(oset, v) );
   vg_assert( ! VG_(OSetWord_Next)(oset, &v) );
   vg_assert( 0 == VG_(OSetWord_Size)(oset) );

   // Re-insert remaining elements, to give OSetWord_Destroy something to
   // work with.
   for (i = 3; i < NN; i++) {
      VG_(OSetWord_Insert)(oset, vs[i]);
   }

   // Print the list
   OSet_Print(oset, "oset1b", wordToStr);

   // Destroy the OSet
   VG_(OSetWord_Destroy)(oset);
}


//---------------------------------------------------------------------------
// Struct example
//---------------------------------------------------------------------------

// This element shows that a key can be in the middle of the element, and
// be of arbitrary size and even span multiple (contiguous) fields.  It
// also demonstrates how an OSet can be used to implement a list of
// non-overlapping intervals.

typedef struct {
   Int   b1;
   Addr  first;
   Addr  last;
   Int   b2;
}
Block;

__attribute__((unused))
static Char *blockToStr(void *p)
{
   static char buf[32];
   Block* b = (Block*)p;
   sprintf(buf, "<(%d) %lu..%lu (%d)>", b->b1, b->first, b->last, b->b2);
   return buf;
}

static Word blockCmp(void* vkey, void* velem)
{
   Addr   key  = *(Addr*)vkey;
   Block* elem = (Block*)velem;

   assert(elem->first <= elem->last);
   if (key < elem->first) return -1;
   if (key > elem->last)  return  1;
   return 0;
}

void example2(void)
{
   Int i, n;
   Addr a;
   Block* vs[NN];
   Block v, prev;
   Block *pv;

   // Create a dynamic OSet of Blocks.  This one uses slow (custom)
   // comparisons.
   OSet* oset = VG_(OSetGen_Create)(offsetof(Block, first),
                                  blockCmp,
                                  (void*)malloc, free);

   // Try some operations on an empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetGen_Contains)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Lookup)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Remove)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( 0 == VG_(OSetGen_Size)(oset) );

   // Create some inputs, with gaps -- intervals are 1..3, 11..13, ... -- but
   // no dups, and shuffle them randomly.
   for (i = 0; i < NN; i++) {
      vs[i] = VG_(OSetGen_AllocNode)(oset, sizeof(Block));
      vs[i]->b1    = i;
      vs[i]->first = i*10 + 1;
      vs[i]->last  = vs[i]->first + 2;
      vs[i]->b2    = i+1;
   }
   seed = 0;
   for (i = 0; i < NN; i++) {
      Int r1  = myrandom() % NN;
      Int r2  = myrandom() % NN;
      Block* tmp = vs[r1];
      vs[r1]  = vs[r2];
      vs[r2]  = tmp;
   }

   // Insert the elements
   for (i = 0; i < NN; i++) {
      VG_(OSetGen_Insert)(oset, vs[i]);
   }

   // Check the size
   vg_assert( NN == VG_(OSetGen_Size)(oset) );

   // Check we can find all the elements we inserted, within the full range
   // of each Block.
   for (i = 0; i < NN; i++) {
      a = vs[i]->first + 0;    assert( VG_(OSetGen_Contains)(oset, &a) );
      a = vs[i]->first + 1;    assert( VG_(OSetGen_Contains)(oset, &a) );
      a = vs[i]->first + 2;    assert( VG_(OSetGen_Contains)(oset, &a) );
   }

   // Check we cannot find elements we did not insert, below and above the
   // ranges of the inserted elements.
   a = 0;
   assert( ! VG_(OSetGen_Contains)(oset, &a) );
   for (i = 0; i < NN; i++) {
      a = vs[i]->first - 1;    assert( ! VG_(OSetGen_Contains)(oset, &a) );
      a = vs[i]->first + 3;    assert( ! VG_(OSetGen_Contains)(oset, &a) );
   }

   // Check we can find all the elements we inserted, and the right values
   // are returned.
   for (i = 0; i < NN; i++) {
      a = vs[i]->first + 0;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
      a = vs[i]->first + 1;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
      a = vs[i]->first + 2;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
      assert( vs[i] == VG_(OSetGen_LookupWithCmp)(oset, &a, blockCmp) );
   }

   // Check that we can iterate over the OSet elements in sorted order, and
   // there is the right number of them.
   n = 0;
   pv = NULL;
   prev.last = 0;
   VG_(OSetGen_ResetIter)(oset);
   while ( (pv = VG_(OSetGen_Next)(oset)) ) {
      Block curr = *pv;
      assert(prev.last < curr.first); 
      prev = curr;
      n++;
   }
   assert(NN == n);
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );

   // Check that we can remove half of the elements, and that their values
   // are as expected.
   for (i = 0; i < NN; i += 2) {
      a = vs[i]->first;    assert( vs[i] == VG_(OSetGen_Remove)(oset, &a) );
   }

   // Check the size
   vg_assert( NN/2 == VG_(OSetGen_Size)(oset) );

   // Check we can find the remaining elements (with the right values).
   for (i = 1; i < NN; i += 2) {
      a = vs[i]->first + 0;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
      a = vs[i]->first + 1;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
      a = vs[i]->first + 2;    assert( vs[i] == VG_(OSetGen_Lookup)(oset, &a) );
   }

   // Check we cannot find any of the elements we removed.
   for (i = 0; i < NN; i += 2) {
      a = vs[i]->first + 0;    assert( ! VG_(OSetGen_Contains)(oset, &a) );
      a = vs[i]->first + 1;    assert( ! VG_(OSetGen_Contains)(oset, &a) );
      a = vs[i]->first + 2;    assert( ! VG_(OSetGen_Contains)(oset, &a) );
   }

   // Check that we can remove the remaining half of the elements, and that
   // their values are as expected.
   for (i = 1; i < NN; i += 2) {
      a = vs[i]->first;    assert( vs[i] == VG_(OSetGen_Remove)(oset, &a) );
   }

   // Try some more operations on the empty OSet to ensure they don't screw up.
   vg_assert( ! VG_(OSetGen_Contains)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Lookup)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Remove)(oset, &v) );
   vg_assert( ! VG_(OSetGen_Next)(oset) );
   vg_assert( 0 == VG_(OSetGen_Size)(oset) );

   // Re-insert all elements, to give OSetGen_Destroy something to work with.
   for (i = 0; i < NN; i++) {
      VG_(OSetGen_Insert)(oset, vs[i]);
   }

   // Destroy the OSet
   VG_(OSetGen_Destroy)(oset);
}

//-----------------------------------------------------------------------
// main()
//-----------------------------------------------------------------------

int main(void)
{
   example1();
   example1b();
   example2();
   return 0;
}
