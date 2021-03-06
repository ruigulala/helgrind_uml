/*
   This file is part of Callgrind, a Valgrind tool for call graph
   profiling programs.

   Copyright (C) 2002-2007, Josef Weidendorfer (Josef.Weidendorfer@gmx.de)

   This tool is derived from and contains lot of code from Cachegrind
   Copyright (C) 2002 Nicholas Nethercote (njn@valgrind.org)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "global.h"
#include "events.h"

/* If debugging mode of, dummy functions are provided (see below)
 */
#if CLG_ENABLE_DEBUG

/*------------------------------------------------------------*/
/*--- Debug output helpers                                 ---*/
/*------------------------------------------------------------*/

static void print_indent(int s)
{
    /* max of 40 spaces */
    char sp[] = "                                        ";
    if (s>40) s=40;
    VG_(printf)(sp+40-s);
}

void CLG_(print_bb)(int s, BB* bb)
{
    if (s<0) {
	s = -s;
	print_indent(s);
    }

    VG_(printf)("BB %p (Obj '%s')", bb_addr(bb), bb->obj->name);
}

static
void print_mangled_cxt(Context* cxt, int rec_index)
{
    int i;

    if (!cxt)
      VG_(printf)("(none)");
    else {
      VG_(printf)("%s", cxt->fn[0]->name);
      if (rec_index >0)
	VG_(printf)("'%d", rec_index +1);
      for(i=1;i<cxt->size;i++)
	VG_(printf)("'%s", cxt->fn[i]->name);
    }
}



void CLG_(print_cxt)(int s, Context* cxt, int rec_index)
{
  if (s<0) {
    s = -s;
    print_indent(s);
  }
  
  if (cxt) {
    UInt *pactive = CLG_(get_fn_entry)(cxt->fn[0]->number);
    CLG_ASSERT(rec_index < cxt->fn[0]->separate_recursions);
    
    VG_(printf)("Cxt %d" ,cxt->base_number + rec_index);
    if (*pactive>0)
      VG_(printf)(" [active=%d]", *pactive);
    VG_(printf)(": ");	
    print_mangled_cxt(cxt, rec_index);
    VG_(printf)("\n");
  }
  else
    VG_(printf)("(no context)\n");
}

void CLG_(print_execstate)(int s, exec_state* es)
{
  if (s<0) {
    s = -s;
    print_indent(s);
  }
  
  if (!es) {
    VG_(printf)("ExecState 0x0\n");
    return;
  }

  VG_(printf)("ExecState [Sig %d, collect %s, nonskipped %p]: jmps_passed %d\n",
	      es->sig, es->collect?"yes":"no",
	      es->nonskipped, es->jmps_passed);
}


void CLG_(print_bbcc)(int s, BBCC* bbcc, Bool jumpaddr)
{
  BB* bb;

  if (s<0) {
    s = -s;
    print_indent(s);
  }
  
  if (!bbcc) {
    VG_(printf)("BBCC 0x0\n");
    return;
  }
 
  bb = bbcc->bb;
  CLG_ASSERT(bb!=0);

#if 0 
  if (jumpaddr)
    VG_(printf)("%s +%p=%p, ",
	      bb->obj->name + bb->obj->last_slash_pos,
	      bb->jmp_offset, bb_jmpaddr(bb));
  else
#endif
    VG_(printf)("%s +%p=%p, ",
		bb->obj->name + bb->obj->last_slash_pos,
		bb->offset, bb_addr(bb));
  CLG_(print_cxt)(s+8, bbcc->cxt, bbcc->rec_index);
}

void CLG_(print_eventset)(int s, EventSet* es)
{
  int i;

  if (s<0) {
    s = -s;
    print_indent(s);
  }

  if (!es) {
    VG_(printf)("(EventSet not set)\n");
    return;
  }

  VG_(printf)("%5s (Size/Cap %d/%d): ",
	      es->name, es->size, es->capacity);

  if (es->size == 0)
    VG_(printf)("-");
  else {
    for(i=0; i< es->size; i++) {
      if (i>0) {
	VG_(printf)(" ");
	if (es->e[i-1].nextTop == i)
	  VG_(printf)("| ");
      }
      VG_(printf)(es->e[i].type->name);
    }
  }
  VG_(printf)("\n");
}


