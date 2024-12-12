#pragma once

void platform_handle_failed_assertion(char*, char*, int);
void platform_add_file_watch(string, void (*callback)(string, void*), void*);
int platform_find_all_files(char* dir, char* format, void* memory, unsigned long long* bytes_used);
void platform_sleep(u64);
