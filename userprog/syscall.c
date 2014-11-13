#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "vm/frame.h"
#include "vm/page.h"

#define MAX_ARGS 3

static void syscall_handler (struct intr_frame *);

int add_file_to_currently_used_files(struct file *file_);
struct file* get_file_from_currently_used_files(int file_descriptor);

int get_physicaladdr(const void *virtual_addr);
void retrieve_syscall_param(struct intr_frame *f, int *arg,
		int number_of_parameters);
bool is_virtual_addr_valid(const void *virtual_addr);

char* get_physical_file(int file_syscall_parameter);
unsigned int get_size(int size_syscall_parameter);
int get_file_descriptor(int file_descriptor_syscall_parameter);
int read_from_standard_input(void *buffer, unsigned size_to_be_read);
int read_from_file(int file_descriptor, void *buffer, unsigned size_to_be_read);
int write_to_standard_output(void *buffer, unsigned size_to_be_read);
int write_to_file(int file_descriptor, void *buffer, unsigned size_to_be_read);
int perform_actions_after_file_open(struct file *file_);
void close_single_file(struct file_details* file_detials);
struct file_details* find_file_details(struct thread *t, int file_descriptor);

// Start - Syscall declarations
void sys_halt_call(void);
void sys_exit_call(struct intr_frame* f);
void sys_exec_call(struct intr_frame* f);
void sys_wait_call(struct intr_frame* f);
void sys_create_call(struct intr_frame* f);
void sys_remove_call(struct intr_frame* f);
void sys_open_call(struct intr_frame* f);
void sys_filesize_call(struct intr_frame* f);
void sys_read_call(struct intr_frame* f);
void sys_write_call(struct intr_frame* f);
void sys_seek_call(struct intr_frame* f);
void sys_tell_call(struct intr_frame* f);
void sys_close_call(struct intr_frame* f);
void sys_mmap_call(struct intr_frame* f);
void sys_munmap_call(struct intr_frame* f);
// End - Syscall declarations

struct supplemental_pte* check_valid_ptr (const void *vaddr, void* esp);
void check_valid_buffer (void* buffer, unsigned size, void* esp, bool to_write);
void check_valid_string (const void* str, void* esp);
void is_page_writable (struct supplemental_pte *spte);
void unpin_ptr (void* vaddr);
void unpin_string (void* str);
void unpin_buffer (void* buffer, unsigned size);

void syscall_init(void) {

	// Initializing the lock.
	lock_init(&file_resource_lock);
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*
 *	This is the main handler for syscalls
 */
static void syscall_handler(struct intr_frame *f UNUSED) {

	int arg[MAX_ARGS];
	check_valid_ptr((const void*) f->esp, f->esp);

	//get the system call number from esp
	int system_call_number = *(int *) f->esp;

	switch (system_call_number) {
	case SYS_HALT: {

		sys_halt_call();
		break;
	}
	case SYS_EXIT: {

		sys_exit_call(f);
		break;
	}
	case SYS_EXEC: {

		sys_exec_call(f);
		break;
	}
	case SYS_WAIT: {

		sys_wait_call(f);
		break;
	}
	case SYS_CREATE: {

		sys_create_call(f);
		break;
	}
	case SYS_REMOVE: {
		sys_remove_call(f);
		break;
	}
	case SYS_OPEN: {

		sys_open_call(f);
		break;
	}
	case SYS_FILESIZE: {

		sys_filesize_call(f);
		break;
	}
	case SYS_READ: {

		sys_read_call(f);
		break;
	}
	case SYS_WRITE: {

		sys_write_call(f);
		break;
	}
	case SYS_SEEK: {

		sys_seek_call(f);
		break;
	}
	case SYS_TELL: {

		sys_tell_call(f);
		break;
	}
	case SYS_CLOSE: {

		sys_close_call(f);
		break;
	}
	case SYS_MMAP: {

		sys_mmap_call(f);
		break;
	}
	case SYS_MUNMAP: {

		sys_munmap_call(f);
		break;
	}
	}
	unpin_ptr(f->esp);
}

/*
 * This is the OS shutdown
 */
void sys_halt_call(void) {

	shutdown_power_off();
}

void sys_exit_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	// The status for sys_exit
	int status = arg[0];
	exit(status);
}

