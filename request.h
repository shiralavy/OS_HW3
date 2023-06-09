#ifndef __REQUEST_H__

void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, int thread_id, int* total_count, int* static_count, int* dynamic_count);

#endif
