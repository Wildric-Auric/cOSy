


class A {
    int var;
    static int counter = 0;

    static void st_func(int arg);
           void func_inst(int arg);
    
};

struct A {
    int var;
};

void func_inst(A*, int arg);

void st_func(int arg);


void main() {
    A a;
    sizeof(a);
    A::st_func(0);
    A a;
    a.st_func();
}