void sys_exec_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	//const char *file = get_physical_file(arg[0]);
	//TODO
	check_valid_string((const void *) arg[0], f->esp);
	f->eax = exec((const char *) arg[0]);
	unpin_string((void *) arg[0]);
	//************
}

void sys_wait_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	int pid = arg[0];
	f->eax = wait(pid);
}

void sys_create_call(struct intr_frame* f) {

	int arg[2];
	retrieve_syscall_param(f, &arg[0], 2);
	//TODO
	check_valid_string((const void *) arg[0], f->esp);
	//*********
	//const char *file = get_physical_file(arg[0]);
	//unsigned initial_size = get_size(arg[1]);
	//f->eax = create(file, initial_size);
	f->eax = create((const char *)arg[0], (unsigned) arg[1]);
	unpin_string((void *) arg[0]);
}

void sys_remove_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	//const char *file = get_physical_file(arg[0]);
	//f->eax = remove(file);
	//TODO
	check_valid_string((const void *) arg[0], f->esp);
	f->eax = remove((const char *) arg[0]);
	//**************
}

void sys_open_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	/*const char *file = get_physical_file(arg[0]);
	f->eax = open(file);*/
	//TODO
	check_valid_string((const void *) arg[0], f->esp);
	f->eax = open((const char *) arg[0]);
	unpin_string((void *) arg[0]);
	//******************
}

void sys_filesize_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);
	int file_descriptor = get_file_descriptor(arg[0]);

	f->eax = filesize(file_descriptor);
}

void sys_read_call(struct intr_frame* f) {

	int arg[3];
	retrieve_syscall_param(f, &arg[0], 3);

	/*void *virtual_buffer = (void *) arg[1];
	unsigned size = get_size(arg[2]);
	is_virtual_addr_valid((const void*) virtual_buffer);

	void *physical_buffer = get_physicaladdr(virtual_buffer);
	int file_descriptor = get_file_descriptor(arg[0]);

	f->eax = read(file_descriptor, physical_buffer, size);*/
	//TODO
	check_valid_buffer((void *) arg[1], (unsigned) arg[2], f->esp,
			true);
	f->eax = read(arg[0], (void *) arg[1], (unsigned) arg[2]);
	unpin_buffer((void *) arg[1], (unsigned) arg[2]);
	//**************
}

void sys_write_call(struct intr_frame* f) {

	int arg[3];
	retrieve_syscall_param(f, &arg[0], 3);

	/*void *virtual_buffer = (void *) arg[1];
	unsigned size = get_size(arg[2]);
	is_virtual_addr_valid((const void*) virtual_buffer);

	void *physical_buffer = get_physicaladdr(virtual_buffer);
	int file_descriptor = get_file_descriptor(arg[0]);

	f->eax = write(file_descriptor, physical_buffer, size);*/
	//TODO
	check_valid_buffer((void *) arg[1], (unsigned) arg[2], f->esp,
			false);
	f->eax = write(arg[0], (const void *) arg[1],
			(unsigned) arg[2]);
	unpin_buffer((void *) arg[1], (unsigned) arg[2]);
	//******************
}

void sys_seek_call(struct intr_frame* f) {

	int arg[2];
	retrieve_syscall_param(f, &arg[0], 2);

	int file_descriptor = get_file_descriptor(arg[0]);
	unsigned position = (unsigned) arg[1];

	seek(file_descriptor, position);
}

void sys_tell_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);

	int file_descriptor = get_file_descriptor(arg[0]);

	f->eax = tell(file_descriptor);
}

void sys_close_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);

	int file_descriptor = get_file_descriptor(arg[0]);

	close(file_descriptor);
}

