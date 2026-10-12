#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

int main() {
    // 1. Setup core LLVM components
    llvm::LLVMContext context;
    llvm::Module module("my_cool_module", context);
    llvm::IRBuilder<> builder(context);

    // 2. Create a main function: `define i64 @main()`
    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getInt64Ty(), false);
    llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    // 3. Create a basic block (a block of code instructions) inside the function
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // 4. Allocate a variable: `variable = 123;`
    llvm::AllocaInst *varPtr = builder.CreateAlloca(builder.getInt64Ty(), nullptr, "variable");
    llvm::Value *val123 = builder.getInt64(123);
    builder.CreateStore(val123, varPtr);

    // 5. Return the value from main (so we can test it)
    llvm::Value *loadedVal = builder.CreateLoad(builder.getInt64Ty(), varPtr, "load_val");
    builder.CreateRet(loadedVal);

    // 6. Print the generated LLVM IR to the terminal
    module.print(llvm::outs(), nullptr);

    return 0;
}
