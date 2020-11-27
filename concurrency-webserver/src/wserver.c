#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "util.h"
#include "pool.h"
#include <execinfo.h>

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>]
//

void * serve(void * arg)
{
	int conn_fd = (int) arg;
	request_handle(conn_fd);
	close_or_die(conn_fd);
	return (void *)0;
}

ThreadPool * pool;

void handle(int sig)
{
	signal(sig, SIG_DFL);
    printf("receive a signal of %d to dump trace\n", sig);

	if (pool) {
		printf("first, shutdown the thread pool\n");
		destroy_thread_pool(pool);
		//pool = NULL;
	}
    void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	printf("frames = %d\n", frames);
	for (i = 0; i < frames; ++i) {
		printf("%s\n", strs[i]);
	}
	free(strs);
	printf("GoodBye coder ...\n");
	exit(1);
}

void sig_actions() {
    signal(SIGUSR1, handle);  // 10
    signal(SIGTERM, handle);  // 15

    // Dump traceback when crash.
    // Core signal's default action is to terminate the process and dump core.
}


int main(int argc, char *argv[]) {
    int c;
    const char *root_dir = default_root, *schedalg = "FIFO";
    int port = 10000, thread_num = 1, buffers = 1;

    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
    case 't':
        thread_num = atoi(optarg);
	    break;
    case 'b':
        buffers = atoi(optarg);
	    break;
    case 's':
        schedalg = optarg;
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

	sig_actions();

	pool = make_thread_pool(thread_num, buffers);
    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

	JobParam * job = (JobParam *) Malloc(sizeof(JobParam));
	print_id();
	printf("malloc job at %p\n", job);
	job->arg = (void *) conn_fd;
	job->func = serve;
	submit(pool, job);
	printf("submit job %p success\n", job);
    }
    return 0;
}