void sys_mmap_call(struct intr_frame* f) {

	int arg[2];
	retrieve_syscall_param(f, &arg[0], 2);

	f->eax = mmap(arg[0], (void *) arg[1]);
}

void sys_munmap_call(struct intr_frame* f) {

	int arg[1];
	retrieve_syscall_param(f, &arg[0], 1);

	munmap(arg[0]);
}

int mmap (int fd, void *addr)
{
	struct file *old_file = get_file_from_currently_used_files(fd);
	if (!old_file || !is_virtual_addr_valid(addr) || ((uint32_t) addr % PGSIZE) != 0)
	{
		return SYSCALL_ERROR;
	}
	struct file *file = file_reopen(old_file);
	if (!file || file_length(old_file) == 0)
	{
		return SYSCALL_ERROR;
	}
	thread_current()->mapid++;
	int32_t ofs = 0;
	uint32_t read_bytes = file_length(file);
	while (read_bytes > 0)
	{
		uint32_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		uint32_t page_zero_bytes = PGSIZE - page_read_bytes;
		if (!push_mapped_file_in_supplemental_page_table(file, ofs,
				addr, page_read_bytes, page_zero_bytes))
		{
			munmap(thread_current()->mapid);
			return SYSCALL_ERROR;
		}
		read_bytes -= page_read_bytes;
		ofs += page_read_bytes;
		addr += PGSIZE;
	}
	return thread_current()->mapid;
}

void munmap (int mapping)
{
	process_remove_mmap(mapping);
}

void halt (void)
{
	shutdown_power_off();
}

/*
 * Terminates the current user program, returning status to the kernel.
 * If the process's parent waits for it (see below), this is the status that will be returned.
 * Conventionally, a status of 0 indicates success and nonzero values indicate errors.
 */
void exit(int status) {

	struct thread *current_thread = thread_current();
	//TODO
	if (is_present_in_kernel(current_thread->parent_tid)) {

		struct spawned_child_thread *my_pos =
				current_thread->my_position_in_parent_children;
		my_pos->status_value = status;
		if (my_pos->is_waiting) {

			lock_acquire(&my_pos->wait_lock);
			cond_signal(&my_pos->wait_cond, &my_pos->wait_lock);
			lock_release(&my_pos->wait_lock);
		}
	}
	printf("%s: exit(%d)\n", current_thread->name, status);
	thread_exit();
}

/*
 *Runs the executable whose name is given in cmd_line, passing any given arguments,
 *and returns the new process's program id (pid).
 *
 *Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason.
 *Thus, the parent process cannot return from the exec until it knows whether the child process successfully loaded its executable.
 *
 *TODO: Must use appropriate synchronization to ensure this.
 */
pid_t exec(const char *cmd_line) {

	pid_t pid = process_execute(cmd_line);
	struct spawned_child_thread* child_process = retrieve_child(pid);

	if (!child_process) {

		return SYSCALL_ERROR;
	}

	while (child_process->load_status == LOAD_NOT_STARTED) {

		lock_acquire(&child_process->exec_lock);
		cond_wait(&child_process->exec_cond, &child_process->exec_lock);
		lock_release(&child_process->exec_lock);
	}

	if (child_process->load_status == FAILED_LOAD) {

		return SYSCALL_ERROR;
	}

	return pid;
}

/*
 *Waits for a child process pid and retrieves the child's exit status.
 *
 *If pid is still alive, waits until it terminates.Then, returns the status that pid passed to exit.
 *If pid did not call exit(), but was terminated by the kernel (e.g. killed due to an exception), wait(pid) must return -1.
 *
 *Note: It is perfectly legal for a parent process to wait for child processes that have already terminated by the time
 *the parent calls wait, but the kernel must still allow the parent to retrieve its child's exit status,
 *or learn that the child was terminated by the kernel.
 *
 *
 *wait must fail and return -1 immediately if any of the following conditions is true:
 *1. pid does not refer to a direct child of the calling process.
 *pid is a direct child of the calling process if and only if the calling process
 *received pid as a return value from a successful call to exec.
 *
 *2.The process that calls wait has already called wait on pid. That is, a process may wait for any given child at most once.
 */
