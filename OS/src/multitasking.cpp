#include "multitasking.h"
#include "memorymanagement.h"

using namespace myos;
using namespace myos::common;

void printf(char *s);
void print_int(int value);


Task::Task(GlobalDescriptorTable *gdt, void entrypoint()){ // reeedit this to process

	cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));

	cpustate->eax = 0;
	cpustate->ebx = 0;
	cpustate->ecx = 0;
	cpustate->edx = 0;

	cpustate->esi = 0;
	cpustate->edi = 0;
	cpustate->ebp = 0;
/*
	cpustate->gs = 0;
	cpustate->fs = 0;
	cpustate->es = 0;
	cpustate->ds = 0;

	cpustate->error = 0;
*/
	
	cpustate->eip = (uint32_t)entrypoint;
	cpustate->cs = gdt->CodeSegmentSelector();
	// cpustate-> ss = ;
	cpustate->eflags = 0x202;

	isParentWaiting = false;
	pid = 0;
	pPid = 0;
}

Task::Task(){
	cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));

	cpustate->eax = 0;
	cpustate->ebx = 0;
	cpustate->ecx = 0;
	cpustate->edx = 0;

	cpustate->esi = 0;
	cpustate->edi = 0;
	cpustate->ebp = 0;
	
	cpustate->eip = -1;
	cpustate->cs = -1;
	cpustate->eflags = 0x202;

	isParentWaiting = false;
	pid = 0;
	pPid = 0;
}

Task::~Task(){

}






TaskManager::TaskManager(GlobalDescriptorTable *gdt){
	this->gdt = gdt;
	numTasks = 0;
	currentTask = -1;

	pidCounter = 0;

	interruptCount = 0;

	ogulCount = 0;
}
TaskManager::~TaskManager(){}

bool TaskManager::AddTask(Task* task, bool fromFork){

	//printProcessTable();
	if(numTasks >= 256){
		return false;
	}

	//set process properties
	//printProcessTable();
	pidCounter++;
	task->pid = pidCounter;
	task->pPid = 0;
	task->state = READY; // task is ready to run.
	if(numTasks <= 0){
		task->pPid = 0; // if process is the first one, then it's parent is kernel which we assume it 0 		
		task->state = RUNNING; // task is running first one.
	}
	else
		task->pPid = tasks[currentTask]->pid;

	if(!fromFork){
		task->pPid = 0;
	}
	//printProcessTable();

	tasks[numTasks] = task;
	numTasks++;
	return true;
}

uint32_t TaskManager::fork(CPUState* cpu,int* child_pid){
	printf("LOW: ");
	print_int(currentTask);
	//printProcessTable();
	Task* baba = tasks[currentTask];
	//Task* ogul2 = new Task(gdt,(void(*)()) baba->cpustate->eip); // aynı adres yerinde yeni process olusturuyor..
	//ogul->cpustate = (CPUState*)(ogul->stack + 4096 - sizeof(CPUState));
	//Task* ogul = new Task(gdt,(void(*)()) baba->cpustate->eip); // aynı adres yerinde yeni process olusturuyor..
	Task *ogul = &ogullar[ogulCount];

	ogul->cpustate->eip = (uint32_t)baba->cpustate->eip;
	ogul->cpustate->cs = gdt->CodeSegmentSelector();


	for(int i = 0;i < 4096; i++){
		ogul->stack[i] = baba->stack[i];
	}

	ogul->cpustate = (CPUState*)(ogul->stack + (4096) - sizeof(CPUState));
	common::uint32_t offset = ((uint32_t)baba->cpustate) - ((uint32_t)cpu);
	ogul->cpustate = (CPUState*)(((uint32_t)ogul->cpustate) - offset);

	*(ogul->cpustate) = *cpu;

	ogul->cpustate->ecx = 0;



	AddTask(&(*ogul), true);


	int pidValue = ogul->pid;
	(*child_pid) = pidValue;
	printf("\n\n\n\n\n");
	//delete (ogul);

	ogulCount++;

	return (uint32_t) tasks[currentTask]->cpustate;
}



