// RUN: %clang_cc1 -load %llvmshlibdir/zavyalov_a_lab1_ClangAST%pluginext -plugin zavyalov_a_lab1_plugin -fsyntax-only %s 2>&1

class A {
    int field = 0;
public:
    void nonconstMethod() {
        ++field;
    }
};

void foo_nonconst(int& x) {
    x += 5;
}

void foo_const_ref(const int& x) {
}

void foo_value(int x) {
}

void foo_address(int* x) {
    *x = 42;
}

void example() {
    int* x = new int(10);        // Можно сделать const (не изменяется)
    int* y = new int(10);        // Изменяется
    int* z = new int(10);         // Можно сделать const (не изменяется)
    int* w = new int(10);         // Изменяется через указатель
    int ** px = &x; // пока заменяется на *, а не на **
    *y = 25;  // присваивание
    
    int** ptr = &w;
    *ptr = new int(45);  // изменение через указатель
    
    int* sum = new int(*x + *y + *z);  // Можно сделать const (не изменяется)
    
    for (int i = 0; i < 10; ++i) { 
        // ...
    }

    int t = 5;
    int &t_ref_nonconst_assgn = t;
    t_ref_nonconst_assgn = 6;

    int &t_ref_nonconst_compound_add = t;
    t_ref_nonconst_compound_add += 4;

    int &t_ref_nonconst_incr = t;
    t_ref_nonconst_incr++;

    int &t_ref_nonconst_func_argument = t;
    foo_nonconst(t_ref_nonconst_func_argument);

    int &t_ref_nonconst_func_argument_by_address = t;
    foo_address(&t_ref_nonconst_func_argument_by_address);

    int &t_ref_nonconst_initializer = t;
    int &t_ref_nonconst_initialized_by_another_ref = t_ref_nonconst_initializer;
    t_ref_nonconst_initializer += 5;

    A a;
    A &a_ref = a;
    a_ref.nonconstMethod();

    
    int &t_ref_const_unchanged = t;

    int &t_ref_const_func_argument_ref = t;
    foo_const_ref(t_ref_const_func_argument_ref);

    int &t_ref_const_func_argument_value = t;
    foo_value(t_ref_const_func_argument_value);

    ++t;
}