int wait(pid_t pid) {

	return process_wait(pid);
}

/**
 *Creates a new file called file initially initial_size bytes in size. Returns true if successful, false otherwise.
 */
bool create(const char *file, unsigned initial_size) {

	lock_acquire(&file_resource_lock);

	bool success = filesys_create(file, initial_size);

	lock_release(&file_resource_lock);

	return success;
}

/**
 *Deletes the file called file. Returns true if successful, false otherwise.
 *A file may be removed regardless of whether it is open or closed, and removing an open file does not close it.
 */
bool remove(const char *file) {

	lock_acquire(&file_resource_lock);

	bool success = filesys_remove(file);

	lock_release(&file_resource_lock);

	return success;
}

/**
 *Opens the file called file.
 *Returns a nonnegative integer handle called a "file descriptor" (fd), or -1 if the file could not be opened.
 */
int open(const char *file) {

	lock_acquire(&file_resource_lock);

	struct file *f = filesys_open(file);
	if (!f) {

		lock_release(&file_resource_lock);
		return SYSCALL_ERROR;
	}

	int file_descriptor = perform_actions_after_file_open(f);

	lock_release(&file_resource_lock);

	return file_descriptor;
}

/**
 *Returns the size, in bytes, of the file open as fd.
 */
int filesize(int file_descriptor) {

	lock_acquire(&file_resource_lock);

	struct file *f = get_file_from_currently_used_files(file_descriptor);
	if (f == NULL) {

		lock_release(&file_resource_lock);
		return SYSCALL_ERROR;
	}

	int size = file_length(f);

	lock_release(&file_resource_lock);

	return size;
}

/**
 *Reads size bytes from the file open as fd into buffer.
 *
 *Returns the number of bytes actually read (0 at end of file),
 *or -1 if the file could not be read (due to a condition other than end of file).
 */
int read(int file_descriptor, void *buffer, unsigned size) {

	if (file_descriptor == STDIN_FILENO) {

		int size_ = read_from_standard_input(buffer, size);
		return size_;
	}

	return read_from_file(file_descriptor, buffer, size);
}

/**
 *Writes size bytes from buffer to the open file fd.
 *
 *Returns the number of bytes actually written, which may be less than size if some bytes could not be written.
 */
int write(int file_descriptor, const void *buffer, unsigned size) {

	if (file_descriptor == STDOUT_FILENO) {

		int size_ = write_to_standard_output(buffer, size);
		return size_;
	}

	return write_to_file(file_descriptor, buffer, size);
}

/**
 * Changes the next byte to be read or written in open file fd to position,
 * expressed in bytes from the beginning of the file.
 */
void seek(int fd, unsigned position) {

	lock_acquire(&file_resource_lock);

	struct file *f = get_file_from_currently_used_files(fd);
	if (f == NULL) {
		lock_release(&file_resource_lock);
		return;
	}

	file_seek(f, position);

	lock_release(&file_resource_lock);
}

/**
 *Returns the position of the next byte to be read or written in open file fd,
 *expressed in bytes from the beginning of the file.
 */
unsigned tell(int fd) {

	lock_acquire(&file_resource_lock);

	struct file *f = get_file_from_currently_used_files(fd);
	if (f == NULL) {

		lock_release(&file_resource_lock);
		return SYSCALL_ERROR;
	}

	off_t offset = file_tell(f);

	lock_release(&file_resource_lock);

	return offset;
}

/**
 *Closes file descriptor fd.
 *
 *Exiting or terminating a process implicitly closes all its open file descriptors,
 *as if by calling this function for each one.
 */
void close(int fd) {

	lock_acquire(&file_resource_lock);

	close_file(fd);

	lock_release(&file_resource_lock);
}