void CLG_(print_cost)(int s, EventSet* es, ULong* c)
{
  Int i, pos;

    if (s<0) {
	s = -s;
	print_indent(s);
    }

    if (!es) {
      VG_(printf)("Cost (Nothing, EventSet not set)\n");
      return;
    }
    if (!c) {
      VG_(printf)("Cost (Null, EventSet %s)\n", es->name);
      return;
    }

    if (es->size == 0) {
      VG_(printf)("Cost (Nothing, EventSet %s with len 0)\n", es->name);
      return;
    } 

    pos = s;
    pos += VG_(printf)("Cost %s [%p]: %s %llu", es->name, c, es->e[0].type->name, c[0]);

    i = 1;
    while(i<es->size) {
      if (pos > 70) {
	VG_(printf)(",\n");
	print_indent(s+5);
	pos = s+5;
      }
      else
	pos += VG_(printf)(", ");
      pos += VG_(printf)("%s %llu", es->e[i].type->name, c[i]);
      i++;
    }
    VG_(printf)("\n");
}


void CLG_(print_short_jcc)(jCC* jcc)
{
    if (jcc)
	VG_(printf)("%p => %p [%llu/%llu,%llu,%llu]",
		    bb_jmpaddr(jcc->from->bb),
		    bb_addr(jcc->to->bb),
		    jcc->call_counter,
		    jcc->cost ? jcc->cost[CLG_(sets).off_sim_Ir]:0,
		    jcc->cost ? jcc->cost[CLG_(sets).off_sim_Dr]:0,
		    jcc->cost ? jcc->cost[CLG_(sets).off_sim_Dw]:0);
    else
	VG_(printf)("[Skipped JCC]");
}

void CLG_(print_jcc)(int s, jCC* jcc)
{
    if (s<0) {
	s = -s;
	print_indent(s);
    }

    if (!jcc) {
	VG_(printf)("JCC to skipped function\n");
	return;
    }
    VG_(printf)("JCC %p from ", jcc);
    CLG_(print_bbcc)(s+9, jcc->from, True);
    print_indent(s+4);    
    VG_(printf)("to   ");
    CLG_(print_bbcc)(s+9, jcc->to, False);
    print_indent(s+4);
    VG_(printf)("Calls %llu\n", jcc->call_counter);
    print_indent(s+4);
    CLG_(print_cost)(s+9, CLG_(sets).full, jcc->cost);
}

/* dump out the current call stack */
void CLG_(print_stackentry)(int s, int sp)
{
    call_entry* ce;

    if (s<0) {
	s = -s;
	print_indent(s);
    }

    ce = CLG_(get_call_entry)(sp);
    VG_(printf)("[%-2d] SP %p, RA %p", sp, ce->sp, ce->ret_addr);
    if (ce->nonskipped)
	VG_(printf)(" NonSkipped BB %p / %s",
		    bb_addr(ce->nonskipped->bb),
		    ce->nonskipped->cxt->fn[0]->name);
    VG_(printf)("\n");
    print_indent(s+5);
    CLG_(print_jcc)(5,ce->jcc);
}

/* debug output */
#if 0
static void print_call_stack()
{
    int c;

    VG_(printf)("Call Stack:\n");
    for(c=0;c<CLG_(current_call_stack).sp;c++)
      CLG_(print_stackentry)(-2, c);
}
#endif

void CLG_(print_bbcc_fn)(BBCC* bbcc)
{
    obj_node* obj;

    if (!bbcc) {
	VG_(printf)("%08x", 0);
	return;
    }

    VG_(printf)("%08lx/%c  %d:", bb_addr(bbcc->bb), 
		(bbcc->bb->sect_kind == Vg_SectText) ? 'T' :
		(bbcc->bb->sect_kind == Vg_SectData) ? 'D' :
		(bbcc->bb->sect_kind == Vg_SectBSS) ? 'B' :
		(bbcc->bb->sect_kind == Vg_SectGOT) ? 'G' :
		(bbcc->bb->sect_kind == Vg_SectPLT) ? 'P' : 'U',
		bbcc->cxt->base_number+bbcc->rec_index);
    print_mangled_cxt(bbcc->cxt, bbcc->rec_index);

    obj = bbcc->cxt->fn[0]->file->obj;
    if (obj->name[0])
	VG_(printf)(" %s", obj->name+obj->last_slash_pos);

    if (VG_(strcmp)(bbcc->cxt->fn[0]->file->name, "???") !=0) {
	VG_(printf)(" %s", bbcc->cxt->fn[0]->file->name);
	if ((bbcc->cxt->fn[0] == bbcc->bb->fn) && (bbcc->bb->line>0))
	    VG_(printf)(":%d", bbcc->bb->line);
    }
}	

