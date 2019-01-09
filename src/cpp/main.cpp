#include <stdio.h>
#include <string>
#include <memory>

using namespace std;
#include "pml.h"
#include "simple.h"
#include "chow_liu.h"

int main() {
    pml("in.txt", "out.txt");
    // simple("ent", "out2.txt");
    // chow_liu("bla", "out3.txt");
    return 0;    
}
