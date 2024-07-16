
#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include<syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include<multitasking.h>
#include<memorymanagement.h>



using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;



void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}


void print_int(int value) {//provided by chatgpt.
    char buffer[12];  // Sufficient to hold all digits of a signed 32-bit int plus null terminator
    char *ptr = buffer + sizeof(buffer) - 1; // Start at the end of the buffer
    *ptr = '\0';  // Null terminate the string


    
    // Handle 0 explicitly, since the below loop will skip it
    if (value == 0) {
        *--ptr = '0';
    }

    int is_negative = value < 0;
    
    // Convert the integer to a string by processing the absolute value of each digit
    if (is_negative) {
        value = -value; // Make the value positive for processing
    }

    while (value > 0) {
        *--ptr = '0' + (value % 10);  // Find the last digit and convert to char
        value /= 10;  // Move to the next digit
    }

    if (is_negative) {
        *--ptr = '-';  // Add minus sign for negative numbers
    }

    printf(ptr); // Print the resulting string
}




void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}




class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};


//random number
long a = 25214903917;
long c = 11;
long previous = 17;


int rand(){
    long r = a* previous + c;
    previous = r;

    if(r < 0) return r*(-1);
    return r;
}


void sysprintf(char *str){
    asm("int $0x80" : : "a" (4), "b" (str));
}



void sysfork(){
    asm("int $0x80" : : "a" (2));


    /* unhandled interrupt 0x0d aliyorum bunu calistirmaya calisirken.
    // bu kod calisirsa forktan iki farkli processe iki farkli return degeri atayabilirim.
    int value_from_eax;
    asm("movl %%eax, %0": "=r" (value_from_eax));
    return value_from_eax;
    */
}

void sysexit(){
    asm("int $0x80" : : "a" (1));
}

void syswaitpid(int pid){
    asm("int $0x80" : : "a" (7), "b" (pid));
}

void sysexecve(void (*entrypoint)()){
    asm("int $0x80" : : "a" (11), "b" (entrypoint));
}
void oylesine(){
    printf("execve calisiyor");
    sysexit();
}


int get_pid()
{
    int value_from_ecx;
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

}
void taskA(){
    int i = 0; 
    sysfork();
    
    int value_from_ecx;
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){// child process

        printf("A");
        sysexecve(oylesine);
        sysexit();
    }
    else{//parent process
        syswaitpid(2);
        printf(" after waitpid");

    }

    printf("fork yeni bitti2");
    while(1){
    }
}




/// calistirilacak fonksiyonlar , hepsi chatgptden cekildi

// Function to print the Collatz sequence for the number n
void printCollatz() { // chat gpt
    for(int i = 0;i<100;i++){
    int n = 7;
    printf("Collatz sequence for ");
    print_int(n);
    printf("\n");
    while (n != 1) {
        print_int(n);
        printf("  ");
        if (n % 2 == 0) {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
    }
    print_int(n);  // Print the last number in the sequence (which is 1)
 }
    sysexit();
    while(1);
}

void long_running_program() { // chat gpt
    int n = 1000;
    int result = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result += i * j;
        }
    }
    printf("result long running: ");
    print_int(result);
    sysexit();

    while(1);
}

void binarySearch() {
        printf("binary");

    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int x = 110;
    int n = sizeof(arr) / sizeof(arr[0]);
    int low = 0, high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        // Eğer x orta değerse, ortanın indeksini döndür
        if (arr[mid] == x){
            print_int(mid);
            sysexit();
        } 

        // Eğer x orta değerden büyükse, sol yarıyı atla
        if (arr[mid] < x) low = mid + 1;

        // Eğer x orta değerden küçükse, sağ yarıyı atla
        else high = mid - 1;
    }

    // Eğer element bulunamazsa -1 döndür
    print_int(-1);
    sysexit();
}

void linearSearch() {
    printf("linear");
    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int x = 175;
    int n = sizeof(arr) / sizeof(arr[0]);

    for (int i = 0; i < n; i++) {
        if (arr[i] == x) {
            print_int(i);
            sysexit(); // Eğer değer bulunursa indeksini döndür
        }
    }
    print_int(-1);
    sysexit();  // Eğer değer bulunamazsa -1 döndür
}


/*
void taskA(){
    int i = 0; 
    sysfork();
    
    int value_from_ecx;
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){// child process
        //sysfork();

        printf("A");
        print_int(i);
        //while(1);
        sysexit();
        //while(1){printf("A");}
    }
    else{//parent process
        print_int(i);
        i = 1;
        printf("B");
        //syswaitpid(2);
        printf(" after waitpid");

        //sysexecve(oylesine);
        
        //while(1);
        //while(1){printf("B");}
    }
    sysfork();

    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0)
        printf("fork yeni bitti1");
    else
        printf("fork yeni bitti2");
    while(1){
    }
}
*/