struct supplemental_pte* check_valid_ptr(const void *vaddr, void* esp)
{
	if (!is_virtual_addr_valid(vaddr)) {

		exit(SYSCALL_ERROR);
	}
	bool load = false;
	struct supplemental_pte *spte = get_supplemental_pte((void *) vaddr);
	if (spte)
	{
		supplemental_page_table_handler(spte);
		load = spte->is_page_loaded;
	}
	else if (vaddr >= esp - STACK_HEURISTIC)
	{
		load = increment_stack_size((void *) vaddr);
	}
	if (!load)
	{
		exit(SYSCALL_ERROR);
	}
	return spte;
}

void check_valid_buffer (void* buffer, unsigned size, void* esp,
		bool to_write)
{
	unsigned i;
	char* local_buffer = (char *) buffer;
	for (i = 0; i < size; i++)
	{
		struct supplemental_pte *spte = check_valid_ptr((const void*)
				local_buffer, esp);
		if (spte && to_write)
		{
			if (!spte->is_page_writable)
			{
				exit(SYSCALL_ERROR);
			}
		}
		local_buffer++;
	}
}

void check_valid_string (const void* str, void* esp)
{
	check_valid_ptr(str, esp);
	while (* (char *) str != 0)
	{
		str = (char *) str + 1;
		check_valid_ptr(str, esp);
	}
}

void unpin_ptr (void* vaddr)
{
	struct supplemental_pte *spte = get_supplemental_pte(vaddr);
	if (spte)
	{
		spte->is_page_pinned = false;
	}
}

void unpin_string (void* str)
{
	unpin_ptr(str);
	while (* (char *) str != 0)
	{
		str = (char *) str + 1;
		unpin_ptr(str);
	}
}

void unpin_buffer (void* buffer, unsigned size)
{
	unsigned i;
	char* local_buffer = (char *) buffer;
	for (i = 0; i < size; i++)
	{
		unpin_ptr(local_buffer);
		local_buffer++;
	}
}

/**
 * Utility functions
 */

/**
 * Checks if a given virtual address is valid.
 */
bool is_virtual_addr_valid(const void *virtual_addr) {

	return (virtual_addr > ((void *) 0x08048000) && is_user_vaddr(virtual_addr));
}

/**
 * Returns the physical address corresponding to the give virtual address.
 * If physical addressed is unmapped, this function will call exit().
 *
 */
int get_physicaladdr(const void *virtual_addr) {

	if (!is_virtual_addr_valid(virtual_addr)) {

		exit(SYSCALL_ERROR);
	}

	void *physical_addr = pagedir_get_page(thread_current()->pagedir,
			virtual_addr);

	if (physical_addr == NULL) {

		exit(SYSCALL_ERROR);
	}

	return (int) physical_addr;
}

/**
 * Adds file to the list of files used by the thread and returns the file descriptor
 */
int add_file_to_currently_used_files(struct file *file_) {

	struct file_details *f_details = malloc(sizeof(struct file_details));

	//TODO
	if(!f_details){
		return SYSCALL_ERROR;
	}

	f_details->file = file_;
	f_details->file_descriptor = thread_current()->current_fd_to_be_assigned;

	thread_current()->current_fd_to_be_assigned++;

	list_push_back(&thread_current()->currently_used_files, &f_details->elem);

	return f_details->file_descriptor;
}

/**
 * Retrieves the file from the threads file list based on file descriptor
 */
struct file* get_file_from_currently_used_files(int file_descriptor) {
	struct thread *t = thread_current();
	struct list_elem *e = list_begin(&t->currently_used_files);

	while (e != list_end(&t->currently_used_files)) {
		struct file_details *file_details_ =
				list_entry (e, struct file_details, elem);

		if (file_descriptor == file_details_->file_descriptor) {

			//found the file. just return
			return file_details_->file;
		}

		e = list_next(e);
	}

	return NULL;
}

/**
 * Close file.
 *
 * If fd = PROCESS_EXIT then closes all files.
 * otherwise just close the file with given file descriptor.
 *
 * frees up memory respectively.
 */
