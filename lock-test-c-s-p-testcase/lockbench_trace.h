#undef TRACE_SYSTEM
#define TRACE_SYSTEM lockbench

#if !defined(_LOCKBENCH_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _LOCKBENCH_TRACE_H_

#include <linux/tracepoint.h>

TRACE_EVENT(lock,
	TP_PROTO(int threads),
	TP_ARGS(threads),
	TP_STRUCT__entry(
	__field(int, threads)),
	TP_fast_assign(
	__entry->threads = threads),
	TP_printk(
	"threads=%d", __entry->threads)
);

TRACE_EVENT(unlock,
	TP_PROTO(int threads),
	TP_ARGS(threads),
	TP_STRUCT__entry(
	__field(int, threads)),
	TP_fast_assign(
	__entry->threads = threads),
	TP_printk(
	"threads=%d", __entry->threads)
);

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE lockbench_trace

#include <trace/define_trace.h>
