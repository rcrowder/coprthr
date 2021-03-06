/* event.c
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */

#include <string.h>
#include <errno.h>

#include "printcl.h"
#include "event.h"
#include "device.h"
#include "command_queue.h"

#include "coprthr_sched.h"
#include "coprthr_mem.h"

#define __malloc_struct(t) (struct t*)malloc(sizeof(struct t))

void __do_release_event_1(struct coprthr_event* ev1) 
{
	/* XXX may need to check state of event for controlled release -DAR */

	/* XXX this assumes the ev is on cmds_complete, check this first! -DAR */

	printcl( CL_DEBUG "__do_release_event: attempt lock cmdq");
	__lock_cmdq1(ev1->dev->devstate->cmdq);
	printcl( CL_DEBUG "__do_release_event: locked cmdq");
	printcl( CL_DEBUG "__do_release_event: remove ev from cmdq");
	printcl( CL_DEBUG "__do_release_event: dev %p\n",ev1->dev);
	TAILQ_REMOVE(&(ev1->dev->devstate->cmdq->cmds_complete),ev1,cmds);
	printcl( CL_DEBUG "__do_release_event: removed from cmdq");
	__unlock_cmdq1(ev1->dev->devstate->cmdq);
	ev1->dev = 0;
	printcl( CL_DEBUG "__do_release_event_1: success");
}

#define __copy3(dst,src) do { \
	(dst)[0]=(src)[0]; (dst)[1]=(src)[1]; (dst)[2]=(src)[2]; \
	} while(0)


void __do_set_cmd_read_buffer_1( 
	struct coprthr_event* ev1,
	struct coprthr_mem* src1, size_t src_offset, size_t len, 
	void* dst
)
{
	printcl( CL_DEBUG "__do_set_cmd_read_buffer_1: src1=%p",src1);

	ev1->cmd = __CL_COMMAND_READ_BUFFER;

	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = dst;
	ev1->cmd_argp->m.src = (void*)src1;
	ev1->cmd_argp->m.src_offset = src_offset;
	ev1->cmd_argp->m.len = len;
}


void __do_set_cmd_write_buffer_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* dst1, size_t dst_offset, size_t len, 
	const void* src
)
{
	ev1->cmd = __CL_COMMAND_WRITE_BUFFER;

	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src;
	ev1->cmd_argp->m.dst_offset = dst_offset;
	ev1->cmd_argp->m.len = len;
}


void __do_set_cmd_copy_buffer_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* src1, struct coprthr_mem* dst1,
	size_t src_offset, size_t dst_offset, size_t len 
)
{
	ev1->cmd = __CL_COMMAND_COPY_BUFFER;

	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src1;
	ev1->cmd_argp->m.dst_offset = dst_offset;
	ev1->cmd_argp->m.src_offset = src_offset;
	ev1->cmd_argp->m.len = len;
}


void __do_set_cmd_read_image_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* src1, 
	const size_t* src_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	void* dst
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = dst;
	ev1->cmd_argp->m.src = (void*)src1;

	if (src_origin) __copy3(ev1->cmd_argp->m.src_origin,src_origin); 
	else printcl( CL_ERR "fix this");

	if (region) __copy3(ev1->cmd_argp->m.region,region);
	else printcl( CL_ERR "fix this");

	ev1->cmd_argp->m.row_pitch = (void*)row_pitch;
	ev1->cmd_argp->m.slice_pitch = (void*)slice_pitch;
}


void __do_set_cmd_write_image_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* dst1, 
	const size_t* dst_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	const void* src
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src;
	__copy3(ev1->cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev1->cmd_argp->m.region,region);
	ev1->cmd_argp->m.row_pitch = (void*)row_pitch;
	ev1->cmd_argp->m.slice_pitch = (void*)slice_pitch;
}


void __do_set_cmd_copy_image_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* src1, struct coprthr_mem* dst1, 
	const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src1;
	__copy3(ev1->cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev1->cmd_argp->m.src_origin,src_origin);
	__copy3(ev1->cmd_argp->m.region,region);
}

/*
void __do_set_cmd_copy_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_image_1(ev->ev1,src->mem1[n],dst->mem1[n],src_origin,
		dst_origin,region);
}
*/


