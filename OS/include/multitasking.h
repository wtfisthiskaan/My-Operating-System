#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H


#include "gdt.h"
#include "common/types.h"
#include "memorymanagement.h"


namespace myos{
	enum State {
		READY,
		RUNNING,
		BLOCKED,
		TERMINATED
	};

	typedef enum State State;

	
	struct CPUState{
		common::uint32_t eax;
		common::uint32_t ebx;
		common::uint32_t ecx;
		common::uint32_t edx;

		common::uint32_t esi;
		common::uint32_t edi;
		common::uint32_t ebp;

/*
		common::uint32_t gs;
		common::uint32_t fs;
		common::uint32_t es;
		common::uint32_t ds;

*/		
		common::uint32_t error;

		common::uint32_t eip;
		common::uint32_t cs;
		common::uint32_t eflags;
		common::uint32_t esp;
		common::uint32_t ss;
	} __attribute__((packed));

	class Task{
		friend class TaskManager;
	private:
		common::uint8_t stack[4096]; // 4KiB
		CPUState* cpustate;

		//processe ozgu seyler
		int pid;
		int pPid;
		State state;
		bool isParentWaiting;
	public:
		Task(GlobalDescriptorTable *gdt, void entrypoint());
		Task();
		~Task();


	};

	class TaskManager{
	private:
		GlobalDescriptorTable* gdt;
		Task* tasks[256];
		Task ogullar[50];

		int numTasks;
		int currentTask;
		int pidCounter;
		int interruptCount;
		int ogulCount;
	public:
		TaskManager(GlobalDescriptorTable* gdt);
		~TaskManager();
		bool AddTask(Task* task,bool fromFork);
		CPUState* Schedule(CPUState* cpustate);
		void printProcessTable();

		//syscalls
		common::uint32_t fork(CPUState* cpu, int* child_pid);
		void exit();
		void waitpid(common::uint32_t child_pid);
		common::uint32_t execve(void (*entrypoint)());
	};
}














#endif