void TaskManager::exit(){
	printf("exit");
	if(!tasks[currentTask]->isParentWaiting){
		printf("abov");
	}
	if(tasks[currentTask]->isParentWaiting){
		printf("dumen");
		for(int i = 0; i < numTasks; i++){
			if(tasks[i]->pid == tasks[currentTask]->pPid){
				tasks[i]->state = READY;
				break;
			}
		}
	}

	tasks[currentTask]->state = TERMINATED; // this task never will be executed again. it is guaranteed by schedule function.
}

void TaskManager::waitpid(uint32_t child_pid){
	printf(" waitpid girdi");
	print_int(child_pid);
	for(int i = 0; i < numTasks; i++){
		if(tasks[i]->pid == child_pid){
			tasks[currentTask]->state = BLOCKED;
			tasks[i]->isParentWaiting = true;
			return;
		}
	}
	// if there is no child, don't do anything..
}


uint32_t TaskManager::execve(void entrypoint()){
	printf("execve girdi");
	print_int(currentTask);
	int pid = tasks[currentTask]->pid;
	int pPid = tasks[currentTask]->pPid;
	bool isParentWaiting = tasks[currentTask]->isParentWaiting;

	tasks[currentTask] = new Task(gdt, entrypoint);
	tasks[currentTask]->pid = pid;
	tasks[currentTask]->pPid = pPid;
	tasks[currentTask]->state = READY;
	tasks[currentTask]->isParentWaiting = isParentWaiting;



	
	printf("execve cikis");
	return (uint32_t) tasks[currentTask]->cpustate;
}


void TaskManager::printProcessTable(){
	printf("\n--------------------------------------------");
	printf("\n             PROCESS TABLE");
	printf("\n PID PPID STATE \n");

	for(int i = 0; i < numTasks; i++){
		//int pid = tasks[i]->pid;
		//int pPid = tasks[i]->pPid;
		State state = tasks[i]->state;

		printf("  ");
		print_int(tasks[i]->pid);
		printf("   ");
		print_int(tasks[i]->pPid);
		printf("   ");
		if(state == READY)
			printf("READY");
		else if(state == BLOCKED)
			printf("BLOCKED");
		else if(state == TERMINATED)
			printf("TERMINATED");
		else
			printf("RUNNING");
		printf("\n\n");
	}
	printf("--------------------------------------------\n");

}


/*
CPUState* TaskManager::Schedule(CPUState* cpustate){//part b strategy 3 schedule

	interruptCount++;
	if(numTasks <= 0 || interruptCount < 5)
		return cpustate;
	

	printProcessTable();

	if(currentTask >= 0){
		tasks[currentTask]->cpustate = cpustate;
	}
	if(tasks[currentTask]->state == RUNNING)
		tasks[currentTask]->state = READY;


if(++currentTask >= numTasks){
		currentTask %= numTasks;
} // if the task is blocked or terminated do not schedule it.

while(tasks[currentTask]->state == BLOCKED || tasks[currentTask]->state == TERMINATED){// if the task is blocked or terminated do not schedule it.
	//print_int(currentTask);
	if(++currentTask >= numTasks){
		currentTask %= numTasks;
	}
}
	//print_int(currentTask);
	tasks[currentTask]->state = RUNNING;


	return tasks[currentTask] -> cpustate;
}*/

CPUState* TaskManager::Schedule(CPUState* cpustate){//asil schedule

	if(numTasks <= 0)
		return cpustate;

	//printProcessTable();

	if(currentTask >= 0){
		tasks[currentTask]->cpustate = cpustate;
	}
	if(tasks[currentTask]->state == RUNNING)
		tasks[currentTask]->state = READY;


if(++currentTask >= numTasks){
		currentTask %= numTasks;
} // if the task is blocked or terminated do not schedule it.

while(tasks[currentTask]->state == BLOCKED || tasks[currentTask]->state == TERMINATED){// if the task is blocked or terminated do not schedule it.
	//print_int(currentTask);
	if(++currentTask >= numTasks){
		currentTask %= numTasks;
	}
}
	//print_int(currentTask);
	tasks[currentTask]->state = RUNNING;


	return tasks[currentTask] -> cpustate;
}