void __do_set_cmd_copy_image_to_buffer_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* src1, struct coprthr_mem* dst1, 
	const size_t* src_origin, const size_t* region, 
	size_t dst_offset
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src1;
	ev1->cmd_argp->m.dst_offset = dst_offset;
	__copy3(ev1->cmd_argp->m.src_origin,src_origin);
	__copy3(ev1->cmd_argp->m.region,region);
}

/*
void __do_set_cmd_copy_image_to_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, const size_t* region, 
	size_t dst_offset
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_image_to_buffer_1( ev->ev1, src->mem1[n], dst->mem1[n], 
		src_origin, region, dst_offset);
}
*/

void __do_set_cmd_copy_buffer_to_image_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* src1, struct coprthr_mem* dst1, 
	size_t src_offset, 
	const size_t* dst_origin, const size_t* region
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)dst1;
	ev1->cmd_argp->m.src = (void*)src1;
	ev1->cmd_argp->m.src_offset = src_offset;
	__copy3(ev1->cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev1->cmd_argp->m.region,region);
}

/*
void __do_set_cmd_copy_buffer_to_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, 
	const size_t* dst_origin, const size_t* region
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_buffer_to_image_1( ev->ev1, src->mem1[n], dst->mem1[n],
		src_offset, dst_origin, region);
}
*/

void __do_set_cmd_map_buffer_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* membuf1,
//	cl_map_flags flags, size_t offset, size_t len,
	int flags, size_t offset, size_t len,
	void* pp
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->flags = flags;
	ev1->cmd_argp->m.dst = (void*)pp;
	ev1->cmd_argp->m.src = (void*)membuf1;
	ev1->cmd_argp->m.src_offset = offset;
	ev1->cmd_argp->m.len = len;
}

/*
void __do_set_cmd_map_buffer( 
	cl_event ev, 
	cl_mem membuf,
	cl_map_flags flags, size_t offset, size_t len,
	void* pp
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_map_buffer_1(ev->ev1, membuf->mem1[n], flags, offset, len, pp); 
}
*/

void __do_set_cmd_map_image_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* image1,
//	cl_map_flags flags, const size_t* origin, const size_t* region,
	int flags, const size_t* origin, const size_t* region,
	size_t* row_pitch, size_t* slice_pitch,
	void* p
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->flags = flags;
	ev1->cmd_argp->m.dst = (void*)p;
	ev1->cmd_argp->m.src = (void*)image1;
	__copy3(ev1->cmd_argp->m.src_origin,origin);
	__copy3(ev1->cmd_argp->m.region,region);
	ev1->cmd_argp->m.row_pitch = (void*)row_pitch;
	ev1->cmd_argp->m.slice_pitch = (void*)slice_pitch;
}

/*
void __do_set_cmd_map_image( 
	cl_event ev, 
	cl_mem image,
	cl_map_flags flags, const size_t* origin, const size_t* region,
	size_t* row_pitch, size_t* slice_pitch,
	void* p
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_map_image_1( ev->ev1, image->mem1[n], flags, origin, region, 
		row_pitch, slice_pitch, p);
}
*/

void __do_set_cmd_unmap_memobj_1( 
	struct coprthr_event* ev1, 
	struct coprthr_mem* memobj1, void* p
)
{
	ev1->cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev1->cmd_argp);

	ev1->cmd_argp->m.dst = (void*)p;
	ev1->cmd_argp->m.src = (void*)memobj1;
}


