#include <syscalls.h>


using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
	
SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber,TaskManager* taskManager)
: InterruptHandler(interruptManager, InterruptNumber + interruptManager->HardwareInterruptOffset())
{
	this->taskManager = taskManager;
}
		
SyscallHandler::~SyscallHandler(){}
        
void printf(char*);

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp){
	CPUState* cpu = (CPUState*)esp;


	switch(cpu->eax){
	case 1:
		taskManager->exit();
		esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
		break;
	case 2:
		int child_pid;
		taskManager->fork(cpu,&child_pid);
		cpu->ecx = child_pid;
		printf("babu");
		break;
	case 4:
		printf((char*)cpu->ebx);
		break;
	case 7:
		taskManager->waitpid(cpu->ebx);
		esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
		break;
	case 11:
		esp = taskManager->execve((void (*)()) cpu->ebx);
		//esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
		break;
	default:
		break;
	}

	return esp;
}