//part A lifecycle 1, working well
void partA(){
    int a;
    for(int i = 0;i < 3;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            printCollatz();
            sysexit();
        }
    }

    for(int i = 0;i < 3;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            long_running_program();
            sysexit();
        }
    }
    printf("fordan cikti baba");
    while(1);
}

void bStartegyOne(){
    int random = rand() % 4;

    if(random == 0){
        for(int i = 0;i < 10;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            printCollatz();
            sysexit();
            }
        }
    }
    else if(random == 1){
        for(int i = 0;i < 10;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            long_running_program();
            sysexit();
            }
        }
    }
    else if(random == 2){
        for(int i = 0;i < 10;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            binarySearch();
            sysexit();
            }
        }
    }
    else{
        for(int i = 0;i < 10;i++){ // run print Collatz 3 times and long running 3 ties
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            linearSearch();
            sysexit();
            }
        }
    }

    printf("Forktan cikti baba");
    while(1);
}


void bStrategyTwo(){
    int firstRandom = rand() % 4;
    int secondRandom = rand() % 4;

    while(firstRandom == secondRandom){// if they are same, change second random value
        secondRandom = rand();
    }

    if(firstRandom == 0 && secondRandom == 1){//long running and print collatz
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            printCollatz();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            long_running_program();
            sysexit();
            }
        }
    }
    else if(firstRandom == 0 && secondRandom == 2){//print collatz and binary search
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            printCollatz();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            binarySearch();
            sysexit();
            }
        }

    }
    else if(firstRandom == 0 && secondRandom == 3){//print collatz and linear search
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            printCollatz();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            linearSearch();
            sysexit();
            }
        }
    }
    else if(firstRandom == 1 && secondRandom == 2){//binary and long running
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            binarySearch();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            long_running_program();
            sysexit();
            }
        }
    }
    else if(firstRandom == 1 && secondRandom == 3){//linear and long running
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            linearSearch();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            long_running_program();
            sysexit();
            }
        }
    }
    else if(firstRandom == 2 && secondRandom == 3){//linear and binary
        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            linearSearch();
            sysexit();
            }
        }

        for(int i = 0;i < 3;i++){ 
        sysfork();
        int value_from_ecx;
        asm("movl %%ecx, %0": "=r" (value_from_ecx));
        print_int(value_from_ecx);

        if(value_from_ecx == 0){
            binarySearch();
            sysexit();
            }
        }
    }

    printf("Forktan cikti baba");
    while(1);
}


void bStrategyThree(){
    sysfork();
    int value_from_ecx;
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){
        printCollatz();
        sysexit();
    }


    sysfork();
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){
        long_running_program();
        sysexit();
    }

    sysfork();
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){
        binarySearch();
        sysexit();
    }

    sysfork();
    asm("movl %%ecx, %0": "=r" (value_from_ecx));
    print_int(value_from_ecx);

    if(value_from_ecx == 0){
        linearSearch();
        sysexit();
    }

    printf("Fork bitti baba");

    while(1);

}




typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;

    TaskManager taskManager(&gdt);


    //---ALL SYSTEM CALL TESTS--, All are working
    Task task1(&gdt, taskA);
    taskManager.AddTask(&task1,false);


    //part a lifecycle 1
    //Task task2(&gdt,partA);
    //taskManager.AddTask(&task2,false);

    //part b strategy 1
    //Task task3(&gdt,bStartegyOne);
    //taskManager.AddTask(&task3,false);

    //part b strategy 2
    //Task task4(&gdt,bStrategyTwo);
    //taskManager.AddTask(&task4,false);

    //part b strategy 3
    //Task task5(&gdt,bStrategyThree);
    //taskManager.AddTask(&task5,false);



    

    //----PART A Lifecycle 1 Strategy Test
    //lifecycle1(&gdt,&taskManager); mmmmmmmmmmmmmmmmmmmmm

    //randoM nuMber, working
    //int no = rand();
    //print_int(no % 4);

    //----PART B

    //---PART B strategy 1
    //bStrategy1(&gdt,&taskManager);


    //---PART B strategy 2
    //bStrategy2(&gdt,&taskManager);

    //---PART B strategy 3
    //bStrategy3(&gdt,&taskManager);


    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80,&taskManager);

    //memory management
    /*uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8) & 0xFF);
    printfHex((heap) & 0xFF);

    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8) & 0xFF);
    printfHex(((size_t)allocated) & 0xFF);
    printf("\n");*/



    interrupts.Activate();
    
    while(1)
    {}
}