void __do_set_cmd_ndrange_kernel_1(
	struct coprthr_event* ev1,
	struct coprthr_kernel* krn1,
	unsigned int work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size
)
{
	int i;

	struct cmdcall_arg* argp = ev1->cmd_argp 
		= (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	ev1->cmd = __CL_COMMAND_NDRANGE_KERNEL;

	ev1->nfunc = 1;

	__init_cmdcall_arg(argp);

	argp->flags = CMDCALL_ARG_K;

	argp->k.krn = krn1;

	int knum = krn1->knum;

	printcl( CL_DEBUG "knum=%d",knum);
	
	argp->k.ksyms = &(krn1->prg1->v_ksyms[knum]);
	argp->k.narg = krn1->prg1->knarg[krn1->knum];
	argp->k.arg_buf_sz = krn1->arg_buf_sz;

	printcl( CL_DEBUG "setting argp->k.arg_kind %p",
		krn1->prg1->karg_kind);
	
	argp->k.arg_kind = krn1->prg1->karg_kind[knum];
	argp->k.arg_sz = krn1->prg1->karg_sz[knum];


		
	/* XXX simplest to copy args, later test copy-on-set -DAR */

	unsigned int narg = krn1->prg1->knarg[krn1->knum];
	__clone(argp->k.pr_arg_off,krn1->arg_off,narg,uint32_t);
	__clone(argp->k.pr_arg_buf,krn1->arg_buf,krn1->arg_buf_sz, void);

	printcl( CL_DEBUG "arg_buf %p,%p",krn1->arg_buf,argp->k.pr_arg_buf);	
	intptr_t offset 
		= (intptr_t)argp->k.pr_arg_buf - (intptr_t)krn1->arg_buf;

	ev1->cmd_argp->k.work_dim = work_dim;

	for(i=0;i<work_dim;i++) {
		if (global_work_offset)
			ev1->cmd_argp->k.global_work_offset[i] = global_work_offset[i];
		ev1->cmd_argp->k.global_work_size[i] = global_work_size[i];
		ev1->cmd_argp->k.local_work_size[i] = local_work_size[i];
	}

	ev1->cmd_argp->k.nxt_argp = 0; /* XXX this allows chains -DAR */

}


void __do_set_cmd_ndrange_kernel_n(
	struct coprthr_event* ev1,
	unsigned int nkrn,
	struct coprthr_kernel* v_krn1[],
	unsigned int v_work_dim[],
	size_t* v_global_work_offset[],
	size_t* v_global_work_size[],
	size_t* v_local_work_size[]
)
{
	int i,n;

	struct cmdcall_arg* argp = ev1->cmd_argp 
		= (struct cmdcall_arg*)malloc(nkrn*sizeof(struct cmdcall_arg));

	ev1->cmd = __CL_COMMAND_NDRANGE_KERNEL;

	ev1->nfunc = nkrn;

	for(n=0; n<nkrn; n++, argp++) {

		__init_cmdcall_arg(argp);

		argp->flags = CMDCALL_ARG_K;

		struct coprthr_kernel* krn = v_krn1[n];

		argp->k.krn = krn;

		int knum = krn->knum;

		printcl( CL_DEBUG "knum=%d",knum);
	
		argp->k.ksyms = &(krn->prg1->v_ksyms[knum]);
		argp->k.narg = krn->prg1->knarg[knum];
		argp->k.arg_buf_sz = krn->arg_buf_sz;

		printcl( CL_DEBUG "setting argp->k.arg_kind %p",
			krn->prg1->karg_kind);
	
		argp->k.arg_kind = krn->prg1->karg_kind[knum];
		argp->k.arg_sz = krn->prg1->karg_sz[knum];

		/* XXX simplest to copy args, later test copy-on-set -DAR */

		unsigned int narg = krn->prg1->knarg[knum];
		__clone(argp->k.pr_arg_off,krn->arg_off,narg,uint32_t);
		__clone(argp->k.pr_arg_buf,krn->arg_buf,krn->arg_buf_sz, void);

		printcl( CL_DEBUG "arg_buf %p,%p",krn->arg_buf,argp->k.pr_arg_buf);	
		intptr_t offset 
			= (intptr_t)argp->k.pr_arg_buf - (intptr_t)krn->arg_buf;

		int dim = argp->k.work_dim = v_work_dim[n];

		for(i=0;i<dim;i++) {
			if (v_global_work_offset[n])
				argp->k.global_work_offset[i] = v_global_work_offset[n][i];
				argp->k.global_work_size[i] = v_global_work_size[n][i];
				argp->k.local_work_size[i] = v_local_work_size[n][i];
		}

		argp->k.nxt_argp = ( (n==nkrn-1)? 0 : argp+1 );

	}

}


void __do_set_cmd_task_1( 
	struct coprthr_event* ev1, struct coprthr_kernel* krn1
)
{
#if(0)
	int i;

	struct cmdcall_arg* argp = ev1->cmd_argp 
		= (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(argp);

	argp->flags = CMDCALL_ARG_K;

	argp->k.krn = krn1; /* XXX depreacted, remove -DAR */

	int devnum;
	for(devnum=0;devnum<krn->prg->ndev;devnum++) 
		if (ev1->dev == krn->prg->devices[devnum]->codev) break;

	if (devnum == krn->prg->ndev) {
		printcl( CL_ERR "internal error");
		exit(-1);
	}

	int knum = krn->krn1[devnum]->knum;
	struct coprthr_program* prg1 = krn->krn1[devnum]->prg1;
	argp->k.ksym = prg1->v_ksyms[knum].kthr;
	argp->k.kcall = prg1->v_ksyms[knum].kcall;
	argp->k.narg = krn->narg;

	printcl( CL_DEBUG "setting argp->k.arg_kind %p",
		krn->krn1[devnum]->prg1->karg_kind[knum]);
	
	argp->k.arg_kind = krn->krn1[devnum]->prg1->karg_kind[knum];
	argp->k.arg_sz = krn->krn1[devnum]->prg1->karg_sz[knum];

	/* XXX simplest to copy args, later test copy-on-set -DAR */

	__clone(argp->k.pr_arg_off,krn->krn1[0]->arg_off,krn->narg,uint32_t);
	__clone(argp->k.pr_arg_buf,krn->krn1[0]->arg_buf,krn->krn1[0]->arg_buf_sz,void);

	ev1->cmd_argp->k.work_dim = 0;
#endif
}


/*
 * wait
 */

void __do_wait_1( struct coprthr_event* ev1 )
{
		printcl( CL_DEBUG "wait for event %p %d\n",ev1,ev1->cmd_stat);

		__lock_event1(ev1);

		while (ev1->cmd_stat != __CL_COMPLETE) {
			printcl( CL_DEBUG "__do_wait_1: wait-sleep\n");
			__wait_event1(ev1);
			printcl( CL_DEBUG "__do_wait_1: wait-wake\n");
		}
 
		printcl( CL_DEBUG "event %p complete\n",ev1);

		__unlock_event1(ev1);
}


__attribute__((noinline))
int __qnotempty( struct coprthr_command_queue* cmdq1  ) 
{
	if (cmdq1->cmds_queued.tqh_first || cmdq1->cmd_submitted 
		|| cmdq1->cmd_running) 
			return 1;

	else
		return 0;
}

void __do_waitq_1( struct coprthr_command_queue* cmdq1 )
{
	printcl( CL_DEBUG "waitq %p\n",cmdq1);

	while ( __qnotempty(cmdq1) );
//			printcl( CL_DEBUG "spinning");

	return;
}


/*
void __do_wait_for_events( cl_uint nev, const cl_event* evlist)
{
	int i;
	cl_event ev;

	for(i=0;i<nev;i++) 
		__do_wait_1(evlist[i]->ev1);

}
*/


struct coprthr_event* coprthr_dread( int dd, struct coprthr_mem* mem, 
	size_t offset, void* buf, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_dread: mem %p",mem);
	printcl( CL_DEBUG "coprthr_dread: mem->res %p",mem->res);

	struct coprthr_event* ev1 = __malloc_struct(coprthr_event);
	__coprthr_init_event(ev1);

	__do_set_cmd_read_buffer_1( ev1, mem, 0, len, buf );

	struct coprthr_device* dev = __ddtab[dd];

	if (flags & COPRTHR_E_NOW) {
		__do_exec_cmd_1( dev,ev1);
	} else {
		__do_enqueue_cmd_1( dev,ev1);
		if (flags & COPRTHR_E_WAIT)
			__do_wait_1(ev1);
	}

	return ev1;
}

size_t coprthr_devread( 
	struct coprthr_device* dev, struct coprthr_mem* mem, size_t offset,
	void* buf, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_devread: mem %p",mem);
	printcl( CL_DEBUG "coprthr_devread: mem->res %p",mem->res);

	if (offset) {
		printcl( CL_WARNING "coprthr_devread: non-zero offset not supported");
		errno = ENOTSUP;
		return 0;
	}

	size_t sz = dev->devops->memread(mem,buf,len);
	return sz;
}

struct coprthr_event* coprthr_dwrite( 
	int dd, struct coprthr_mem* mem, size_t offset, void* buf, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_dwrite: memr %p",mem);
	printcl( CL_DEBUG "coprthr_dwrite: mem->res %p",mem->res);

	struct coprthr_event* ev1 = __malloc_struct(coprthr_event);
	__coprthr_init_event(ev1);

	__do_set_cmd_write_buffer_1( ev1, mem, 0, len, buf );

	struct coprthr_device* dev = __ddtab[dd];

	if (flags & COPRTHR_E_NOW) {
		__do_exec_cmd_1( dev,ev1);
	} else {
		__do_enqueue_cmd_1( dev,ev1);
		if (flags & COPRTHR_E_WAIT)
			__do_wait_1(ev1);
	}

	return ev1;
}

size_t coprthr_devwrite( 
	struct coprthr_device* dev, struct coprthr_mem* mem, size_t offset,
	void* buf, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_devwrite: mem %p",mem);
	printcl( CL_DEBUG "coprthr_devwrite: mem->res %p",mem->res);

	if (offset) {
		printcl( CL_WARNING "coprthr_devwrite: non-zero offset not supported");
		errno = ENOTSUP;
		return 0;
	}

	size_t sz = dev->devops->memwrite(mem,buf,len);
	return sz;
}


struct coprthr_event* coprthr_dcopy( 
	int dd, struct coprthr_mem* mem_src, size_t offset_src, 
	struct coprthr_mem* mem_dst, size_t offset, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_dcopy: mem_src %p",mem_src);
	printcl( CL_DEBUG "coprthr_dcopy: mem_src->res %p",mem_src->res);

	printcl( CL_DEBUG "coprthr_dcopy: mem_dst %p",mem_dst);
	printcl( CL_DEBUG "coprthr_dcopy: mem_dst->res %p",mem_dst->res);

	struct coprthr_event* ev1 = __malloc_struct(coprthr_event);
	__coprthr_init_event(ev1);

	__do_set_cmd_copy_buffer_1( ev1, mem_src, mem_dst, 0, 0, len );

	struct coprthr_device* dev = __ddtab[dd];

	if (flags & COPRTHR_E_NOW) {
		__do_exec_cmd_1( dev,ev1);
	} else {
		__do_enqueue_cmd_1( dev,ev1);
		if (flags & COPRTHR_E_WAIT)
			__do_wait_1(ev1);
	}

	return ev1;
}


size_t coprthr_devcopy( 
	struct coprthr_device* dev, struct coprthr_mem* mem_src, size_t offset_src,
	struct coprthr_mem* mem_dst, size_t offset_dst, size_t len, int flags
)
{
	printcl( CL_DEBUG "coprthr_devcopy: mem_src %p",mem_src);
	printcl( CL_DEBUG "coprthr_devcopy: mem_src->res %p",mem_src->res);

	printcl( CL_DEBUG "coprthr_devcopy: mem_dst %p",mem_dst);
	printcl( CL_DEBUG "coprthr_devcopy: mem_dst->res %p",mem_dst->res);

	if (offset_src || offset_dst) {
		printcl( CL_WARNING "coprthr_devcopy: non-zero offset not supported");
		errno = ENOTSUP;
		return 0;
	}

	size_t sz = dev->devops->memcopy(mem_src,mem_dst,len);

	return sz;
}


int coprthr_dwaitev( int dd, struct coprthr_event* ev1)
{ 
	__do_wait_1(ev1); 
	return 0;
}


int coprthr_dwait( int dd )
{ 
	struct coprthr_device* dev = __ddtab[dd];
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;
	__do_waitq_1(cmdq1); 
	return 0;
}


void* coprthr_dexec( 
	int dd, struct coprthr_kernel* krn1, 
	unsigned int nargs, void** args,
	unsigned int nthr, int flags
)
{
	printcl( CL_DEBUG "coprthr_dexec");

	int iarg;
	for(iarg=0;iarg<nargs;iarg++) {
		__do_set_kernel_arg_1(krn1,iarg,0,args[iarg]);
	}

	struct coprthr_event* ev1 = __malloc_struct(coprthr_event);

	__coprthr_init_event(ev1);

	size_t gwo = 0;
	size_t gws = nthr;
	size_t lws = 16;
//	size_t lws = 2;

	unsigned int one[] = { 1 };
	size_t* v_gwo[] = { &gwo };
	size_t* v_gws[] = { &gws };
	size_t* v_lws[] = { &lws };

//	__do_set_cmd_ndrange_kernel_1( ev1, krn1, 1, &gwo, &gws, &lws);
	__do_set_cmd_ndrange_kernel_n( ev1, 1, &krn1, one, v_gwo, v_gws, v_lws);

	struct coprthr_device* dev = __ddtab[dd];

	__do_enqueue_cmd_1( dev,ev1);

	return ev1;
}


void* coprthr_dnexec( 
	int dd, unsigned int nkrn, struct coprthr_kernel* v_krn[],
	unsigned int v_nargs[], void** v_args[],
	unsigned int v_nthr[], int flags
)
{
	int n;

	printcl( CL_DEBUG "coprthr_dnexec");

	if (nkrn>1)
		printcl( CL_WARNING "multiple kernels not supported");

//	struct coprthr1_kernel* krn1 = v_krn[0];
//	unsigned int nargs = v_nargs[0];
//	void** args = v_args[0];
//	unsigned int nthr = (v_nthr)? v_nthr[0] : 1;

	int k;
	for(k=0;k<nkrn;k++) {

		int iarg;
		for(iarg=0;iarg<v_nargs[k];iarg++) {
			__do_set_kernel_arg_1(v_krn[k],iarg,0,v_args[k][iarg]);
		}

	}





	struct coprthr_event* ev1 = __malloc_struct(coprthr_event);

	__coprthr_init_event(ev1);

//	size_t gwo = 0;
//	size_t gws = nthr;
//	size_t lws = 1;

	unsigned int* v_one = (unsigned int*)malloc(nkrn*sizeof(unsigned int));
	size_t** v_gwo = (size_t**)malloc(nkrn*sizeof(size_t*));
	size_t** v_gws = (size_t**)malloc(nkrn*sizeof(size_t*));
	size_t** v_lws = (size_t**)malloc(nkrn*sizeof(size_t*));
	for(n=0; n<nkrn; n++) {
		v_one[n] = 1;
		v_gwo[n] = (size_t*)malloc(sizeof(size_t));
		v_gws[n] = (size_t*)malloc(sizeof(size_t));
		v_lws[n] = (size_t*)malloc(sizeof(size_t));
		v_gwo[n][0] = 0;
		v_gws[n][0] = v_nthr[n];
		v_lws[n][0] = 1;
	}
	

//	__do_set_cmd_ndrange_kernel_1( ev1, krn1, 1, &gwo, &gws, &lws);
//	ev1->nfunc = 1;
	__do_set_cmd_ndrange_kernel_n(ev1, nkrn, v_krn, v_one, v_gwo, v_gws, v_lws);

	struct coprthr_device* dev = __ddtab[dd];

	__do_enqueue_cmd_1( dev,ev1);

	return ev1;
}


int coprthr_devexec( 
	struct coprthr_device* dev, int nthr, struct coprthr_kernel* krn1, 
	unsigned int narg, void** args 
)
{
	printcl( CL_DEBUG "coprthr_devexec");

	int iarg;
	for(iarg=0;iarg<narg;iarg++) {
		__do_set_kernel_arg_1(krn1,iarg,0,args[iarg]);
	}

	struct coprthr_event* ev1 = (struct coprthr_event*)
		malloc(sizeof(struct coprthr_event));

	size_t gwo = 0;
	size_t gws = nthr;
	size_t lws = 16;

	__do_set_cmd_ndrange_kernel_1( ev1, krn1, 1, &gwo, &gws, &lws);

	__do_exec_cmd_1( dev, ev1);

//	return ev1;
	free(ev1);

	return 0;
}