void CLG_(print_bbcc_cost)(int s, BBCC* bbcc)
{
  BB* bb;
  Int i, cjmpNo;
  ULong ecounter;

  if (s<0) {
    s = -s;
    print_indent(s);
  }
  
  if (!bbcc) {
    VG_(printf)("BBCC 0x0\n");
    return;
  }
 
  bb = bbcc->bb;
  CLG_ASSERT(bb!=0);
    
  CLG_(print_bbcc)(s, bbcc, False);

  ecounter = bbcc->ecounter_sum;

  print_indent(s+2);
  VG_(printf)("ECounter: sum %llu ", ecounter);
  for(i=0; i<bb->cjmp_count; i++) {
      VG_(printf)("[%d]=%llu ",
		  bb->jmp[i].instr, bbcc->jmp[i].ecounter);
  }
  VG_(printf)("\n");

  cjmpNo = 0; 
  for(i=0; i<bb->instr_count; i++) {
      InstrInfo* ii = &(bb->instr[i]);
      print_indent(s+2);
      VG_(printf)("[%2d] IOff %2d ecnt %3llu ",
		  i, ii->instr_offset, ecounter);
      CLG_(print_cost)(s+5, ii->eventset, bbcc->cost + ii->cost_offset);

      /* update execution counter */
      if (cjmpNo < bb->cjmp_count)
	  if (bb->jmp[cjmpNo].instr == i) {
	      ecounter -= bbcc->jmp[cjmpNo].ecounter;
	      cjmpNo++;
	  }
  }
}


/* dump out an address with source info if available */
void CLG_(print_addr)(Addr addr)
{
    Char fl_buf[FILENAME_LEN];
    Char fn_buf[FN_NAME_LEN];
    const UChar* obj_name;
    SegInfo* si;
    int ln, i=0, opos=0;
	
    if (addr == 0) {
	VG_(printf)("%08lx", addr);
	return;
    }

    CLG_(get_debug_info)(addr, fl_buf, fn_buf, &ln, &si);

    if (VG_(strcmp)(fn_buf,"???")==0)
	VG_(printf)("%p", addr);
    else
	VG_(printf)("%p %s", addr, fn_buf);

    if (si) {
      obj_name = VG_(seginfo_filename)(si);
      if (obj_name) {
	while(obj_name[i]) {
	  if (obj_name[i]=='/') opos = i+1;
	  i++;
	}
	if (obj_name[0])
	  VG_(printf)(" %s", obj_name+opos);
      }
    }

    if (ln>0)
    	VG_(printf)(" (%s:%u)", fl_buf,ln);
}

void CLG_(print_addr_ln)(Addr addr)
{
  CLG_(print_addr)(addr);
  VG_(printf)("\n");
}

static ULong bb_written = 0;

void CLG_(print_bbno)(void)
{
  if (bb_written != CLG_(stat).bb_executions) {
    bb_written = CLG_(stat).bb_executions;
    VG_(printf)("BB# %llu\n",CLG_(stat).bb_executions);
  }
}

void CLG_(print_context)(void)
{
  BBCC* bbcc;

  CLG_DEBUG(0,"In tid %d [%d] ",
	   CLG_(current_tid),  CLG_(current_call_stack).sp);
  bbcc =  CLG_(current_state).bbcc;
  print_mangled_cxt(CLG_(current_state).cxt,
		    bbcc ? bbcc->rec_index : 0);
  VG_(printf)("\n");
}

void* CLG_(malloc)(UWord s, char* f)
{
    CLG_DEBUG(3, "Malloc(%lu) in %s.\n", s, f);
    return VG_(malloc)(s);
}

#else /* CLG_ENABLE_DEBUG */

void CLG_(print_bbno)(void) {}
void CLG_(print_context)(void) {}
void CLG_(print_jcc)(int s, jCC* jcc) {}
void CLG_(print_bbcc)(int s, BBCC* bbcc, Bool b) {}
void CLG_(print_bbcc_fn)(BBCC* bbcc) {}
void CLG_(print_cost)(int s, EventSet* es, ULong* cost) {}
void CLG_(print_bb)(int s, BB* bb) {}
void CLG_(print_cxt)(int s, Context* cxt, int rec_index) {}
void CLG_(print_short_jcc)(jCC* jcc) {}
void CLG_(print_stackentry)(int s, int sp) {}
void CLG_(print_addr)(Addr addr) {}
void CLG_(print_addr_ln)(Addr addr) {}

#endif