void close_file(int file_descriptor) {
	struct thread *current_thread = thread_current();
	struct list_elem *next, *e;

	if (file_descriptor != PROCESS_EXIT) {
		struct file_details *file_details_ = find_file_details(current_thread,
				file_descriptor);
		if (file_details_ != NULL) {

			//helper to close file with descriptor fd.
			close_single_file(file_details_);
		}
	} else {
		for (e = list_begin(&current_thread->currently_used_files);
				e != list_end(&current_thread->currently_used_files); e =
						next) {
			next = list_next(e);

			struct file_details *file_details_ =
					list_entry (e, struct file_details, elem);

			if (file_details_ != NULL) {
				close_single_file(file_details_);
			}
		}
	}
}

/**
 * Closes a single file
 */
void close_single_file(struct file_details* file_detials) {

	if (file_detials != NULL) {
		file_close(file_detials->file);
		list_remove(&file_detials->elem);

		free(file_detials);
	}
}

/**
 * Finds the file details in the list of currently used files of thread using the file descriptor.
 */
struct file_details* find_file_details(struct thread *t, int file_descriptor) {
	struct list_elem *e;

	for (e = list_begin(&t->currently_used_files);
			e != list_end(&t->currently_used_files); e = list_next(e)) {

		struct file_details *file_details_ =
				list_entry (e, struct file_details, elem);

		if (file_descriptor == file_details_->file_descriptor) {
			return file_details_;
		}
	}
	return NULL;
}

// Returns the file pointer
char* get_physical_file(int file_syscall_parameter) {

	return (const char *) get_physicaladdr((const void *) file_syscall_parameter);
}

// returns the size as unsidned value.
unsigned int get_size(int size_syscall_parameter) {
	return (unsigned) size_syscall_parameter;
}

// Returns the file descriptor.
int get_file_descriptor(int file_descriptor_syscall_parameter) {
	return file_descriptor_syscall_parameter;
}

// Reads for standard input.
int read_from_standard_input(void *buffer, unsigned size_to_be_read) {
	unsigned i;
	uint8_t* buffer_ = (uint8_t *) buffer;
	for (i = 0; i < size_to_be_read; i++) {
		buffer_[i] = input_getc();
	}
	return size_to_be_read;
}

// Reads from file
int read_from_file(int file_descriptor, void *buffer, unsigned size_to_be_read) {
	lock_acquire(&file_resource_lock);
	struct file *f = get_file_from_currently_used_files(file_descriptor);
	if (f == NULL) {
		lock_release(&file_resource_lock);
		return SYSCALL_ERROR;
	}
	int bytes = file_read(f, buffer, size_to_be_read);
	lock_release(&file_resource_lock);
	return bytes;
}

// Write to standard output
int write_to_standard_output(void *buffer, unsigned size_to_be_read) {
	putbuf(buffer, size_to_be_read);
	return size_to_be_read;
}

// Writes to file
int write_to_file(int file_descriptor, void *buffer, unsigned size_to_be_read) {

	lock_acquire(&file_resource_lock);
	struct file *f = get_file_from_currently_used_files(file_descriptor);
	if ((f == NULL) || check_if_file_write_deny(f)) {
		lock_release(&file_resource_lock);
		return SYSCALL_ERROR;
	}

	int bytes = file_write(f, buffer, size_to_be_read);
	lock_release(&file_resource_lock);
	return bytes;
}

// Performs actions after file is opened
int perform_actions_after_file_open(struct file *file_) {

	if (file_ == NULL) {
		return SYSCALL_ERROR;
	}

	return add_file_to_currently_used_files(file_);
}

// Retrieves the system call parameters
void retrieve_syscall_param(struct intr_frame *f, int *arg,
		int number_of_parameters) {
	int i;
	int *ptr;

	int index = 0;
	for (i = 1; i <= number_of_parameters; i++, index++) {
		ptr = f->esp + i * sizeof(char *);
		//is_virtual_addr_valid((const void *) ptr);
		//TODO
		check_valid_ptr((const void *) ptr, f->esp);
		//***********
		arg[index] = *ptr;
	}
}
