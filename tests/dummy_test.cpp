#include <iostream>

// Test mínimo sin framework.
// Su objetivo es validar la infraestructura de CTest.
// - Si se ejecuta correctamente, el proceso termina con código 0 (test pasa).
// - Si queremos simular un fallo, bastaría con devolver un código distinto.
int main() {
    std::cout << "[dummy_test] CTest infrastructure OK" << std::endl;
    return 0;
}
