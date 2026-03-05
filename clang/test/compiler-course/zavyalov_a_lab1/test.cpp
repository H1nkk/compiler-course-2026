// RUN: %clang_cc1 -load %llvmshlibdir/zavyalov_a_lab1_ClangAST%pluginext -plugin zavyalov_a_lab1_plugin -fsyntax-only %s 2>&1

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
}