/* command_queue.h
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

#ifndef _command_queue_h
#define _command_queue_h

#include "coprthr_device.h"
#include "coprthr_sched.h"

//#define __lock_cmdq(cmdq) pthread_mutex_lock(&((cmdq)->ptr_imp->mtx))
#define __lock_cmdq1(cmdq1) pthread_mutex_lock( &((cmdq1)->mtx) )

//#define __unlock_cmdq(cmdq) pthread_mutex_unlock(&(cmdq)->ptr_imp->mtx)
#define __unlock_cmdq1(cmdq1) pthread_mutex_unlock( &((cmdq1)->mtx) )

//#define __wait_cmdq(cmdq) pthread_cond_wait(&(cmdq)->ptr_imp->sig,&(cmdq)->ptr_imp->mtx)
#define __wait_cmdq1(cmdq1) \
	pthread_cond_wait( &((cmdq1)->sig), &((cmdq1)->mtx) )

//#define __sig_cmdq(cmdq) pthread_cond_signal(&(cmdq)->ptr_imp->sig)
#define __sig_cmdq1(cmdq1) pthread_cond_signal( &((cmdq1)->sig) )


//void __do_create_command_queue( cl_command_queue cmdq );
//void __do_release_command_queue( cl_command_queue cmdq );
//void __do_enqueue_cmd( cl_command_queue cmdq, cl_event ev );
//void __do_finish( cl_command_queue cmdq );

void __do_create_command_queue_1( struct coprthr_device* dev );
void __do_release_command_queue_1( struct coprthr_device* dev ) ;
void __do_enqueue_cmd_1(struct coprthr_device* dev, struct coprthr_event* ev1);
void __do_finish_1( struct coprthr_device* dev );
void __do_exec_cmd_1( struct coprthr_device* dev, struct coprthr_event* ev1 );


#endif

