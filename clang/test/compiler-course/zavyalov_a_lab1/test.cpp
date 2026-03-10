// RUN: %clang_cc1 -load %llvmshlibdir/zavyalov_a_lab1_ClangAST%pluginext -plugin zavyalov_a_lab1_plugin -fsyntax-only %s 2>&1

class A {
    int field = 0;
public:
    void nonconstMethod() {
        ++field;
    }

    int constMethod() const {
        return 44 + 5;
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
    // Pointers that can be made const
    int* x = new int(10);        // Можно сделать const (не изменяется)

    int* z = new int(10);         // Можно сделать const (не изменяется)

    int ** px = &x; // can be made const
    int *** ppx = &px;
    int **** pppx = &ppx;

    int* sum = new int(*x + *z);  // Можно сделать const (не изменяется)

    A *custom_class_test_const = new A(); // Можно сделать const (не изменяется)
    custom_class_test_const->constMethod();
    

    // Pointers that can not be made const
    int* y = new int(10);        // Изменяется
    int* second_y = y;          // changed with y
    *y = 25;  // присваивание
    
    int* w = new int(10);         // Изменяется через указатель

    int** w_ptr = &w;
    *w_ptr = new int(45);  // изменение через указатель

    int* third_y = y; // same as second_y but declared after y's change

    int* ptr_reassigned = new int(40); // cant be made const
    ptr_reassigned = new int(30);

    int* ptr_incr = new int(10); // cant be made const
    ptr_incr++;
    
    A *custom_class_test_nonconst = new A(); // Можно сделать const (не изменяется)
    custom_class_test_nonconst->nonconstMethod();


    // References that can not be made const
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
    A &a_nonconst_ref = a;
    a_nonconst_ref.nonconstMethod();


    // References that can be made const
    int &t_ref_const_unchanged = t;

    int &t_ref_const_func_argument_ref = t;
    foo_const_ref(t_ref_const_func_argument_ref);

    int &t_ref_const_func_argument_value = t;
    foo_value(t_ref_const_func_argument_value);

    A &a_const_ref = a;
    a_const_ref.constMethod();

    ++t;
}